#!/bin/bash

passed=0 ;
failed=0 ;
cd tests/interference ; 
for i in *.L2f ; do

  # If the output does not exists, skip the current test
  if ! test -f ${i}.out ; then
    continue ;
  fi
  echo $i ;

  # Test
  pushd ./ ;
  cd ../../ ;
  # Generate the graph
  ./interference tests/interference/${i} &> tests/interference/${i}.out.tmp ;
  cd tests/interference ;
  sed '/^\s*$/d' ${i}.out.tmp > ${i}.out2.tmp 
  sort ${i}.out2.tmp > ${i}.out3.tmp ;
  cmp ${i}.out3.tmp ${i}.out ;
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
