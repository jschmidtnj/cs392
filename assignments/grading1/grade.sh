#!/bin/bash
set -e

# variables
grades_file_name="grades.txt"
rm -f $grades_file_name
touch "$grades_file_name"
grades_file="$(pwd)"/$grades_file_name
# functions
process_zip_file() {
  # echo $1
  # this exists only in this function
  # exxtract the name of the file without extension
  local base=${1%.*}
  # echo $base
  # cut expects either file input or data on standard in
  # <<< places the value of the variable on stdin
  # and is called a 'here string'
  local first="$(cut -d '_' -f3 <<<$base)"
  local second="$(cut -d '_' -f2 <<<$base)"
  local authorname="Author: $first $second"
  # local dirname="$first"_"$second"
  echo -e "\n$authorname\n\n" >> $grades_file
}

rm -rf tmp
unzip *.zip -d tmp
cd tmp
for zipfile in *.zip
do
  unzip $zipfile -d tmp
  process_zip_file "$zipfile"
  cp *.sh tmp/grade.sh
  cd tmp
  chmod +x *.sh
  ./grade.sh >> "$grades_file"
  cd - > /dev/null
  rm -rf tmp
done
cd ..
rm -rf tmp
