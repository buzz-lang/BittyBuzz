#!/bin/bash

# DEPRECATED

####################
# Global variables #
####################

SRC_DIR="@CMAKE_SOURCE_DIR@"
BIN_DIR="@CMAKE_BINARY_DIR@"
PREFIX="@CMD_PREFIX@"
CFLAGS="@CFLAGS@"
LDFLAGS="@LDFLAGS@"
LDSCRIPT="@LDSCRIPT@"
LDLIBS="@LDLIBS@"
BBZ_BASE_BST_FILE="@BBZ_BASE_BST_FILE@"

BBZ_LIB_DIR=${BIN_DIR}/bittybuzz
BBZ_LIB_INC=${SRC_DIR}/bittybuzz
BBZ_LIB_NAME=bittybuzz-crazyflie
CRAZYFLIELIB_DIR=${BIN_DIR}/crazyflie/lib
CRAZYFLIELIB_INC=${SRC_DIR}/crazyflie/lib
CRAZYFLIELIB_NAME=bbzcrazyflie-crazyflie
BO2BBO_PATH=${BIN_DIR}/bittybuzz/exec/bo2bbo
KILO_SYMGEN_PATH=${BIN_DIR}/bittybuzz/exec/crazyflie_bcodegen

GEN_PATH=${BIN_DIR}/crazyflie/behaviors
GEN_SYMS_FILENAME=bbzsymbols.h

# Checking options
bzz_file=
bst_file=
cfunction_file=
sourceList=()
verbose="0"

while test $# -gt 0; do
    case "$1" in
        -h|--help)
            echo "$0 - Compile a buzz script for crazyflie."
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
            shift
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

bbz_filename=$(basename $(realpath ${bzz_file}))
bbz_name="${bbz_filename%.*}"
GEN_DIR=${GEN_PATH}/${bbz_name}
LOG_FILE=${GEN_DIR}/log.txt

LOG() {
    [ "$verbose" = "1" ] && echo "$1";
    echo "$1" >> ${LOG_FILE};
}
LOGF() {
    [ "$verbose" = "1" ] && printf "$1";
    printf "$1" >> ${LOG_FILE};
}

[ "$verbose" = "1" ] && mkdir -v -p ${GEN_DIR} || mkdir -p ${GEN_DIR} | echo

echo "### LOG OF ${bbz_name} COMPILING ###" > ${LOG_FILE}
ERR_STR="[$bbz_name] ERROR. For more detail, check the log at $(realpath --relative-to=${BIN_DIR}/.. ${LOG_FILE})";

#########################
# Checking dependencies #
#########################

LOG "[$bbz_name] Checking dependencies:"

LOGF "\tCheck for $PREFIX compiler... "
if [ -z "$CC" ]; then
    hash $PREFIX-gcc 2>/dev/null || {
        LOG "Not Found";
        echo >&2 "[$bbz_name] Error: $PREFIX-gcc is required but it's not installed.  Aborting.";
        exit 1;
    }
    export CC=$PREFIX-gcc
fi
LOG "Done $CC"

LOGF "\tCheck for $PREFIX-objcopy... "
if [ -z "$OC" ]; then
    hash $PREFIX-objcopy 2>/dev/null || {
        LOG "Not Found";
        echo >&2 "[$bbz_name] Error: $PREFIX-objcopy is required but it's not installed.  Aborting.";
        exit 1;
    }
    export OC=$PREFIX-objcopy
fi
LOG "Done $OC"

LOGF "\tCheck for $PREFIX-objdump... "
if [ -z "$OD" ]; then
    hash $PREFIX-objdump 2>/dev/null || {
        LOG "Not Found";
        echo >&2 "[$bbz_name] Error: $PREFIX-objdump is required but it's not installed.  Aborting.";
        exit 1;
    }
    export OD=$PREFIX-objdump
fi
LOG "Done $OD"

