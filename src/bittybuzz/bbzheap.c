#include "bbzheap.h"

/****************************************/
/****************************************/

#define gc_hasmark(x) ((x).mdata & BBZHEAP_MASK_GCMARK)
#define gc_mark(x)    (x).mdata |= BBZHEAP_MASK_GCMARK
#define gc_unmark(x)  (x).mdata &= ~BBZHEAP_MASK_GCMARK

/****************************************/
/****************************************/

void bbzheap_clear() {
    vm->heap.rtobj = vm->heap.data + RESERVED_ACTREC_MAX * sizeof(bbzobj_t);
    vm->heap.ltseg = vm->heap.data + BBZHEAP_SIZE;
    for(int16_t i = (RESERVED_ACTREC_MAX-1)* sizeof(bbzobj_t); i >= 0; --i) {
        vm->heap.data[i] = 0;
    }
}

/****************************************/
/****************************************/

static uint8_t bbzheap_obj_alloc_prepare_obj(uint8_t t, bbzobj_t* x) {
    /* Set valid bit and type */
    x->mdata = (t << BBZTYPE_TYPEIDX) | BBZHEAP_MASK_OBJ_VALID;
    /* Take care of special initialisations */
    if (t == BBZTYPE_TABLE) {
        if (!bbzheap_tseg_alloc(&x->t.value)) return 0;
    }
    else if (t == BBZTYPE_CLOSURE) {
        bbzclosure_unmake_lambda(*x);
        (x)->l.value.actrec = BBZ_DFLT_ACTREC; // Default activation record
    }
    /* Success */
    return 1;
}

uint8_t bbzheap_obj_alloc(uint8_t t,
                          bbzheap_idx_t* o) {
    /* Look for empty slot */
    for(uint16_t i = RESERVED_ACTREC_MAX;
        i < (uint16_t)(vm->heap.rtobj - vm->heap.data) / sizeof(bbzobj_t);
        ++i) {
        if(!bbzheap_obj_isvalid(*bbzheap_obj_at(i))) {
            /* Empty slot found */
            /* Set result */
            *o = i;
            return bbzheap_obj_alloc_prepare_obj(t, bbzheap_obj_at(i));
        }
    }
    /* No empty slot found, must create a new one */
    /* ...but first, make sure there is room */
    if(vm->heap.rtobj + sizeof(bbzobj_t) > vm->heap.ltseg) { return 0; }
    /* Set result */
    *o = (uint16_t)(vm->heap.rtobj - vm->heap.data) / sizeof(bbzobj_t);
    vm->heap.rtobj += sizeof(bbzobj_t);
    return bbzheap_obj_alloc_prepare_obj(t, (bbzobj_t*)(vm->heap.rtobj - sizeof(bbzobj_t)));
}

/****************************************/
/****************************************/

bbzobj_t* bbzheap_obj_at(bbzheap_idx_t i) {
    return (bbzobj_t*)vm->heap.data + i;
}

/****************************************/
/****************************************/

static uint8_t bbzheap_tseg_alloc_prepare_seg(bbzheap_tseg_t* x) {
    /* Set valid bit of segment and -1 index for next */
    tseg_makevalid(*x);
    /* Invalidate keys and values */
    for(uint8_t j = 0; j < BBZHEAP_ELEMS_PER_TSEG; ++j) {
        x->keys[j] = 0;
        x->values[j] = 0;
    }
    /* Success */
    return 1;
}

uint8_t bbzheap_tseg_alloc(bbzheap_idx_t* s) {
    /* Look for empty slot */
    int16_t qot = (int16_t)(vm->heap.data + BBZHEAP_SIZE - vm->heap.ltseg) / sizeof(bbzheap_tseg_t);
    for(int16_t i = 0;
        i < qot;
        ++i) {
        if(!bbzheap_tseg_isvalid(*bbzheap_tseg_at(i))) {
            /* Empty slot found */
            /* Set result */
            *s = (uint16_t)i;
            return bbzheap_tseg_alloc_prepare_seg(bbzheap_tseg_at(i));
        }
    }
    /* Make sure there is room */
    if(vm->heap.ltseg - sizeof(bbzheap_tseg_t) < vm->heap.rtobj) return 0;
    /* Set result */
    *s = (uint16_t)qot;
    /* Update pointer to leftmost valid segment */
    vm->heap.ltseg -= sizeof(bbzheap_tseg_t);
    return bbzheap_tseg_alloc_prepare_seg((bbzheap_tseg_t*)vm->heap.ltseg);
}

