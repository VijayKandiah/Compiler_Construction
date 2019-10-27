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
ciscs="0" ;
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
    ./a.out &> tests/${i}.out.tmp ;
    cmp tests/${i}.out.tmp tests/${i}.out ;
    if test $? -eq 0 ; then
      didSucceed=1 ;
    fi
  fi
  if test $didSucceed == "1" ; then
    let passed=$passed+1 ;

    cisc=`grep "mem.* +=" prog.L2 | wc -l | awk '{print $1}'` ;
    if test "$cisc" != "0" ; then
      ciscs="1" ;
    fi
    cisc=`grep "mem.* -=" prog.L2 | wc -l | awk '{print $1}'` ;
    if test "$cisc" != "0" ; then
      ciscs="1" ;
    fi
    cisc=`grep "+=.*mem" prog.L2 | wc -l | awk '{print $1}'` ;
    if test "$cisc" != "0" ; then
      ciscs="1" ;
    fi
    cisc=`grep "\-=.*mem" prog.L2 | wc -l | awk '{print $1}'` ;
    if test "$cisc" != "0" ; then
      ciscs="1" ;
    fi
    cisc=`grep "cjump .*<.*" prog.L2 | wc -l | awk '{print $1}'` ;
    if test "$cisc" != "0" ; then
      ciscs="1" ;
    fi
    cisc=`grep "cjump .*>.*" prog.L2 | wc -l | awk '{print $1}'` ;
    if test "$cisc" != "0" ; then
      ciscs="1" ;
    fi
    cisc=`grep "<- mem " prog.L2 | awk '{if ($5 != 0 && $4 != "rsp") print $5}' | wc -l | awk '{print $1}'` ;
    if test "$cisc" != "0" ; then
      ciscs="1" ;
    fi
    cisc=`grep "mem .* <- " prog.L2 | awk '{if ($3 != 0 && $2 != "rsp") print $3}' | wc -l | awk '{print $1}'` ;
    if test "$cisc" != "0" ; then
      ciscs="1" ;
    fi

    cisc=`grep "@" prog.L2 | wc -l | awk '{print $1}'` ;
    if test "$cisc" != "0" ; then
      ciscs="1" ;
    fi
  else
    let failed=$failed+1 ;
    testsFailed="${i} ${testsFailed}";
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
if test "$ciscs" == "0" ; then
  if test "$passed" == "$total" ; then
    echo "While all tests succeeded, you did not generate any complex instruction. "
    exit 0 ;
  fi
fi
echo "Test passed: $passed out of $total"
