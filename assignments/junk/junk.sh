#! /bin/bash

# I pledge my honor that I have abided by the Stevens Honor System.
# - Joshua Schmidt 1/29/20

working_dir=~/.junk

function print_usage {
  cat << EOF
Usage: junk.sh [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.
EOF
}

flag=-1

function check_flag {
  if [ $flag -ne  -1 ]; then
    echo "Error: Too many options enabled."
    print_usage
    exit 1
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

# after command is verified make the working directory:

if [ ! -d $working_dir ]; then
  mkdir $working_dir
fi

# then execute command:

if [ $flag -eq -1 ]; then
  # no flag found
  if [ ${#filenames[*]} -eq 0 ]; then
    # no files found - print usage
    print_usage
  else
    # delete given files
    for f in "${filenames[@]}"; do
      mv $f $working_dir
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
