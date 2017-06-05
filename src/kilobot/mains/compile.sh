#!/bin/bash

####################
# Global variables #
####################

SRC_DIR="@CMAKE_SOURCE_DIR@"
BIN_DIR="@CMAKE_BINARY_DIR@"
AVR_CFLAGS="@AVR_CFLAGS@"
AVR_LDFLAGS="@AVR_LDFLAGS@"

BBZ_LIB_DIR=${BIN_DIR}/bittybuzz
BBZ_LIB_INC=${SRC_DIR}/bittybuzz
BBZ_LIB_NAME=bittybuzz-kilobot
KILOLIB_DIR=${BIN_DIR}/kilobot/lib
KILOLIB_INC=${SRC_DIR}/kilobot/lib
KILOLIB_NAME=bbzkilobot-kilobot
BO2BBO_PATH=${BIN_DIR}/bittybuzz/exec/bo2bbo
KILO_SYMGEN_PATH=${BIN_DIR}/bittybuzz/exec/kilo_bcodegen

GEN_DIR=${BIN_DIR}/kilobot/mains/gen
GEN_SYMS_FILE=${GEN_DIR}/bbzkilosymgen.h

# Checking options
bzz_file=
bst_file=
cfunction_file=
sourceList=()
verbose="0"

while test $# -gt 0; do
    case "$1" in
        -h|--help)
            echo "$0 - Compile a buzz script for kilobot."
            echo " "
            echo "$0 [options] <c_functions.c> [other_source.c [...]] [other_headers.h [...]]"
            echo " "
            echo "options:"
            echo "-h, --help            show brief help"
            echo "-b, --bzz=FILE.bzz    specify the buzz script to use [required]"
            echo "-B, --bst=FILE.bst    specify the buzz symbol table file to use [optional]"
            exit 0
            ;;
        -V|--verbose)
            verbose="1"
            ;;
        -b)
            shift
            if test $# -gt 0; then
                bzz_file=$1
            else
                echo "no Buzz script specified"
                exit 1
            fi
            shift
            ;;
        --bzz*)
            bzz_file=`echo $1 | sed -e 's/^[^=]*=//g'`
            shift
            ;;
        -B)
            shift
            if test $# -gt 0; then
                bst_file=$1
            else
                echo "no output dir specified"
                exit 1
            fi
            shift
            ;;
        --bst*)
            bst_file=`echo $1 | sed -e 's/^[^=]*=//g'`
            shift
            ;;
        *)
            if [ -z "$cfunction_file" ]; then
                cfunction_file=$1
            else
                sourceList+=($1)
            fi
            shift
            ;;
    esac
done

if [ -z "$cfunction_file" ]; then
    echo >&2 "no main script specified";
    exit 1;
fi

if [ -z "$bzz_file" ]; then
    echo >&2 "no buzz script specified";
    exit 1;
fi

#########################
# Checking dependencies #
#########################

[ "$verbose" = "1" ] && echo "Checking dependencies:"

[ "$verbose" = "1" ] && printf "\tCheck for avr compiler... "
if [ -z "$AVR_CC" ]; then
    hash avr-gcc 2>/dev/null || {
        [ "$verbose" = "1" ] && echo "Not Found";
        echo >&2 "Error: avr-gcc is required but it's not installed.  Aborting.";
        exit 1;
    }
    export AVR_CC=avr-gcc
fi
[ "$verbose" = "1" ] && echo "Done $AVR_CC"

[ "$verbose" = "1" ] && printf "\tCheck for avr-objcopy... "
if [ -z "$AVR_OC" ]; then
    hash avr-objcopy 2>/dev/null || {
        [ "$verbose" = "1" ] && echo "Not Found";
        echo >&2 "Error: avr-objcopy is required but it's not installed.  Aborting.";
        exit 1;
    }
    export AVR_OC=avr-objcopy
fi
[ "$verbose" = "1" ] && echo "Done $AVR_OC"

[ "$verbose" = "1" ] && printf "\tCheck for buzz parser... "
if [ -z "$BZZ_PAR" ]; then
    hash bzzparse 2>/dev/null || {
        [ "$verbose" = "1" ] && echo "Not Found";
        echo >&2 "Error: bzzparse is required but it's not installed.  Aborting.";
        exit 1;
    }
    export BZZ_PAR=bzzparse
fi
[ "$verbose" = "1" ] && echo "Done $BZZ_PAR"

[ "$verbose" = "1" ] && printf "\tCheck for buzz assembler... "
if [ -z "$BZZ_ASM" ]; then
    hash bzzasm 2>/dev/null || {
        [ "$verbose" = "1" ] && echo "Not Found";
        echo >&2 "Error: bzzasm is required but it's not installed.  Aborting.";
        exit 1;
    }
    export BZZ_ASM=bzzasm
fi
[ "$verbose" = "1" ] && echo "Done $BZZ_ASM"

[ "$verbose" = "1" ] && printf "\tCheck for bo2bbo... "
hash $BO2BBO_PATH 2>/dev/null || {
    [ "$verbose" = "1" ] && echo "Not Found";
    echo >&2 "Error: bo2bbo is required but cannot be found. Maybe did you move this compiler script?  Aborting.";
    exit 1;
}
[ "$verbose" = "1" ] && echo "Done $BO2BBO_PATH"

