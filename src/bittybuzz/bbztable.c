#include "bbztable.h"
#include "bbzvm.h"

/****************************************/
/****************************************/

uint8_t bbztable_get(bbzheap_idx_t t,
                     bbzheap_idx_t k,
                     bbzheap_idx_t* v) {
    if (!bbztype_istable(*bbzheap_obj_at(t))) return 0;
    /* Get segment index */
    int16_t si = bbzheap_obj_at(t)->t.value;
    /* Get segment data */
    bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
    /* Go through segments */
    while (1) {
        /* Go through valid keys in the segment */
        for (uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (bbzheap_tseg_elem_isvalid(sd->keys[i]) &&
                bbztype_cmp(bbzheap_obj_at(bbzheap_tseg_elem_get(sd->keys[i])),
                            bbzheap_obj_at(k)) == 0) {
                /* Key found */
                *v = bbzheap_tseg_elem_get(sd->values[i]);
                return 1;
            }
        }
        /* Are we done? */
        if (!bbzheap_tseg_hasnext(sd)) return 0;
        /* Get next segment */
        si = bbzheap_tseg_next_get(sd);
        sd = bbzheap_tseg_at(si);
    }
}

/****************************************/
/****************************************/

uint8_t bbztable_set(bbzheap_idx_t t,
                     bbzheap_idx_t k,
                     bbzheap_idx_t v) {
    /* Search for the given key, keeping track of first free slot */
    /* Get segment index */
    int16_t si = bbzheap_obj_at(t)->t.value;
    /* Get segment data */
    bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
    /* Free segment and slot */
    int16_t fseg = -1, fslot = -1;
    /* Target segment and slot */
    int16_t seg = -1, slot = -1;
    /* Go through segments */
    while (1) {
        /* Go through valid keys in the segment */
        for (uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (!bbzheap_tseg_elem_isvalid(sd->keys[i])) {
                if (fseg < 0) {
                    /* First free slot found */
                    fseg = si;
                    fslot = i;
                }
            }
            else if (bbztype_cmp(bbzheap_obj_at(bbzheap_tseg_elem_get(sd->keys[i])),
                                bbzheap_obj_at(k)) == 0) {
                /* Key found */
                seg = si;
                slot = i;
            }
        }
        /* Did we find the key? */
        if (seg >= 0) break;
        /* Are we done? */
        if (!bbzheap_tseg_hasnext(sd)) break;
        /* Get next segment */
        si = bbzheap_tseg_next_get(sd);
        sd = bbzheap_tseg_at(si);
    }
    /* When we get here, we have three possibilities:
        * 1. We found the key;
        * 2. We did not find the key, and found an empty slot;
        * 3. We did not find the key, nor found an empty slot
        * Also, sd points to the last segment visited, and si is its index.
        */
    if(seg >= 0) {
        /* 1. We found the key, change associated value */
        /* NOTE: Setting a value to nil is equivalent to erasing the element from the table */
        if(!bbztype_isnil(*bbzheap_obj_at(v)))
            bbzheap_tseg_elem_set(bbzheap_tseg_at(seg)->values[slot], v);
        else {
            /* Setting nil, erase element */
            bbzheap_tseg_at(seg)->keys[slot] = 0;
            bbzheap_tseg_at(seg)->values[slot] = 0;
            /* Is segment empty? */
            for(uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
                if(bbzheap_tseg_elem_isvalid(sd->keys[i])) return 1;
            }
            /* If we get here, it's because the segment is empty */
            /* Is it the first segment? */
            if(si == bbzheap_obj_at(t)->t.value) {
                /* Yes; is it the only segment for this table? */
                if(bbzheap_tseg_hasnext(sd)) {
                    /* No, there's more segments */
                    /* Update the table segment index */
                    bbzheap_obj_at(t)->t.value = bbzheap_tseg_next_get(sd);
                    /* Invalidate the segment */
                    sd->mdata = 0;
                }
            }
            else {
                /* No, the segment is not the first */
                /* Find the preceding segment */
                uint16_t pi = bbzheap_obj_at(t)->t.value;
                bbzheap_tseg_t* pd = bbzheap_tseg_at(pi);
                while(bbzheap_tseg_next_get(pd) != si) {
                    pi = bbzheap_tseg_next_get(pd);
                    pd = bbzheap_tseg_at(pi);
                }
                /* Set the next of the preceding to the next of current */
                bbzheap_tseg_next_set(pd, bbzheap_tseg_next_get(sd));
                /* Invalidate the current segment */
                sd->mdata = 0;
            }
        }
    }
    /* Ignore setting nil on new elements */
    else if(!bbztype_isnil(*bbzheap_obj_at(v))) {
        if(fseg >= 0) {
            /* 2. We did not find the key, and found an empty slot */
            bbzheap_tseg_elem_set(bbzheap_tseg_at(fseg)->keys[fslot], k);
            bbzheap_tseg_elem_set(bbzheap_tseg_at(fseg)->values[fslot], v);
        }
        else {
            /* 3. We did not find the key, nor an empty slot */
            /* Create a new segment */
            uint16_t s;
            if(!bbzheap_tseg_alloc(&s)) return 0;
            bbzheap_tseg_next_set(sd, s);
            /* Set key and value */
            bbzheap_tseg_elem_set(bbzheap_tseg_at(s)->keys[0], k);
            bbzheap_tseg_elem_set(bbzheap_tseg_at(s)->values[0], v);
        }
    }
    /* Success */
    return 1;
}

/****************************************/
/****************************************/

uint8_t bbztable_size(bbzheap_idx_t t) {
    /* Get segment index */
    int16_t si = bbzheap_obj_at(t)->t.value;
    /* Get segment data */
    bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
    /* Initialize size to zero */
    uint8_t sz = 0;
    /* Go through elements and segments */
    while(1) {
        /* Count valid keys in the segment */
        for(uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i)
            if(bbzheap_tseg_elem_isvalid(sd->keys[i])) ++sz;
        /* Are we done? */
        if(!bbzheap_tseg_hasnext(sd)) break;
        /* Get next segment */
        si = bbzheap_tseg_next_get(sd);
        sd = bbzheap_tseg_at(si);
    }
    /* Done */
    return sz;
}

/****************************************/
/****************************************/

void bbztable_foreach(bbzheap_idx_t t, bbztable_elem_funp fun, void* params) {
    /* Get segment index */
    int16_t si = bbzheap_obj_at(t)->t.value;
    /* Go through each segment */
    bbzheap_tseg_t* tseg;
    do {
        tseg = bbzheap_tseg_at(si);
        /* Go through each valid element in the segment */
        for (uint8_t i = 0; i < BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (bbzheap_tseg_elem_isvalid(tseg->keys[i])) {
                /* Call function */
                fun(bbzheap_tseg_elem_get(tseg->keys[i]),
                    bbzheap_tseg_elem_get(tseg->values[i]),
                    params);
            }
        }
        si = bbzheap_tseg_next_get(tseg);
    } while (si != BBZHEAP_SEG_NO_NEXT);
}