LOGF "\tCheck for buzz parser... "
if [ -z "$BZZ_PAR" ]; then
    hash bzzparse 2>/dev/null || {
        LOG "Not Found";
        echo >&2 "[$bbz_name] Error: bzzparse is required but it's not installed.  Aborting.";
        exit 1;
    }
    export BZZ_PAR=bzzparse
fi
LOG "Done $BZZ_PAR"

LOGF "\tCheck for buzz assembler... "
if [ -z "$BZZ_ASM" ]; then
    hash bzzasm 2>/dev/null || {
        LOG "Not Found";
        echo >&2 "[$bbz_name] Error: bzzasm is required but it's not installed.  Aborting.";
        exit 1;
    }
    export BZZ_ASM=bzzasm
fi
LOG "Done $BZZ_ASM"

LOGF "\tCheck for bo2bbo... "
hash $BO2BBO_PATH 2>/dev/null || {
    LOG "Not Found";
    echo >&2 "[$bbz_name] Error: bo2bbo is required but cannot be found. Did you move this compiler script?  Aborting.";
    exit 1;
}
LOG "Done $(realpath --relative-to=${BIN_DIR}/.. $BO2BBO_PATH)"

LOGF "\tCheck for crazyflie_bcodegen... "
hash $KILO_SYMGEN_PATH 2>/dev/null || {
    LOG "Not Found";
    echo >&2 "[$bbz_name] Error: crazyflie_bcodegen is required but cannot be found. Did you move this compiler script?  Aborting.";
    exit 1;
}
LOG "Done $(realpath --relative-to=${BIN_DIR}/.. $KILO_SYMGEN_PATH)"

LOGF "\tCheck for BittyBuzz library... "
if [ ! -f "$BBZ_LIB_DIR/lib$BBZ_LIB_NAME.a" ]; then
    LOG "Not Found";
    echo >&2 "[$bbz_name] Error: BittyBuzz library was not found in $(realpath --relative-to=${BIN_DIR}/.. $BBZ_LIB_DIR)."
    echo >&2 "[$bbz_name] Please edit this file ($0) and place the path to the library.  Aborting.";
    exit 1;
fi
LOG "Done $(realpath --relative-to=${BIN_DIR}/.. $BBZ_LIB_DIR/lib$BBZ_LIB_NAME.a)"

LOGF "\tCheck for crazyflie library... "
if [ ! -f "$CRAZYFLIELIB_DIR/lib$CRAZYFLIELIB_NAME.a" ]; then
    LOG "Not Found";
    echo >&2 "[$bbz_name] Error: crazyflie library was not found in $(realpath --relative-to=${BIN_DIR}/.. $CRAZYFLIELIB_DIR)."
    echo >&2 "[$bbz_name] Please edit this file ($0) and place the path to the library.  Aborting.";
    exit 1;
fi
LOG "Done $(realpath --relative-to=${BIN_DIR}/.. $CRAZYFLIELIB_DIR/lib$CRAZYFLIELIB_NAME.a)"

# Checking dependencies done
LOG "[$bbz_name] Checking dependencies Done."
LOG ""

#####################
# Compiling Section #
#####################

# start compiling...

GEN_SYMS_DIR=${GEN_DIR}
GEN_SYMS_FILE=${GEN_SYMS_DIR}/${GEN_SYMS_FILENAME}

LOG "[$bbz_name] Linking BST file with BittyBuzz BST file..."
cat ${BBZ_BASE_BST_FILE} > ${GEN_DIR}/${bbz_name}.bst
if [ ! -z "$bst_file" ]; then
    cat ${bst_file} >> ${GEN_DIR}/${bbz_name}.bst
