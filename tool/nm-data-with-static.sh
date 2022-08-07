#!/bin/bash
set -u -e -o pipefail

scriptdir="$(dirname "$0")"
elffile="$1"

MDEBUG_LIST_SYMBOLS="${MDEBUG_LIST_SYMBOLS:-${scriptdir}/mdebug_list_symbols}"

awk -f - << 'AWKSCRIPT' <("${MDEBUG_LIST_SYMBOLS}" -s "$elffile") <(nm -n "$elffile") | sort
    FILENAME == ARGV[1] && $1 ~ /[[:xdigit:]]{8}/ && $3 == "scData" { print $1, "d", $5 }
    
    FILENAME == ARGV[2] && $2 == "D" { print }
AWKSCRIPT