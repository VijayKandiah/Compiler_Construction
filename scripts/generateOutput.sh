#!/bin/bash

if test $# -lt 2 ; then
  echo "USAGE: `basename $0` EXTENSION_FILE COMPILER" ;
  exit 1;
fi
extFile=$1 ;
compiler=$2 ;

cd tests ;
for i in *.${extFile} ; do

  # If the output already exists, skip the current test
  if test -f ${i}.out ; then
    continue ;
  fi
  echo "Generating the oracle for \"$i\"" ;

  # Generate the binary
  pushd ./ ;
  cd ../ ;
  ./${compiler} tests/${i} ;
  if test $? -ne 0 ; then
    echo "  Compilation error" ;

  else
    ./a.out &> tests/${i}.out ;
    if test $? -ne 0 ; then
      echo "  Runtime error" ;
      rm tests/${i}.out ;
    fi
  fi
  popd ; 

  echo "" ;
done
