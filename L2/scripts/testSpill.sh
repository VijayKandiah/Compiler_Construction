#!/bin/bash

passed=0 ;
failed=0 ;
cd tests/spill ; 
for i in *.L2f ; do

  # If the output does not exists, skip the current test
  if ! test -f ${i}.out ; then
    continue ;
  fi
  echo $i ;

  # Test
  pushd ./ ;
  cd ../../ ;
  # Generate the binary
  make  
  # Spill
  ./spill tests/spill/${i} &> tests/spill/${i}.out.tmp ;
  # Remove empty lines from the generated file
  sed '/^\s*$/d' tests/spill/${i}.out.tmp > tests/spill/${i}.out2.tmp 
  cmp tests/spill/${i}.out2.tmp tests/spill/${i}.out ;
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
