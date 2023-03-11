#!/bin/bash

#######################################################################
#
# Copyright (C) 2020-2021 David C. Harrison. All right reserved.
#
# You may not use, distribute, publish, or modify this code without 
# the express written permission of the copyright holder.
#
#######################################################################

(( check = ( RANDOM % 1024 )  + 1 ))
(( gtotal = 0))
(( gpass = 0))
(( apass = 0 ))

echo ""
function alg() {
  flag=$1
  name=$2
  (( total = 0))

  > test.out
  timeout 1s ./pager --$flag"3" -c $check >> test.out
  timeout 1s ./pager --$flag"4" -c $check >> test.out
  pass=`grep "PASS $check" test.out | wc -l`
  if [ "$pass" -lt "2" ]; then
    apass=0
    return
  fi

  > test.out
  for f in {1..100}
  do
    (( total += 1))
    timeout 1s ./pager --$flag -c $check >> test.out
    exit_status=$?
    if [[ $exit_status -eq 124 ]]; then
      echo "FAIL - Timeout" >> test.out
    fi
  done

  pass=`grep "PASS $check" test.out | wc -l`

  if [ "$pass" -lt "0" ]; then
    pass=0
  fi

  (( gtotal += total ))

  pct=0;
  if (( pass > 0 ))
  then
    pct=`echo "scale=2; $pass / $total * 100.0" | bc -l`
  fi

  rm test.out
  (( apass = pass ));
}

(( max = 100 ))

(( apass = 0 ))
alg "fifo"
(( fipass = apass ))
(( gpass += fipass ))
fipct=0;
if (( apass > 0 ))
then
  fipct=`echo "scale=2; $apass / $max * 30.0" | bc -l`
fi
printf "%30s:  %3d/%3d  %6.1f%% of 30%%\n" "First In First Out"    $fipass $max $fipct

(( apass = 0 ))
alg "lfu"
(( lfpass = apass ))
(( gpass += lfpass ))
lfpct=0;
if (( apass > 0 ))
then
  lfpct=`echo "scale=2; $apass / $max * 10.0" | bc -l`
fi
printf "%30s:  %3d/%3d  %6.1f%% of 10%%\n" "Least Frequently Used" $lfpass $max $lfpct

(( apass = 0 ))
alg "lru" 
(( lrpass = apass ))
(( gpass += lfpass ))
lrpct=0;
if (( apass > 0 ))
then
  lrpct=`echo "scale=2; $apass / $max * 10.0" | bc -l`
fi
printf "%30s:  %3d/%3d  %6.1f%% of 10%%\n" "Least Recently Used"   $lrpass $max $lrpct

(( apass = 0 ))
alg "sc" 
(( scpass = apass ))
(( gpass += scpass ))
scpct=0;
if (( apass > 0 ))
then
  scpct=`echo "scale=2; $apass / $max * 20.0" | bc -l`
fi
printf "%30s:  %3d/%3d  %6.1f%% of 20%%\n" "Second Chance"         $scpass $max $scpct

(( apass = 0 ))
alg "cq" 
(( cqpass = apass ))
(( gpass += cqpass ))
cqpct=0;
if (( apass > 0 ))
then
  cqpct=`echo "scale=2; $apass / $max * 20.0" | bc -l`
fi
printf "%30s:  %3d/%3d  %6.1f%% of 20%%\n" "Circular Queue"         $cqpass $max $cqpct

gpct=0;
if (( gpass > 0 ))
then
   gpct=`echo "scale=2; $fipct + $lrpct + $lfpct + $scpct + $cqpct" | bc -l`
fi

echo ""
printf "%30s:   - / -   %6.1f%% of 90%%\n" "Tests" $gpct
