#!/bin/bash

set -e

make clean > /dev/null # 2>&1 redirects stderr to stdout
make > /dev/null

./demo & # run in background
pid=$! # get pid for background task

sleep 1
echo "Bash script: Copying dl2.so to dl.so."
cp dl2.so dl1.so

wait $pid # wait for the background task to terminate
