#!/bin/bash

passed=0 ;
failed=0 ;
cd tests/liveness ; 
for i in *.L2f ; do

  # If the output already exists, skip the current test
  if ! test -f ${i}.out ; then
    continue ;
  fi
  echo $i ;

  # Generate the binary
  pushd ./ ;
  cd ../../ ;
  ./liveness tests/liveness/${i} &> tests/liveness/${i}.out.tmp ;
  cmp tests/liveness/${i}.out.tmp tests/liveness/${i}.out ;
  if ! test $? -eq 0 ; then
    echo "  Failed" ;
    let failed=$failed+1 ;
  else
    echo "  Passed" ;
    let passed=$passed+1 ;
  fi
  popd ; 
done
let total=$passed+$failed ;

echo "########## SUMMARY" ;
echo "Test passed: $passed out of $total"
