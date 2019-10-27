#!/bin/bash

cd tests/liveness ; 
for i in *.L2f ; do

  # If the output already exists, skip the current test
  if test -f ${i}.out ; then
    continue ;
  fi
  echo $i ;

  # Generate the binary
  pushd ./ ;
  cd ../../ ;
  ./liveness tests/liveness/${i} &> tests/liveness/${i}.out ;
  popd ; 
done
