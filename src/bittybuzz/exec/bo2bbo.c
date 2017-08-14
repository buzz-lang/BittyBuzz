#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bittybuzz/bbzfloat.h"

#define read_write(buf) {\
    (void)fread(&(buf),sizeof(buf),1,f_in);\
    (void)fwrite(&(buf),sizeof(buf),1,f_out);\
}

typedef enum {
    /**
     * Opcodes without argument
     */
    INSTR_NOP = 0,
    INSTR_DONE,
    INSTR_PUSHNIL,
    INSTR_DUP,
    INSTR_POP,
    INSTR_RET0,
    INSTR_RET1,
    INSTR_ADD,
    INSTR_SUB,
    INSTR_MUL,
    INSTR_DIV,
    INSTR_MOD,
    INSTR_POW,
    INSTR_UNM,
    INSTR_AND,
    INSTR_OR,
    INSTR_NOT,
    INSTR_EQ,
    INSTR_NEQ,
    INSTR_GT,
    INSTR_GTE,
    INSTR_LT,
    INSTR_LTE,
    INSTR_GLOAD,
    INSTR_GSTORE,
    INSTR_PUSHT,
    INSTR_TPUT,
    INSTR_TGET,
    INSTR_CALLC,
    INSTR_CALLS,
    /**
     * Opcodes with argument
     */
    INSTR_PUSHF,
    INSTR_PUSHI,
    INSTR_PUSHS,
    INSTR_PUSHCN,
    INSTR_PUSHCC,
    INSTR_PUSHL,
    INSTR_LLOAD,
    INSTR_LSTORE,
    INSTR_JUMP,
    INSTR_JUMPZ,
    INSTR_JUMPNZ
} instr;

typedef struct PACKED darray {
    void* *array;
    size_t used;
    size_t size;
} darray;

void initArray(darray *a, size_t initialSize) {
    a->array = malloc(initialSize * sizeof(void*));
    a->used = 0;
    a->size = initialSize;
}

void insertArray(darray *a, void* element) {
    // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
    // Therefore a->used can go up to a->size
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(void*));
    }
    a->array[a->used++] = element;
}

