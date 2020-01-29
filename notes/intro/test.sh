#! /bin/bash

function print_usage {
  cat << EOF
Usage: junk.sh [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.
EOF
}

help_flag=0
list_flag=0
purge_flag=0
foundopt=false

while getopts ":hlp" option; do
  foundopt=true
  case "$option" in
    h) help_flag=1
        ;;
    l) list_flag=1
        ;;
    p) purge_flag=1
        ;;
    ?) printf "Error: Unknown option '-%s'.\n" $OPTARG >&2
      exit 1
      ;;
  esac
done
shift "$((OPTIND-1))"
declare -a filenames
index=0
for f in $@; do
  filenames[$index]="$f"
  ((++index))
done
if [ $help_flag -eq 1 ] || [ $foundopt = false ]; then
  print_usage
fi
if [ $list_flag -eq 1 ]; then
  echo "list"
fi
if [ $purge_flag -eq 1 ]; then
  echo "purge"
fi
# echo ${filenames[*]}

exit 0