fi
LOG "[$bbz_name] Parsing $(realpath --relative-to=${BIN_DIR}/.. ${bzz_file})..."
${BZZ_PAR} ${bzz_file} ${GEN_DIR}/${bbz_name}.basm ${GEN_DIR}/${bbz_name}.bst >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
LOG "[$bbz_name] Assembling $(realpath --relative-to=${BIN_DIR}/.. ${GEN_DIR}/${bbz_name}.basm)"
${BZZ_ASM} ${GEN_DIR}/${bbz_name}.basm ${GEN_DIR}/${bbz_name}.bo ${GEN_DIR}/${bbz_name}.bdb >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
LOG "[$bbz_name] Generating Symbol header file: $(realpath --relative-to=${BIN_DIR}/.. ${GEN_SYMS_DIR})/${GEN_SYMS_FILENAME}"
pushd $(dirname ${KILO_SYMGEN_PATH}) >> ${LOG_FILE}
${KILO_SYMGEN_PATH} ${GEN_DIR}/${bbz_name}.bo ${GEN_SYMS_FILE} >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
popd >> ${LOG_FILE}
LOG "[$bbz_name] Converting .bo to .bbo ..."
${BO2BBO_PATH} ${GEN_DIR}/${bbz_name}.bo ${GEN_DIR}/${bbz_name}.bbo >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
BBO_SIZE=$(stat -c%s ${GEN_DIR}/${bbz_name}.bbo)
BBO_SIZE_PLUS_2=$((BBO_SIZE + 2))
BOOTLOADER_ADDR=28672
BCODE_SIZE_ADDR=$((BOOTLOADER_ADDR - 2))
BCODE_ADDR=$((BOOTLOADER_ADDR - BBO_SIZE_PLUS_2))
LOG "[$bbz_name] Compiling and Linking c functions..."
${CC} ${CFLAGS} -o ${GEN_DIR}/${bbz_name}.elf -I${SRC_DIR} -I${BIN_DIR} -I${GEN_DIR} -I${GEN_DIR} -I${BBZ_LIB_DIR} -I${CRAZYFLIELIB_DIR} -I${BBZ_LIB_INC} -I${CRAZYFLIELIB_INC} ${cfunction_file} ${sourceList[@]} ${GEN_SYMS_FILE} ${LDSCRIPT} ${LDFLAGS} -L${BBZ_LIB_DIR} -L${CRAZYFLIELIB_DIR} -l${BBZ_LIB_NAME} -l${CRAZYFLIELIB_NAME} -Wl,-Map,${GEN_DIR}/${bbz_name}.map ${LDLIBS} >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
LOG "[$bbz_name] Generating hex file..."
${OC} -O ihex -R .eeprom -R .fuse -R .lock -R .signature ${GEN_DIR}/${bbz_name}.elf ${GEN_DIR}/${bbz_name}.hex >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
LOG "[$bbz_name] Generating debug files... "
${CC} ${CFLAGS/-Wl,-s/} -Wno-deprecated -g -o ${GEN_DIR}/${bbz_name}.elfdbg -I${SRC_DIR} -I${BIN_DIR} -I${GEN_DIR} -I${GEN_DIR} -I${BBZ_LIB_DIR} -I${CRAZYFLIELIB_DIR} -I${BBZ_LIB_INC} -I${CRAZYFLIELIB_INC} ${cfunction_file} ${sourceList[@]} ${GEN_SYMS_FILE} ${LDSCRIPT} ${LDFLAGS//-Wl,-s/} -L${BBZ_LIB_DIR} -L${CRAZYFLIELIB_DIR} -l${BBZ_LIB_NAME} -l${CRAZYFLIELIB_NAME} -Wl,-Map,${GEN_DIR}/${bbz_name}.map ${LDLIBS} >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
${OC} --only-keep-debug ${GEN_DIR}/${bbz_name}.elfdbg ${GEN_DIR}/${bbz_name}.dbg >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
${OC} --strip-debug ${GEN_DIR}/${bbz_name}.elfdbg >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
${OC} --add-gnu-debuglink ${GEN_DIR}/${bbz_name}.dbg ${GEN_DIR}/${bbz_name}.elfdbg >> ${LOG_FILE} || { echo >&2 "${ERR_STR}"; exit 1; }
${OD} -xDSgetl ${GEN_DIR}/${bbz_name}.elfdbg > ${GEN_DIR}/${bbz_name}.lst 2>> ${LOG_FILE}
LOG "[$bbz_name] Done"
LOG ""
LOG "[$bbz_name] Thank you."
exit 0
