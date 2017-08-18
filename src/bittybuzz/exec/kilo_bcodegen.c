#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

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

bool is_valid_char(char c) {
    return
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') ||
        c == '_' ||
        c == '\0';
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: \n\tbcodegen <buzzscript.bo> <outfile.h>\n\n\n"


               "Metaprogram which takes a Buzz object (.bo) file generated \n"
               "by the Buzz compiler (bzzc or bzzasm) and generates a header \n"
               "file containing raw bytecode for kilobot programs, as well \n"
               "as macros corresponding to the string ID of strings appearing \n"
               "in the Buzz program.\n\n"

               "The bytecode is available as 'uint8_t bcode[]', and its size \n"
               "is stored as 'uint16_t bcode_size'.\n\n"

               "Given a string, use the 'BBZSTRING_ID' macro defined in \n"
               "\"bbzstring.h\" to get the string ID of a string. Note that \n"
               "all characters that would otherwise make an invalid \n"
               "identifier should be replaced by an underscore (case remains \n"
               "unchanged). Thus, some string names may collide.\n"
               "E.g. \"2 Swarms\" -> BBZSTRING_ID(__Swarms).\n");
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
        fprintf(stderr, "Failed to run bo2bbo on %s\n", basename(argv[1]));
        return 1;
    }

    while(fgets(str, sizeof(str)-1, bo2bbo_proc) != NULL) {
        printf("%s", str);
    }

    fclose(bo2bbo_proc);

    FILE* f_in  = fopen(outFile, "rb");
    if (!f_in) {
        fprintf(stderr, "Cannot open %s\n", outFile);
        return 2;
    }

    FILE* f_out = fopen(argv[2], "w");
    if (!f_out) {
        fprintf(stderr, "Cannot open %s\n", argv[2]);
        return 2;
    }

    fseek(f_in, 0, SEEK_END);
    uintmax_t bcode_size = (uintmax_t)ftell(f_in);
    fseek(f_in, 0, SEEK_SET);

    fprintf(f_out, "#ifndef KILOBCODEGEN_H\n");
    fprintf(f_out, "#define KILOBCODEGEN_H\n\n");
    fprintf(f_out, "#include <inttypes.h>\n");
    fprintf(f_out, "#include <avr/pgmspace.h>\n\n");
    fprintf(f_out, "/*__attribute__((section(\".bcode.data\")))*/ PROGMEM "
            "// Write bytecode inside the flash\n");
    fprintf(f_out, "const uint8_t bcode[] = {");

    if (bcode_size > 0) {
        uint8_t buf;
        fread(&buf, 1, 1, f_in);
        fprintf(f_out, "%" PRIu8, buf);
        for (unsigned int i = 1; i < bcode_size; ++i) {
            fread(&buf, 1, 1, f_in);
            fprintf(f_out, ",%" PRIu8, buf);
        }
        // We make sure that the alignment is on 2 bytes because it will be in the flash and
        // the alignment is needed for the simulator
        if (bcode_size % 2 == 1) {
            fprintf(f_out, ",0");
        }
    }

    fprintf(f_out, "};\n\n");
    fprintf(f_out, "/*__attribute__((section(\".bcode.size\")))*/ PROGMEM __attribute__((used))\n");
    fprintf(f_out, "const uint16_t bcode_size = %d;\n\n", (unsigned int)bcode_size);

    FILE* f_obj = fopen(argv[1], "rb");

    if (!f_obj) {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
        return 2;
    }

    char buf;
    uint16_t strcnt;
    fread(&strcnt, sizeof(strcnt), 1, f_obj);
    for (int i = 0; i < strcnt; ++i) {
        strcpy(str, "");
        do {
            fread(&buf, sizeof(buf), 1, f_obj);
            if (!is_valid_char(buf)) {
                buf = '_';
            }
            strncat(str, &buf, 1);
        } while(buf != '\0');
        if (!strchr(str, ' ')) {
            fprintf(f_out, "#define BBZSTRID_%s %d\n", str, i);
        }
    }

    fprintf(f_out, "\n#endif // !KILOBCODEGEN_H\n");

    fclose(f_in);
    fclose(f_out);
    fclose(f_obj);

    free(outFile);

    return 0;
}