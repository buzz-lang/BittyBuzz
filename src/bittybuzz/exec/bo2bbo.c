#include <stdio.h>
#include <stdint.h>

#include "bittybuzz/bbzfloat.h"

#define read_write(buf) {\
    fread(&buf,sizeof(buf),1,f_in);\
    fwrite(&buf,sizeof(buf),1,f_out);\
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
        if (f_in) fclose(f_in);
        return 2;
    }

    fseek(f_in, 0, SEEK_END);
    long fsize = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);

    uint16_t str_cnt;
    read_write(str_cnt);
    char charBuf;
    for(int i = 0; i < str_cnt; ++i) {
        do fread(&charBuf,1,1,f_in);
        while (charBuf != 0);
    }
    uint8_t  opcode;
    int32_t  argi;
    float    argf;
    uint16_t bufi;
    do {
        read_write(opcode);
        switch(opcode) {
            case INSTR_NOP: // fallthrough
            case INSTR_DONE: // fallthrough
            case INSTR_PUSHNIL: // fallthrough
            case INSTR_DUP: // fallthrough
            case INSTR_POP: // fallthrough
            case INSTR_RET0: // fallthrough
            case INSTR_RET1: // fallthrough
            case INSTR_ADD: // fallthrough
            case INSTR_SUB: // fallthrough
            case INSTR_MUL: // fallthrough
            case INSTR_DIV: // fallthrough
            case INSTR_MOD: // fallthrough
            case INSTR_POW: // fallthrough
            case INSTR_UNM: // fallthrough
            case INSTR_AND: // fallthrough
            case INSTR_OR: // fallthrough
            case INSTR_NOT: // fallthrough
            case INSTR_EQ: // fallthrough
            case INSTR_NEQ: // fallthrough
            case INSTR_GT: // fallthrough
            case INSTR_GTE: // fallthrough
            case INSTR_LT: // fallthrough
            case INSTR_LTE: // fallthrough
            case INSTR_GLOAD: // fallthrough
            case INSTR_GSTORE: // fallthrough
            case INSTR_PUSHT: // fallthrough
            case INSTR_TPUT: // fallthrough
            case INSTR_TGET: // fallthrough
            case INSTR_CALLC: // fallthrough
            case INSTR_CALLS:
                break;
            case INSTR_PUSHF:
                fread(&argf,sizeof(argf),1,f_in);
                bufi = (uint16_t)argf;
                fwrite(&bufi,sizeof(bufi),1,f_out);
                break;
            case INSTR_PUSHI: // fallthrough
            case INSTR_PUSHS: // fallthrough
            case INSTR_PUSHCN: // fallthrough
            case INSTR_PUSHCC: // fallthrough
            case INSTR_PUSHL: // fallthrough
            case INSTR_LLOAD: // fallthrough
            case INSTR_LSTORE: // fallthrough
            case INSTR_JUMP: // fallthrough
            case INSTR_JUMPZ: // fallthrough
            case INSTR_JUMPNZ:
                fread(&argi,sizeof(argi),1,f_in);
                bufi = (uint16_t)argi;
                fwrite(&bufi,sizeof(bufi),1,f_out);
                if (argi > 65535 || argi < -32768) {
                    fprintf(stderr, "Warning [%s:%d]: Integer (0x%08X) at position %d "
                                    "is out of 16 bit integer range. "
                                    "A part of the data will be lost.\n",
                            argv[1],
                            (int)(ftell(f_in) - sizeof(argi)),
                            argi,
                            (uint32_t) (ftell(f_in) - sizeof(argi)));
                }
                break;
        }
    } while (ftell(f_in) < fsize);

    fclose(f_in);
    fclose(f_out);

    return 0;
}