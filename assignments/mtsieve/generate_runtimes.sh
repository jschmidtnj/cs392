#!/bin/bash
#/*******************************************************************************
# * Name   : generate_runtimes.sh
# * Author : Joshua Schmidt and Matt Evanago
# * Date   : 4/16/20
# * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : generate runtimes
# ******************************************************************************/

set -e

command="./mtsieve -s100 -e200000000 -t"
output_file=runtimes.txt

rm -f $output_file
rm -f $working_file
working_file=tmp.txt
cat /proc/cpuinfo | grep 'model name' | sed -n 's/model name\t: //p' > $working_file
num_cores=0
core_info=''
while IFS='' read -r line || [ -n "${line}" ]; do
  num_cores=$((num_cores+1))
  core_info=$line
done < $working_file
rm -f $working_file
num_threads=$((num_cores * 2))

rm -f $output_file

echo "/*******************************************************************************
 * Name   : runtimes.txt
 * Author : Joshua Schmidt and Matt Evanago
 * Date   : 4/16/20
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : multi-threaded seive runtime results
 ******************************************************************************/

Runtimes:
" >> $output_file

cpu_message="cpu: $core_info, num threads:  $num_threads"
echo $cpu_message

for ((i = 1; i <= num_threads; i++))
do
  current_command=$command$i
  res=$({ time $(eval $current_command) >/dev/null; } 2>&1 | grep real | sed -n 's/real\t//p')
  current_output="$current_command : $res"
  echo $current_output
  echo $current_output >> $output_file
done

echo "
Questions:
" >> $output_file
echo $cpu_message >> $output_file
echo "
Time does not scale linearly with the number of threads. After five threads are added, we start to see
the runtime start to increase again, from 1.448 seconds to 1.683 seconds for 8 threads, and then somewhat randomly
increasing and decreasig until it hits 16 threads. There are diminishing returns for adding more threads in this
algorithm, with the largest decrease occuring when going from 1 thread to 2 threads. Then the runtime plateaus, and
becomes chaotic." >> $output_file
