#!/bin/bash
###############################################################################
# Author: Joshua Schmidt
# Date: 1/29/20
# Pledge: I pledge my honor that I have abided by the Stevens Honor System.
# Description: Junk Bin
###############################################################################

readonly junk_dir=~/.junk

function print_usage {
  cat << EOF
Usage: $(basename "$0") [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.
EOF
}

readonly error_exit_num=1

flag=-1

function check_flag {
  if [ $flag -ne  -1 ]; then
    echo "Error: Too many options enabled."
    print_usage
    exit $error_exit_num
  fi
}

while getopts ":hlp" option; do
  case "$option" in
    h) check_flag
       flag=0
       ;;
    l) check_flag
       flag=1
       ;;
    p) check_flag
       flag=2
       ;;
    ?) printf "Error: Unknown option '-%s'.\n" $OPTARG >&2
      exit $error_exit_num
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
numfiles=${#filenames[*]}
if [ $numfiles -gt 0 ]; then
  check_flag
fi

# after command is verified make the working directory:

if [ ! -d $junk_dir ]; then
  mkdir $junk_dir
fi

# then execute command:

if [ $flag -eq -1 ]; then
  # no flag found
  if [ $numfiles -eq 0 ]; then
    # no files found - print usage
    print_usage
  else
    # delete given files
    for f in "${filenames[@]}"; do
      if [ ! -f "$f" ] && [ ! -d "$f" ]; then
        printf "Warning: '%s' not found.\n" $f >&2
      else
        mv $f $junk_dir;
      fi
    done
  fi
elif [ $flag -eq 0 ]; then
  # help flag
  print_usage
elif [ $flag -eq 1 ]; then
  # list flag
  ls -laF $junk_dir
else
  # purge flag
  rm -rf $junk_dir/*
fi

exit 0
