# embed_pem.cmake -- converts one PEM file into a generated C source
# exposing it as a byte array + length, so certificate/key content lives
# in a real, syntax-highlighted .pem file instead of a hand-typed C
# string literal (which can't safely hold arbitrary PEM line breaks and
# invites transcription mistakes anyway).
#
# Not included directly -- invoked via `cmake -P embed_pem.cmake` from
# an add_custom_command, with INPUT/OUTPUT/SYMBOL passed as -D
# definitions. See any Cloud_Messaging tutorial's CMakeLists.txt for the
# add_custom_command that calls this.
#
# Copyright (c) 2026 Tiremo
# SPDX-License-Identifier: MIT

if(NOT DEFINED INPUT OR NOT DEFINED OUTPUT OR NOT DEFINED SYMBOL)
    message(FATAL_ERROR "embed_pem.cmake needs -DINPUT=<path.pem> -DOUTPUT=<path.c> -DSYMBOL=<c_identifier>")
endif()

file(READ "${INPUT}" _hex HEX)
string(LENGTH "${_hex}" _hexLen)
math(EXPR _byteCount "${_hexLen} / 2")

# "AABBCC..." -> "0xAA,0xBB,0xCC,..." -- CMake's regex replace works on
# the whole string in one pass, no per-byte loop needed.
string(REGEX REPLACE "(..)" "0x\\1," _bytes "${_hex}")

file(WRITE "${OUTPUT}"
"/* Generated from ${INPUT} by cmake/embed_pem.cmake -- do not edit by hand. */
#include <stddef.h>

const unsigned char ${SYMBOL}[] = { ${_bytes} 0x00 };  /* NUL-terminated for convenience */
const size_t ${SYMBOL}_len = ${_byteCount}u;            /* byte count, NOT including that NUL */
")