void freeArray(darray *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

/**
 * Returns 0 if equals, <0 if a < b, >0 if a > b
 */
typedef int8_t (*cmp_func)(void* a, void* b);

typedef struct PACKED dtable {
    darray keys;
    darray values;
    size_t length;
    cmp_func cmp;
} dtable;

void initTable(dtable* t, size_t initialSize, cmp_func func) {
    initArray(&t->keys, initialSize);
    initArray(&t->values, initialSize);
    t->length = 0;
    t->cmp = func;
}

void insertTable(dtable* t, void* key, void* element) {
    insertArray(&t->keys, key);
    insertArray(&t->values, element);
    ++t->length;
}

void setTable(dtable *t, void *key, void *element) {
    for (unsigned int i = 0; i < t->keys.used; ++i) {
        if(t->cmp(t->keys.array[i], key) == 0) {
            t->values.array[i] = element;
            return;
        }
    }
    insertTable(t, key, element);
}

void* getTable(dtable* t, void* key) {
    for (unsigned int i = 0; i < t->keys.used; ++i) {
        if(t->cmp(t->keys.array[i], key) == 0) {
            return t->values.array[i];
        }
    }
    return NULL;
}

void freeTable(dtable* t) {
    freeArray(&t->keys);
    freeArray(&t->values);
    t->length = 0;
    t->cmp = NULL;
}

typedef void (*foreach_func)(void* key, void* value, void* params);

void foreachTable(dtable* t, foreach_func func, void* params) {
    for (unsigned int i = 0; i < t->keys.used; ++i) {
        func(t->keys.array[i], t->values.array[i], params);
    }
}

int8_t cmpint(void* a, void* b) {
    if((int)(intptr_t)a > (int)(intptr_t)b) return 1;
    if((int)(intptr_t)a < (int)(intptr_t)b) return -1;
    return 0;
}

typedef struct PACKED foreachint_params {
    FILE* f_out;
    dtable* refs;
} foreachint_params;

void foreachint(void* key, void* value, void* params) {
    foreachint_params* p = (foreachint_params*)params;
    fseek(p->f_out, (long)key, SEEK_SET);
    int16_t v = (int16_t)(intptr_t)getTable(p->refs, value);
    fwrite(&v, sizeof(v), 1, p->f_out);
    /* Prints corresponding addresses from the input file to the output file on the terminal. */
    //printf("%d => %d\n", (int)(intptr_t)value, (int)v);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Reformat buzz object file in a format compatible with BittyBuzz VM.\n");
        printf("Usage:\n\t%s <buzzbinary.bo> <outputfile.bbo>\n", argv[0]);
        return 1;
    }

    FILE* f_in  = fopen(argv[1], "rb");
    FILE* f_out = fopen(argv[2], "wb");

    if(!f_in) {
        if (f_out) fclose(f_out);
        return 2;
    }
    if(!f_out) {
        fclose(f_in);
        return 2;
    }

    fseek(f_in, 0, SEEK_END);
    long fsize = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);

    dtable refs, repl;
    initTable(&refs, 10, cmpint);
    initTable(&repl, 10, cmpint);

    uint16_t str_cnt;
    read_write(str_cnt);
    char charBuf;
    for(int i = 0; i < str_cnt; ++i) {
        do (void)fread(&charBuf,1,1,f_in);
        while (charBuf != 0);
    }
    uint8_t  opcode;
    int32_t  argi;
    float    argf;
    int16_t  bufi;
    do {
        setTable(&refs, (void*)(intptr_t)(uint32_t)ftell(f_in), (void*)(intptr_t)(int16_t)ftell(f_out));
        read_write(opcode);
        switch(opcode) {
            case INSTR_NOP:     // fallthrough
            case INSTR_DONE:    // fallthrough
            case INSTR_PUSHNIL: // fallthrough
            case INSTR_DUP:     // fallthrough
            case INSTR_POP:     // fallthrough
            case INSTR_RET0:    // fallthrough
            case INSTR_RET1:    // fallthrough
            case INSTR_ADD:     // fallthrough
            case INSTR_SUB:     // fallthrough
            case INSTR_MUL:     // fallthrough
            case INSTR_DIV:     // fallthrough
            case INSTR_MOD:     // fallthrough
            case INSTR_POW:     // fallthrough
            case INSTR_UNM:     // fallthrough
            case INSTR_AND:     // fallthrough
            case INSTR_OR:      // fallthrough
            case INSTR_NOT:     // fallthrough
            case INSTR_EQ:      // fallthrough
            case INSTR_NEQ:     // fallthrough
            case INSTR_GT:      // fallthrough
            case INSTR_GTE:     // fallthrough
            case INSTR_LT:      // fallthrough
            case INSTR_LTE:     // fallthrough
            case INSTR_GLOAD:   // fallthrough
            case INSTR_GSTORE:  // fallthrough
            case INSTR_PUSHT:   // fallthrough
            case INSTR_TPUT:    // fallthrough
            case INSTR_TGET:    // fallthrough
            case INSTR_CALLC:   // fallthrough
            case INSTR_CALLS:
                break;
            case INSTR_PUSHF:
                (void)fread(&argf,sizeof(argf),1,f_in);
                bufi = (uint16_t)bbzfloat_fromfloat(argf);
                fwrite(&bufi,sizeof(bufi),1,f_out);
                break;
            case INSTR_PUSHI:   // fallthrough
            case INSTR_PUSHS:   // fallthrough
            case INSTR_LLOAD:   // fallthrough
            case INSTR_LSTORE:  // fallthrough
                (void)fread(&argi,sizeof(argi),1,f_in);
                bufi = (uint16_t)argi;
                fwrite(&bufi,sizeof(bufi),1,f_out);
                if (argi > INT16_MAX || argi < INT16_MIN) {
                    fprintf(stderr, "Warning [%s:%d]: Integer (0x%08X) at position %d "
                                    "is out of 16 bit integer range. "
                                    "A part of the data will be lost.\n",
                            argv[1],
                            (int)(ftell(f_in) - sizeof(argi)),
                            argi,
                            (uint32_t) (ftell(f_in) - sizeof(argi)));
                }
                break;
            case INSTR_JUMP:    // fallthrough
            case INSTR_JUMPZ:   // fallthrough
            case INSTR_JUMPNZ:  // fallthrough
            case INSTR_PUSHL:   // fallthrough
            case INSTR_PUSHCN:  // fallthrough
            case INSTR_PUSHCC:
                (void)fread(&argi,sizeof(argi),1,f_in);
                insertTable(&repl, (void *) (intptr_t)ftell(f_out), (void *) (intptr_t)argi);
                bufi = (uint16_t)(argi);
                fwrite(&bufi,sizeof(bufi),1,f_out);
                break;
            default:
                fprintf(stderr,"Warning [%s:%d]: Unknown opcode (0x%08X).\n",
                        argv[1],
                        (int)ftell(f_in),
                        opcode);
                break;
        }
    } while (ftell(f_in) < fsize);

    foreachint_params p = {f_out, &refs};
    foreachTable(&repl, foreachint, &p);

    freeTable(&refs);
    freeTable(&repl);
    fclose(f_in);
    fclose(f_out);

    return 0;
}
#pragma GCC diagnostic pop