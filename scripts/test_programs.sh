#!/bin/bash

if test $# -lt 2 ; then
  echo "USAGE: `basename $0` EXTENSION_FILE COMPILER" ;
  exit 1;
fi
extFile=$1 ;
compiler=$2 ;

passed=0 ;
failed=0 ;
testNumber=0 ;
testsFailed="" ;
cd tests ; 
for i in *.${extFile} ; do

  # If the output already exists, skip the current test
  if ! test -f ${i}.out ; then
    continue ;
  fi
  echo -n -e "\rTest ${testNumber}: $i                          " ;
  let testNumber=${testNumber}+1 ;

  # Generate the binary
  pushd ./ &> /dev/null ;
  cd ../ ;
  didSucceed=0 ;
  ./${compiler} tests/${i} ;
  if test $? -eq 0 ; then
    valgrind ./a.out &> OUT ;
    numberOfErrors=`grep "???" OUT | wc -l | awk '{print $1}'` ;
    if test "$numberOfErrors" == "0" ; then
      didSucceed=1 ;
    fi
    rm OUT ;
  fi
  if test $didSucceed == "1" ; then
    let passed=$passed+1 ;
  else
    let failed=$failed+1 ;
    testsFailed="${i} ${testsFailed}";
    rm ${i} ${i}.out ;
  fi
  popd &> /dev/null ; 
done
let total=$passed+$failed ;
echo "" ;
echo "" ;

echo "########## SUMMARY" ;
if test "${testsFailed}" != "" ; then
  echo "Failed tests: ${testsFailed}" ;
fi
echo "Test passed: $passed out of $total"
