#!/bin/bash

#######################################################################
#
# Copyright (C) 2020-2021 David C. Harrison. All right reserved.
#
# You may not use, distribute, publish, or modify this code without 
# the express written permission of the copyright holder.
#
#######################################################################

if [ "$#" -ne 1 ] || ! [ -f "$1" ]; then
  echo "Usage: $0 executable" >&2
  exit 1
fi

speedup=1.7
limit=`echo "scale=2; 1/$speedup" | bc -l`

echo "" 
sort=$1
type=$2
tests=0
passes=0
> test.out
for t in -m
do
  for f in {1..10}
  do
    cnt=`shuf -i 64-128 -n 1`
    ipcs -m | grep $USER > ipcs.out
    if [ -s ipcs.out ]
    then
      ipcrm shm `ipcs -m | grep $USER | awk '{print $2}'` 
    fi
    rm `ls -l /dev/shm | grep $USER | awk '{print $9}'` 2>/dev/null
    ./$sort $t $cnt | tee sort.out
    cat sort.out >> test.out
    ipcs -m | grep $USER | tee ipcs.out
    if [ -s ipcs.out ]
    then
      echo "ERROR: Dangling UNIX shared memory segments!"
      ipcrm shm `ipcs -m | grep $USER | awk '{print $2}'` 
      exit;
    fi
    segs=`ls /dev/shm | grep $USER | wc -l`
    if (( segs > 0 ))
    then
      echo "ERROR: Dangling POSIX shared memory segments!"
      rm `ls -l /dev/shm | grep $USER | awk '{print $9}'` 2>/dev/null
      exit;
    fi
  done
done
echo "" 
pass=`grep PASS test.out | wc -l`
fail=`grep FAIL test.out | wc -l`
(( total = 10 ))
(( tests += total ))
pct=0
if (( pass + fail == total ))
then
  pct=`echo "scale=2; $pass / $total * 100.0" | bc -l`
  (( passes += pass ))
else
  echo "Invalid number of test executions!"
  echo "" 
  pass=0
fi
printf "%20s:  %2d/%2d  %5.1f%%\n" "Functional" $pass $total $pct
echo "" 

> test.out
for f in {1..10}
do
  cnt=`shuf -i 768-1280 -n 1`
  #printf "%4s: " $cnt
  (time ./$sort -s $cnt) &> sort.out
  fail=`grep FAIL sort.out | wc -l`
  if (( fail > 0 ))
  then
    head -1 sort.out
  else 
    ssecs=`tail -3 sort.out | head -1 | tr "m" "\n" | tail -1 | sed 's/.$//'`
    (time ./$sort -m $cnt) &> sort.out
    pass=`grep PASS sort.out | wc -l`
    if (( pass < 1 ))
    then
      head -1 sort.out
    else
      msecs=`tail -3 sort.out | head -1 | tr "m" "\n" | tail -1 | sed 's/.$//'`
      spd=`echo "scale=2; $ssecs / $msecs" | bc -l`
      if [ 1 -eq "$(echo "${spd} <= ${speedup}" | bc)" ]
      then
        echo "   Speedup: FAIL   ($spd times faster <= $speedup)" | tee sort.out
      else
        echo "   Speedup: PASS   ($spd times faster)" | tee sort.out
      fi
    fi
  fi
  cat sort.out >> test.out
done
echo "" 
pass=`grep PASS test.out | wc -l`
fail=`grep FAIL test.out | wc -l`
(( total = 10 ))
(( tests += total ))
(( passes += pass ))
pct=0;
if (( total > 0 ))
then
  pct=`echo "scale=2; $pass / $total * 100.0" | bc -l`
fi
printf "%20s:  %2d/%2d  %5.1f%%\n" "Non-Functional" $pass $total $pct
echo "" 

rm test.out
rm sort.out

pct=0
if (( tests > 0 ))
then
  pct=`echo "scale=2; $passes / $tests * 100.0" | bc -l`
fi
printf "%20s:  %2d/%2d  %5.1f%% of 45%%\n" "Tests" $passes $tests $pct
