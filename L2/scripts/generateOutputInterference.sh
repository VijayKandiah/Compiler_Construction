#!/bin/bash

cd tests/interference ; 
for i in *.L2f ; do

  # If the output already exists, skip the current test
  if test -f ${i}.out ; then
    continue ;
  fi
  echo $i ;

  # Generate the binary
  pushd ./ ;
  cd ../../ ;
  ./interference tests/interference/${i} &> tests/interference/${i}.out ;
  cd tests/interference ;
  sed '/^\s*$/d' ${i}.out > ${i}.out2 ;
  sort ${i}.out2 > ${i}.out ;
  rm ${i}.out2 ;
  popd ; 
done
