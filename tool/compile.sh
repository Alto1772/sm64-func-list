#!/bin/bash

CFLAGS=($CFLAGS -Wall -Wextra "${OPTFLAG:--g}" -o "mdebug_list_symbols")
CFILES=("mdebug_list_symbols.c")
CC="${CC:-gcc}"

echo "$CC" "${CFLAGS[@]}" "${CFILES[@]}"
"$CC" "${CFLAGS[@]}" "${CFILES[@]}"
