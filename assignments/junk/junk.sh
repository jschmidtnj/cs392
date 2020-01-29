#! /bin/bash

usage_string="Usage: junk.sh [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.\n"

# printf "$usage_string"

size_flag=0

function process_args() {
  while getopts ":s" option; do
    case "$option" in
      s) size_flag=1
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
  if [ $size_flag -eq 1 ]; then
    echo "Yay, size flag!"
  fi
  echo ${filenames[*]}
}

process_args

exit 0
