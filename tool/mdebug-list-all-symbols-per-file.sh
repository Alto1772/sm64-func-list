#!/bin/bash
set -u -e -o pipefail

scriptdir="$(dirname "$0")"
elffile="$1"

MDEBUG_LIST_SYMBOLS="${MDEBUG_LIST_SYMBOLS:-${scriptdir}/mdebug_list_symbols}"

awk -f - <<'AWKSCRIPT' <("$MDEBUG_LIST_SYMBOLS" -s "$elffile") <("$MDEBUG_LIST_SYMBOLS" -e "$elffile") | sort -k 6 -k 1 | column -t
BEGIN {
    curfdfile = "<unknown>"
}

FILENAME == ARGV[1] {
    if ($0 ~ /^-- File /) {
        curfdfile = substr($3, 2, length($3) - 2)
    }
    else if ($1 ~ /[[:xdigit:]]{8}/) {
        files[curfdfile][length(files[curfdfile])] = $1 " " $2 " " $3 " " $4 " " $5
    }
}

FILENAME == ARGV[2] && $1 ~ /[[:xdigit:]]{8}/ {
    #print NF, $0
    files[$6][length(files[$6])] = $1 " " $2 " " $3 " " $4 " " $5
}

END {
    for (file in files) {
        for (i in files[file]) {
            print files[file][i], file
        }
    }
}
AWKSCRIPT