/****************************************/
/****************************************/
static void bbzheap_gc_mark(bbzheap_idx_t obj) {
    static uint8_t callstack = 1; // The value of 1 is necessary
    if (++callstack <= BBZHEAP_GCMARK_DEPTH && !gc_hasmark(*bbzheap_obj_at(obj))) {
        /* Mark gc bit */
        gc_mark(*bbzheap_obj_at(obj));
        /* If it's a table, go through it and mark all associated objects */
        if (bbztype_istable(*bbzheap_obj_at(obj))) {
            /* Segment index in heap */
            bbzheap_idx_t si = bbzheap_obj_at(obj)->t.value;
            /* Actual segment data in heap */
            bbzheap_aseg_t *sd = bbzheap_aseg_at(si);
            /* Go through the segments */
            while (1) {
                gc_tseg_mark(*sd);
                for (uint8_t j = 0; j < 2 * BBZHEAP_ELEMS_PER_TSEG; ++j) {
                    if (bbzheap_aseg_elem_isvalid(sd->values[j])) {
                        bbzheap_gc_mark(bbzheap_aseg_elem_get(sd->values[j]));
                    }
                }
                if (!bbzheap_aseg_hasnext(sd)) break;
                si = bbzheap_aseg_next_get(sd);
                sd = bbzheap_aseg_at(si);
            }
        }
        else if (bbztype_isclosurelambda(*bbzheap_obj_at(obj)) &&
                 bbzheap_obj_at(obj)->l.value.actrec != BBZ_DFLT_ACTREC) {
            bbzheap_gc_mark(bbzheap_obj_at(obj)->l.value.actrec);
        }
    }
    --callstack;
}

void bbzheap_gc(bbzheap_idx_t* st,
                uint16_t sz) {
    uint16_t i;
    const uint16_t qot = (int16_t)(vm->heap.rtobj - vm->heap.data) / sizeof(bbzobj_t),
                   qot2 = (int16_t)(vm->heap.data + BBZHEAP_SIZE - vm->heap.ltseg) / sizeof(bbzheap_tseg_t);
    /* Set all segment's gc bits to zero */
    for(i = qot2; i-- != 0;)
        gc_tseg_unmark(*bbzheap_tseg_at(i));
    /* Set all gc bits to zero */
    for(i = qot; i-- != 0;) {
        gc_unmark(*bbzheap_obj_at(i));
    }
    for(i = qot; i-- != 0;) {
        if (bbzheap_obj_ispermanent(*bbzheap_obj_at(i))) {
            bbzheap_gc_mark((bbzheap_idx_t)(i));
        }
    }
    /* Go through the stack and set the gc bit of valid variables */
    for(i = sz; i-- != 0;) {
        /* Mark gc bit */
        bbzheap_gc_mark(st[i]);
    }
    /* Go through the objects; invalidate those with 0 gc bit */
    for(i = qot; i-- != 0;) {
        if(!gc_hasmark(*bbzheap_obj_at(i)) && bbzheap_obj_isvalid(*bbzheap_obj_at(i))) {
            /* Invalidate object */
            obj_makeinvalid(*bbzheap_obj_at(i));
            /* If it's a table, invalidate its segments too */
            if(bbztype_istable(*bbzheap_obj_at(i))) {
                /* Segment index in heap */
                bbzheap_idx_t si = bbzheap_obj_at(i)->t.value;
                // FIXED We should add a tseg GC mark. In the case we
                // where have two 'equal' tables, but one has a mark
                // and one does not, we do not want to invalidate the
                // table segments.
                if(gc_tseg_hasmark(*bbzheap_tseg_at(bbzheap_obj_at(i)->t.value))) {
                    continue;
                }
                /* Actual segment data in heap */
                bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
                /* Go through the segments and invalidate them all */
                while(1) {
                    tseg_makeinvalid(*sd);
                    if(!bbzheap_tseg_hasnext(sd)) break;
                    si = bbzheap_tseg_next_get(sd);
                    sd = bbzheap_tseg_at(si);
                }
            }
        }
    }
    /* Move rightmost object pointer as far left as possible */
    for(;
        vm->heap.rtobj > vm->heap.data + RESERVED_ACTREC_MAX*sizeof(bbzobj_t);
        vm->heap.rtobj -= sizeof(bbzobj_t))
        if(bbzheap_obj_isvalid(*(bbzobj_t*)(vm->heap.rtobj - sizeof(bbzobj_t))))
            break;
    /* Move leftmost table segment pointer as far right as possible */
    for(;
        vm->heap.ltseg < vm->heap.data + BBZHEAP_SIZE;
        vm->heap.ltseg += sizeof(bbzheap_tseg_t)) {
        if(bbzheap_tseg_isvalid(*(bbzheap_tseg_t*)vm->heap.ltseg))
            break;
    }
}

