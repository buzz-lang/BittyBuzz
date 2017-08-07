#include "bbzdarray.h"

/****************************************/
/****************************************/

uint8_t bbzdarray_new(bbzheap_idx_t* d) {
    /* Allocation of a new array */
    if (!bbzheap_obj_alloc(BBZTYPE_TABLE, d)) return 0;
    /* Set the bit that tells it's a dynamic array */
    bbzheap_obj_at(*d)->t.mdata |= BBZTABLE_DARRAY_MASK;
    return 1;
}

/****************************************/
/****************************************/

void bbzdarray_destroy(bbzheap_idx_t d) {
    bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(d);
    uint16_t si = da->value;
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si);
    while (1) {
        tseg_makeinvalid(*sd);
        if (!bbzheap_aseg_hasnext(sd)) break;
        si = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(si);
    }
    obj_makeinvalid(*((bbzobj_t*)da));
}

/****************************************/
/****************************************/

uint8_t bbzdarray_get(bbzheap_idx_t d,
                      uint16_t idx,
                      bbzheap_idx_t* v) {
    if (!bbztype_isdarray(*bbzheap_obj_at(d))) return 0;
    const uint16_t qot = idx / ((uint16_t)(2*BBZHEAP_ELEMS_PER_TSEG)),
            rem = idx % ((uint16_t)(2*BBZHEAP_ELEMS_PER_TSEG));
    uint16_t si = bbzheap_obj_at(d)->t.value; // Segment index
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si); // Segment data
    /* Loop to fetch the last segment */
    for (idx = 0; idx < qot && bbzheap_aseg_hasnext(sd); ++idx) {
        si = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(si);
    }
    if (idx == qot &&
        bbzheap_aseg_elem_isvalid(sd->values[rem])) {
        *v = bbzheap_aseg_elem_get(sd->values[rem]);
        return 1;
    }
    return 0;
}

/****************************************/
/****************************************/

uint8_t bbzdarray_set(bbzheap_idx_t d,
                      uint16_t idx,
                      bbzheap_idx_t v) {
    uint16_t qot = idx / (uint16_t)(2*BBZHEAP_ELEMS_PER_TSEG),
            rem = idx % (uint16_t)(2*BBZHEAP_ELEMS_PER_TSEG);
    uint16_t i = 0;
    uint16_t si = bbzheap_obj_at(d)->t.value; // Segment index
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si); // Segment data
    /* Loop to fetch the last segment */
    for (i = 0; i < qot && bbzheap_aseg_hasnext(sd); ++i) {
        si = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(si);
    }
    if (i == qot &&
        bbzheap_aseg_elem_isvalid(sd->values[rem])) {
        bbzheap_aseg_elem_set(sd->values[rem], v);
        return 1;
    }
    return 0;
}

/****************************************/
/****************************************/

uint8_t bbzdarray_remove(bbzheap_idx_t d, uint16_t idx) { // FIXME
    bbzheap_aseg_t* v = NULL; // The value to remove
    bbzheap_idx_t si = bbzheap_obj_at(d)->t.value; // Segment index
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si); // Segment data
    bbzheap_aseg_t* prevsd = NULL; // To keep track of the previous segment
    /* If the array is empty, return with Failure */
    if (!bbzheap_aseg_hasnext(sd) && !bbzheap_aseg_elem_isvalid(sd->values[0])) {
        return 0;
    }
    /* Loop to fetch the last segment */
    for (uint16_t i = 0; bbzheap_aseg_hasnext(sd); ++i) {
        if (i == idx / (2*BBZHEAP_ELEMS_PER_TSEG)) {
            v = sd;
        }
        si = bbzheap_aseg_next_get(sd);
        prevsd = sd;
        sd = bbzheap_aseg_at(si);
    }
    /* We are now at the last segment */
    /* If the element to remove was not found, return with Failure */
    if (v == NULL) {
        return 0;
    }
    /* Find the last valid element */
    for (si = 0; // Reusing 'si' as the position in the segment
         si < 2*BBZHEAP_ELEMS_PER_TSEG && bbzheap_aseg_elem_isvalid(sd->values[si]);
         ++si);
    if (si > 0) {
        // If a valid element was found,
        // place the last element in place of the element to remove
        // and remove the latter
        bbzheap_aseg_elem_set(v->values[idx % (2*BBZHEAP_ELEMS_PER_TSEG)], bbzheap_aseg_elem_get(sd->values[si - 1]));
        sd->values[si - 1] &= ~MASK_VALID_SEG_ELEM;
    }
    else {
        if (prevsd != NULL) {
            /* If no valid element were found, remove the last one of the previous segment */
            bbzheap_aseg_elem_set(v->values[idx % (2 * BBZHEAP_ELEMS_PER_TSEG)],
                                  bbzheap_aseg_elem_get(sd->values[2 * BBZHEAP_ELEMS_PER_TSEG - 1]));
            prevsd->values[2 * BBZHEAP_ELEMS_PER_TSEG - 1] &= ~MASK_VALID_SEG_ELEM;
            /* Remove the empty segment */
            tseg_makeinvalid(*sd);
            bbzheap_aseg_next_set(prevsd, NO_NEXT);
        }
        else {
            return 0; // Should never be reached.
        }
    }
    return 1;
}

