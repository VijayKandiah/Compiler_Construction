#!/bin/bash

function generateTests {

  # Fetch the inputs
  srcLang=$1 ;
  dstLang=$2 ;
  echo "Considering $srcLang" ;

  # Go the to compiler directory
  pushd ./ &> /dev/null ;
  cd $srcLang ;

  # Generate the tests
  added="0" ;
  cd tests ; 
  for i in *.${srcLang} ; do

    # If the output already exists, skip the current test
    if ! test -f ${i}.out ; then
      continue ;
    fi

    # Iterate over the optimization levels
    for optNumber in `seq 0 3` ; do
      optLevel="-O${optNumber}" ;

      # Generate the target language
      pushd ./ &> /dev/null ;
      cd ../ ;
      ./${srcLang}c ${optLevel} tests/${i} ;
      if test $? -ne 0 ; then
        popd &> /dev/null ; 
        continue ;
      fi
      ./a.out &> tests/${i}.out.tmp ;
      cmp tests/${i}.out.tmp tests/${i}.out ;
      if test $? -ne 0 ; then
        popd &> /dev/null ; 
        continue ;
      fi
      echo "  Considering the test $i " ;

      # Check that the current file does not exist already in the tests of the compiler below
      found="0" ;
      pushd ./ &> /dev/null ;
      cd ../$dstLang/tests ;
      for j in *.${dstLang} ; do
        cmp $j ../../${srcLang}/prog.${dstLang} &> /dev/null ;
        if test $? -eq 0 ; then
          found="1" ;
          break ;
        fi
      done
      if test $found == "0" ; then

        # Find a unique name
        num=0 ;
        while true ; do
          testName="test${num}.${dstLang}" ;
          if ! test -f $testName ; then
            break ;
          fi
          let num=$num+1 ;
        done
        echo "    Add a new test named $testName" ;
        cp ../../${srcLang}/prog.${dstLang} $testName ;
        added="1" ;
      fi

      popd &> /dev/null ; 
      popd &> /dev/null ; 
    done
  done
  popd &> /dev/null ; 

  # Generate the oracles
  if test $added == "1" ; then
    echo "  Generate the oracles for the new tests added to $dstLang" ;
    pushd ./ &> /dev/null ;
    cd $dstLang ;
    make oracle ;
    popd &> /dev/null ;
  fi

  return ;
}

generateTests "LB" "LA" ;
generateTests "LA" "IR" ;
generateTests "IR" "L3" ;
generateTests "L3" "L2" ;
generateTests "L2" "L1" ;
