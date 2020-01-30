#! /bin/bash

# I pledge my honor that I have abided by the Stevens Honor System.
# - Joshua Schmidt 1/29/20

readonly working_dir=~/.junk

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
if [ $flag -eq -1 ]; then
  for f in "${filenames[@]}"; do
    if [ ! -f "$f" ]; then
      printf "Error: cannot find file '%s'.\n" $f >&2
      exit $error_exit_num
    fi
  done
fi

# after command is verified make the working directory:

if [ ! -d $working_dir ]; then
  mkdir $working_dir
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
      mv $f $working_dir || exit $error_exit_num;
    done
  fi
elif [ $flag -eq 0 ]; then
  # help flag
  print_usage
elif [ $flag -eq 1 ]; then
  # list flag
  ls -laF $working_dir
else
  # purge flag
  rm -rf $working_dir/*
fi

exit 0