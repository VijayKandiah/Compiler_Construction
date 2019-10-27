#!/bin/bash

cd tests/spill ; 
for i in *.L2f ; do

  # If the output already exists, skip the current test
  if test -f ${i}.out ; then
    continue ;
  fi
  echo $i ;

  # Generate the binary
  pushd ./ ;
  cd ../../ ;
  ./spill tests/spill/${i} &> tests/spill/${i}.out ;
  sed '/^\s*$/d' tests/spill/${i}.out > tests/spill/${i}.out2 ;
  mv tests/spill/${i}.out2 tests/spill/${i}.out ;
  popd ; 
done
