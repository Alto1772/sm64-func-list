#!/bin/bash
set -u -e -o pipefail

scriptdir="$(dirname "$0")"
elffile="$1"

MDEBUG_LIST_SYMBOLS="${MDEBUG_LIST_SYMBOLS:-${scriptdir}/mdebug_list_symbols}"

"$MDEBUG_LIST_SYMBOLS" -s "$elffile" | awk '$1~/[[:xdigit:]]{8}/{if($4=="stProc") print $1, "T", $5; else if($4=="stStaticProc") print $1, "t", $5;}' | sort