/****************************************/
/****************************************/

uint8_t bbzdarray_pop(bbzheap_idx_t d) {
    bbzheap_idx_t si = bbzheap_obj_at(d)->t.value; // Segment index
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si); // Segment data
    bbzheap_aseg_t* prevsd = NULL; // To keep track of the previous segment
    /* If the array is empty, return with Failure */
    if (!bbzheap_aseg_hasnext(sd) && !bbzheap_aseg_elem_isvalid(sd->values[0])) {
        return 0;
    }
    /* Loop to fetch the last segment */
    while (bbzheap_aseg_hasnext(sd)) {
        si = bbzheap_aseg_next_get(sd);
        prevsd = sd;
        sd = bbzheap_aseg_at(si);
    }
    /* We are now at the last segment */
    /* Find the last valid element */
    for (si = 0; // Reusing 'si' as the position in the segment
         si < 2 * BBZHEAP_ELEMS_PER_TSEG && bbzheap_aseg_elem_isvalid(sd->values[si]);
         ++si);
    if (si > 0) {
        /* If a valid element was found, remove the last one */
        sd->values[si - 1] &= ~MASK_VALID_SEG_ELEM;
    }
    else {
        if (prevsd != NULL) {
            /* If no valid element were found, remove the last one of the previous segment */
            prevsd->values[2*BBZHEAP_ELEMS_PER_TSEG-1] &= ~MASK_VALID_SEG_ELEM;
            /* Remove the empty segment */
            tseg_makeinvalid(*sd);
            bbzheap_aseg_next_set(prevsd, NO_NEXT);
        }
        else {
            return 0; // Should never be reached.
        }
    }
    return 1;
}

/****************************************/
/****************************************/

uint8_t bbzdarray_push(bbzheap_idx_t d,
                       bbzheap_idx_t v) {
    /* Initialisation for the loop */
    uint16_t si = bbzheap_obj_at(d)->t.value; // Segment index
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si); // Segment data
    /* Loop to fetch the last segment */
    while (bbzheap_aseg_hasnext(sd)) {
        si = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(si);
    }
    /* We are now at the last segment */
    /* Find the first free space */
    for (si = 0; // Reusing 'si' as the position in the segment
         si < 2 * BBZHEAP_ELEMS_PER_TSEG && bbzheap_aseg_elem_isvalid(sd->values[si]);
         ++si);

    if (si >= 2*BBZHEAP_ELEMS_PER_TSEG) {
        /* Last segment is full ; add a new segment */
        uint16_t o;
        if (!bbzheap_aseg_alloc(&o)) return 0;
        bbzheap_aseg_next_set(sd, o);
        si = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(si);
        si = 0;
    }

    /* Append value to segment */
    bbzheap_aseg_elem_set(sd->values[si], v);

    return 1;
}

/****************************************/
/****************************************/

uint16_t bbzdarray_size(bbzheap_idx_t d) {
    uint16_t size = 0;
    uint16_t si = bbzheap_obj_at(d)->t.value; // Segment index
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si); // Segment data
    while (1) {
        for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
                bbzheap_aseg_next_set(sd, NO_NEXT);
                return size;
            }
            ++size;
        }
        if (!bbzheap_aseg_hasnext(sd)) break;
        si = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(si);
    }
    return size;
}

/****************************************/
/****************************************/