/****************************************/
/****************************************/

#ifndef BBZCROSSCOMPILING

static const char* bbzvm_types_desc[] = { "nil", "integer", "float", "string", "table", "closure", "userdata" };

void bbzheap_print() {
    /* Object-related stuff */
    uint16_t objimax = (vm->heap.rtobj - vm->heap.data) / sizeof(bbzobj_t);
    printf("Max object index: %d\n", objimax - 1);
    uint16_t objnum = 0;
    for(uint16_t i = 0; i < objimax; ++i)
        if(bbzheap_obj_isvalid(*bbzheap_obj_at(i))) ++objnum;
    printf("Valid objects: %d\n", objnum);
    for(uint16_t i = 0; i < objimax; ++i)
        if(bbzheap_obj_isvalid(*bbzheap_obj_at(i))) {
            printf("\t#%d: [%s]", i, bbzvm_types_desc[bbztype(*bbzheap_obj_at(i))]);
            if (bbzheap_obj_ispermanent(*bbzheap_obj_at(i))) printf("*");
            switch(bbztype(*bbzheap_obj_at(i))) {
                case BBZTYPE_NIL:
                    break;
                case BBZTYPE_STRING: // fallthrough
                case BBZTYPE_INT:
                    printf(" %d", bbzheap_obj_at(i)->i.value);
                    break;
                case BBZTYPE_FLOAT:
                    printf(" %f", bbzfloat_tofloat(bbzheap_obj_at(i)->f.value));
                    break;
                case BBZTYPE_TABLE:
                    printf(" %" PRIu16, bbzheap_obj_at(i)->t.value);
                    break;
                case BBZTYPE_USERDATA:
                    printf(" %" PRIXPTR, bbzheap_obj_at(i)->u.value);
                    break;
                case BBZTYPE_CLOSURE:
                    if (bbztype_isclosurenative(*bbzheap_obj_at(i))) printf("[n]");
                    if (bbztype_isclosurelambda(*bbzheap_obj_at(i))) printf("[l]");
                    else
                        printf(" %d", (int)(intptr_t)bbzheap_obj_at(i)->c.value);
                    break;
                default:
                    break;
            }
            printf("\n");
        }
    /* Segment-related stuff */
    int tsegimax = (vm->heap.data + BBZHEAP_SIZE - vm->heap.ltseg) / sizeof(bbzheap_tseg_t);
    printf("Max table segment index: %d\n", tsegimax);
    int tsegnum = 0;
    for(int i = 0; i < tsegimax; ++i)
        if(bbzheap_tseg_isvalid(*bbzheap_tseg_at(i))) ++tsegnum;
    printf("Valid table segments: %d\n", tsegnum);
    bbzheap_tseg_t* seg;
    for(int i = 0; i < tsegimax; ++i) {
        seg = bbzheap_tseg_at(i);
        if(bbzheap_tseg_isvalid(*seg)) {
            printf("\t#%d: {", i);
            for(int j = 0; j < BBZHEAP_ELEMS_PER_TSEG; ++j)
                if(bbzheap_tseg_elem_isvalid(seg->keys[j]))
                    printf(" (%d,%d)",
                           bbzheap_tseg_elem_get(seg->keys[j]),
                           bbzheap_tseg_elem_get(seg->values[j]));
            printf(" /next=(%x|%d) }\n", bbzheap_tseg_next_get(seg), bbzheap_tseg_next_get(seg));
        }
    }
    printf("\n");
}
#endif // !BBZCROSSCOMPILING