[ "$verbose" = "1" ] && printf "\tCheck for kilo_bcodegen... "
hash $KILO_SYMGEN_PATH 2>/dev/null || {
    [ "$verbose" = "1" ] && echo "Not Found";
    echo >&2 "Error: kilo_bcodegen is required but cannot be found. Maybe did you move this compiler script?  Aborting.";
    exit 1;
}
[ "$verbose" = "1" ] && echo "Done $KILO_SYMGEN_PATH"

[ "$verbose" = "1" ] && printf "\tCheck for BittyBuzz library... "
if [ ! -f "$BBZ_LIB_DIR/lib$BBZ_LIB_NAME.a" ]; then
    [ "$verbose" = "1" ] && echo "Not Found";
    echo >&2 "Error: BittyBuzz library was not found in $BBZ_LIB_DIR."
    echo >&2 "Please edit this file ($0) and place the path to the library.  Aborting.";
    exit 1;
fi
[ "$verbose" = "1" ] && echo "Done $BBZ_LIB_DIR/lib$BBZ_LIB_NAME.a"

[ "$verbose" = "1" ] && printf "\tCheck for KiloLib library... "
if [ ! -f "$KILOLIB_DIR/lib$KILOLIB_NAME.a" ]; then
    [ "$verbose" = "1" ] && echo "Not Found";
    echo >&2 "Error: KiloLib library was not found in $KILOLIB_DIR."
    echo >&2 "Please edit this file ($0) and place the path to the library.  Aborting.";
    exit 1;
fi
[ "$verbose" = "1" ] && echo "Done $KILOLIB_DIR/lib$KILOLIB_NAME.a"

# Checking dependencies done
[ "$verbose" = "1" ] && echo "Checking dependencies Done."
[ "$verbose" = "1" ] && echo " "

#####################
# Compiling Section #
#####################

# start compiling...

mkdir -p ${GEN_DIR}

bbz_filename=$(basename $(realpath ${bzz_file}))
bbz_name="${bbz_filename%.*}"

[ "$verbose" = "1" ] && echo "Parsing ${bzz_file}..."
${BZZ_PAR} ${bzz_file} ${GEN_DIR}/${bbz_name}.basm ${bst_file} || { echo >&2 "ERROR"; exit 1; }
[ "$verbose" = "1" ] && echo "Assembling ${GEN_DIR}/${bbz_name}.basm"
${BZZ_ASM} ${GEN_DIR}/${bbz_name}.basm ${GEN_DIR}/${bbz_name}.bo ${GEN_DIR}/${bbz_name}.bdb || { echo >&2 "ERROR"; exit 1; }
[ "$verbose" = "1" ] && echo "Generating Symbol header file: ${GEN_SYMS_FILE}"
pushd $(dirname ${KILO_SYMGEN_PATH}) > /dev/null
${KILO_SYMGEN_PATH} ${GEN_DIR}/${bbz_name}.bo ${GEN_SYMS_FILE} || { echo >&2 "ERROR"; exit 1; }
popd > /dev/null
[ "$verbose" = "1" ] && echo "Converting .bo to .bbo ..."
${BO2BBO_PATH} ${GEN_DIR}/${bbz_name}.bo ${GEN_DIR}/${bbz_name}.bbo || { echo >&2 "ERROR"; exit 1; }
BBO_SIZE=$(stat -c%s ${GEN_DIR}/${bbz_name}.bbo)
BBO_SIZE_PLUS_2=$((BBO_SIZE + 2))
BOOTLOADER_ADDR=28672
BCODE_SIZE_ADDR=$((BOOTLOADER_ADDR - 2))
BCODE_ADDR=$((BOOTLOADER_ADDR - BBO_SIZE_PLUS_2))
[ "$verbose" = "1" ] && echo "Compiling and Linking c functions..."
${AVR_CC} ${AVR_CFLAGS} -o ${GEN_DIR}/${bbz_name}.elf -I${SRC_DIR} -I${BIN_DIR} -I${GEN_DIR} -I${BBZ_LIB_DIR} -I${KILOLIB_DIR} -I${BBZ_LIB_INC} -I${KILOLIB_INC} ${cfunction_file} ${sourceList[@]} ${GEN_SYMS_FILE} ${AVR_LDFLAGS} -L${BBZ_LIB_DIR} -L${KILOLIB_DIR} -l${BBZ_LIB_NAME} -l${KILOLIB_NAME} -Wl,-T,${SRC_DIR}/kilobot/mains/link.ld -Wl,-Map,${GEN_DIR}/${bbz_name}.map || { echo >&2 "ERROR"; exit 1; }
[ "$verbose" = "1" ] && echo "Generating hex file..."
${AVR_OC} -O ihex -R .eeprom -R .fuse -R .lock -R .signature ${GEN_DIR}/${bbz_name}.elf ${GEN_DIR}/${bbz_name}.hex || { echo >&2 "ERROR"; exit 1; }
[ "$verbose" = "1" ] && echo "Done"
[ "$verbose" = "1" ] && echo " "
[ "$verbose" = "1" ] && echo "Thank you."
[ "$verbose" = "1" ] && echo "Chuck Norris"
exit 0