uint8_t bbzdarray_clone(bbzheap_idx_t d,
                        bbzheap_idx_t* newd) {
    if(!bbzdarray_new(newd)) return 0;
    uint16_t si = bbzheap_obj_at(d)->t.value; // Segment index
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si); // Segment data
    while (1) {
        for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
                break;
            }
            if (!bbzdarray_push(*newd, bbzheap_aseg_elem_get(sd->values[i]))) return 0;
        }
        if (!bbzheap_aseg_hasnext(sd)) break;
        si = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(si);
    }
    return 1;
}

/****************************************/
/****************************************/

void bbzdarray_clear(bbzheap_idx_t d) {
    uint16_t i;
    bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(d); // darray
    bbzheap_aseg_t* sd = bbzheap_aseg_at(da->value); // Segment data
    /* If the array is empty, it's already cleared! */
    if (!bbzheap_aseg_hasnext(sd) && !bbzheap_aseg_elem_isvalid(sd->values[0])) {
        return;
    }
    /* Loop to fetch the last segment */
    while (bbzheap_aseg_hasnext(sd)) {
        tseg_makeinvalid(*sd);
        da->value = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(da->value);
    }
    /* We are now at the last segment */
    /* Invalidate all valid elements in the segment */
    for (i = 0; // Reusing 'i' as the position in the segment
         i < 2 * BBZHEAP_ELEMS_PER_TSEG && bbzheap_aseg_elem_isvalid(sd->values[i]);
         ++i) {
        sd->values[i] &= ~MASK_VALID_SEG_ELEM;
    }
}

/****************************************/
/****************************************/

void bbzdarray_foreach(bbzheap_idx_t d,
                       bbzdarray_elem_funp fun,
                       void* params) {
    uint16_t si = bbzheap_obj_at(d)->t.value; // Segment index
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si); // Segment data
    while (1) {
        for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
            if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
                return;
            }
            fun(d, bbzheap_aseg_elem_get(sd->values[i]), params);
        }
        if (!bbzheap_aseg_hasnext(sd)) break;
        si = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(si);
    }
}

/****************************************/
/****************************************/

uint16_t bbzdarray_find(bbzheap_idx_t d,
                        bbzdarray_elem_cmpp cmp,
                        bbzheap_idx_t data) {
    uint16_t pos = 0;
    bbzdarray_t* da = (bbzdarray_t*)bbzheap_obj_at(d);
    uint16_t si = da->value;
    bbzheap_aseg_t* sd = bbzheap_aseg_at(si);
    /* Go through the darray segments */
    while (1) {
        /* Go through the subelements of the segment */
        for (uint16_t i = 0; i < 2*BBZHEAP_ELEMS_PER_TSEG; ++i) {
            /* No more elements? */
            if (!bbzheap_aseg_elem_isvalid(sd->values[i])) {
                break;
            }
            /* Element found? */
            if (bbzheap_obj_isvalid(*bbzheap_obj_at(bbzheap_aseg_elem_get(sd->values[i]))) &&
                cmp(bbzheap_obj_at(bbzheap_aseg_elem_get(sd->values[i])), bbzheap_obj_at(data)) == 0) {
                return pos;
            }
            ++pos;
        }
        /* Go to next segment */
        if (!bbzheap_aseg_hasnext(sd)) break;
        si = bbzheap_aseg_next_get(sd);
        sd = bbzheap_aseg_at(si);
    }
    return pos;
}

/****************************************/
/****************************************/

uint8_t bbzdarray_lambda_alloc(bbzheap_idx_t d, uint8_t* l) {
    /* Look for empty slot */
    for(uint8_t i = 0;
        i < RESERVED_ACTREC_MAX;
        ++i)
        if(!bbzheap_obj_isvalid(*bbzheap_obj_at(i))) {
            /* Empty slot found */
            bbzobj_t* x = bbzheap_obj_at(i);
            /* Set valid bit and type */
            obj_makevalid(*x);
            bbztype_cast(*x, BBZTYPE_TABLE);
            /* Set the bit that tells it's a dynamic array */
            x->t.mdata |= BBZTABLE_DARRAY_MASK;
            /* Set result */
            *l = i;
            /* Allocate an array segment */
            if(!bbzheap_aseg_alloc(&(x->t.value))) return 0;
            uint16_t idx = bbzdarray_size(d);
            uint16_t v;
            for (uint16_t j = 0; j < idx; ++j) {
                bbzdarray_get(d, j, &v);
                if (!bbzdarray_push(*l, v)) return 0;
            }
            /* Success */
            return 1;
        }
    /* No empty slot found, we're out of reserved memory! */
    return 0;
}