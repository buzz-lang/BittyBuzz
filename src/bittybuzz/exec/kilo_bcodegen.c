#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef _WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

/**
 * @brief Removes the "extension" from a file spec.
 * @param[in] mystr The string to process.
 * @param[in] dot The extension separator.
 * @param[in] sep The path separator (0 means to ignore).
 * @return An allocated string identical to the original but
 * with the extension removed. It must be freed when you're
 * finished with it.
 * @note If you pass in NULL or the new string can't be allocated,
 * it returns NULL.
 */
char *remove_ext (char* mystr, char dot, char sep) {
    char *retstr, *lastdot, *lastsep;

    // Error checks and allocate string.

    if (mystr == NULL)
        return NULL;
    if ((retstr = malloc (strlen (mystr) + 1)) == NULL)
        return NULL;

    // Make a copy and find the relevant characters.

    strcpy (retstr, mystr);
    lastdot = strrchr (retstr, dot);
    lastsep = (sep == 0) ? NULL : strrchr (retstr, sep);

    // If it has an extension separator.

    if (lastdot != NULL) {
        // and it's before the extenstion separator.

        if (lastsep != NULL) {
            if (lastsep < lastdot) {
                // then remove it.

                *lastdot = '\0';
            }
        } else {
            // Has extension separator with no path separator.

            *lastdot = '\0';
        }
    }

    // Return the modified string.

    return retstr;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Metaprogram which generates a header file containing raw bytecode for "
                       "bbzkilobot programs.\n");
        printf("The bytecode is available as 'uint8_t bcode[]', and its size is stored as "
                       "'uint16_t bcode_size'.\n");
        printf("Usage: \n\tbcodegen <buzzscript.bo> <outfile.h>\n");
        return 1;
    }

    char str[1024] = "./bo2bbo ";
    char* outFile = remove_ext(basename(argv[1]), '.', PATH_SEP);
    outFile = realloc(outFile, strlen(outFile) + 5);
    strcat(outFile, ".bbo");

    basename(argv[1]);
    strcat(str, argv[1]);
    strcat(str, " ");
    strcat(str, outFile);
    FILE* bo2bbo_proc = popen(str, "r");
    if (!bo2bbo_proc) {
        fprintf(stderr, "Failed to run bo2bbo on %s", basename(argv[1]));
        return 1;
    }

    while(fgets(str, sizeof(str)-1, bo2bbo_proc) != NULL) {
        printf("%s", str);
    }

    fclose(bo2bbo_proc);

    FILE* f_in  = fopen(outFile, "rb");
    FILE* f_out = fopen(argv[2], "w");

    if (!f_in) {
        if (f_out) fclose(f_out);
        return 2;
    }
    if (!f_out) {
        return 2;
    }

    fseek(f_in, 0, SEEK_END);
    uintmax_t bcode_size = (uintmax_t)ftell(f_in);
    fseek(f_in, 0, SEEK_SET);

    fprintf(f_out, "#ifndef KILOBCODEGEN_H\n");
    fprintf(f_out, "#define KILOBCODEGEN_H\n\n");
    fprintf(f_out, "#include <inttypes.h>\n\n");
    fprintf(f_out, "__attribute__((section(\".bcode.data\"))) // Write bytecode inside the flash\n");
    fprintf(f_out, "const uint8_t bcode[] = {");

    if (bcode_size > 0) {
        uint8_t buf;
        fread(&buf, 1, 1, f_in);
        fprintf(f_out, "%" PRIu8, buf);
        for (int i = 1; i < bcode_size; ++i) {
            fread(&buf, 1, 1, f_in);
            fprintf(f_out, ",%" PRIu8, buf);
        }
    }

    fprintf(f_out, "};\n\n");
    fprintf(f_out, "__attribute__((section(\".bcode.size\"))) __attribute__((used))\n");
    fprintf(f_out, "const uint16_t bcode_size = %d;\n\n", (unsigned int)bcode_size);

    FILE* f_obj = fopen(argv[1], "rb");

    if (!f_obj) {
        return 2;
    }

    char buf;
    uint16_t strcnt;
    fread(&strcnt, 2, 1, f_obj);
    for (int i = 0; i < strcnt; ++i) {
        strcpy(str, "");
        do {
            fread(&buf, 1, 1, f_obj);
            strncat(str, &buf, 1);
        } while(buf != '\0');
        if (!strchr(str, ' ')) {
            fprintf(f_out, "#define BBZKILO_SYMID_GEN_%s %d\n", str, i);
        }
    }

    fprintf(f_out, "\n#endif // !KILOBCODEGEN_H\n");

    fclose(f_in);
    fclose(f_out);
    fclose(f_obj);

    free(outFile);

    return 0;
}