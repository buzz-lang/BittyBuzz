#include <stdio.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Metaprogram which generates a header file containing raw bytecode.\n");
        printf("The bytecode is available as 'uint8_t bcode[]', and its size is stored as "
               "'uint16_t bcode_size'.\n");
        printf("Usage: \n\tbcodegen <buzzscript.bbo> <outfile.h>\n");
        return 1;
    }

    FILE* f_in  = fopen(argv[1], "rb");
    FILE* f_out = fopen(argv[2], "w");

    if (!f_in) {
        if (f_out) fclose(f_out);
        return 2;
    }
    if (!f_out) {
        if (f_in) fclose(f_in);
        return 2;
    }

    fseek(f_in, 0, SEEK_END);
    int bcode_size = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);

    fprintf(f_out, "#ifndef BCODEGEN_H\n");
    fprintf(f_out, "#define BCODEGEN_H\n\n");
    fprintf(f_out, "#include <avr/pgmspace.h>\n\n");
    fprintf(f_out, "const uint16_t bcode_size = %d;\n\n", bcode_size);
    fprintf(f_out, "PROGMEM // Write bytecode inside the flash\n");
    fprintf(f_out, "const uint8_t bcode[] = {");

    if (bcode_size > 0) {
        char buf;
        fread(&buf, 1, 1, f_in);
        fprintf(f_out, "%d", (int)buf);
        for (int i = 1; i < bcode_size; ++i) {
            fread(&buf, 1, 1, f_in);
            fprintf(f_out, ",%d", (int)buf);
        }
    }

    fprintf(f_out, "};\n");
    fprintf(f_out, "\n#endif // !BCODEGEN_H\n");

    fclose(f_in);
    fclose(f_out);

    return 0;
}