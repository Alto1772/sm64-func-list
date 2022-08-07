#!/bin/bash
set -eo pipefail

scriptdir="$(dirname "$0")"

usage() {
    echo "Usage: $0 [-ictsdln] [.] <dcmp_file> <leek_files> [leek_files...]"
    exit $1
}
if [ $# -lt 2 ]; then
    usage 1
fi

indent=2
start_split=1
max_col=50
symtype=T
awkargs=()
files=()

while [ $# -gt 0 ]; do
    case "$1" in
        -i|--indent)
            indent=$2
            shift 2
            ;;
        -c|--max-col)
            max_col=$2
            shift 2
            ;;
        -t|--symbol-type)
            symtype=$2
            shift 2
            ;;
        -s|--start-split)
            start_split=$2
            shift 2
            ;;
        -d|--dcmp-start)
            awkargs+=(dsof=$2)
            shift 2
            ;;
        -l|--leek-start)
            awkargs+=(lsof=$2)
            shift 2
            ;;
        -n|--nm-cmd)
            nm_cmd="$2"
            shift 2
            ;;
        -h|--help)
            usage 0
            ;;
        --)
            files+=("$@")
            break
            ;;
        --*|-*)
            echo "Unknown argument: $1" >&2
            exit 1
            ;;
        *)
            files+=("$1")
            shift
    esac
done

if [ "$symtype" = t -o "$symtype" = T ]; then
    nm_cmd="${nm_cmd:-${scriptdir}/nm-text-with-static.sh}"
elif [ "$symtype" = d -o "$symtype" = D ]; then
    nm_cmd="${nm_cmd:-${scriptdir}/nm-data-with-static.sh}"
elif [ "$symtype" = b -o "$symtype" = B ]; then
    nm_cmd="${nm_cmd:-${scriptdir}/nm-bss-with-static.sh}"
fi
    
awkargs+=(indent=$indent start_split=$start_split sym=$symtype max_col=$max_col)

tmpfiles=()
for file in "${files[@]}"; do
    tmpfile="$(mktemp)"
    tmpfiles+=("$tmpfile")
    #echo "$file = $tmpfile"
    "$nm_cmd" "$file" > "$tmpfile"
done

awk -f - << 'AWKSCRIPT' ${awkargs[@]/#/-v } "${tmpfiles[@]}"
BEGIN {
	# if (!sym) sym = "T"
    sym = toupper(sym)

	# if (!indent) indent = 2
	indentstr = ""
	for (i = 0; i < indent; i++) indentstr = indentstr " "
    
    # if (!start_split) start_split = 1
    for (i = 1; i < ARGC; i++) {
        if (start_split <= i - 1)
            leek_files[i] = ARGV[i]
        else
            dcmp_files[i] = ARGV[i]
    }
    
    if (dsof) split(dsof, dcmp_start_offsets, ",")
    if (lsof) split(lsof, leek_start_offsets, ",")
}

$2 == sym || $2 == tolower(sym) {
	offset = strtonum("0x"$1)
    for (i = 1; i < ARGC; i++) {
        if (FILENAME == ARGV[i]) {
            symname = $3
            if ($2 == tolower(sym)) {
                symname = "[s] " symname
            }
            if (i in dcmp_files) {
                offset += strtonum(dcmp_start_offsets[i])
                dcmp_offsets[offset] = symname
                if (max_col < length(symname)) max_col = length(symname)
            }
            else if (i in leek_files) {
                offset += strtonum(leek_start_offsets[i - start_split])
                leek_offsets[offset] = symname
            }
        }
    }
	if (!(offset in _offsets)) {
        _offsets[offset] = ocount
        offsets[ocount++] = offset
    }
}

END {
    asort(offsets)
	for (i in offsets) {
        offset = offsets[i]
		printf "%s%-*s = %s\n",
		       indentstr, max_col,
		       offset in dcmp_offsets ? dcmp_offsets[offset] : sprintf("? (0x%x)", offset),
		       offset in leek_offsets ? leek_offsets[offset] : sprintf("? (0x%x)", offset)
	}
}
AWKSCRIPT

for file in "${tmpfiles[@]}"; do
    rm "$file"
done