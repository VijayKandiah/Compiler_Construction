#!/bin/bash

if test $# -lt 1 ; then
  echo "USAGE: `basename $0` LANGUAGE_NUMBER" ;
  exit 1
fi
languageNumber=$1 ;

# Prepare the sources
./bin/setup.sh

# Find the script
pushd ./ ;
cd ./scripts/322-interps ;
checkScript="`pwd`/run-test-fests" ;
popd ;

# Compile the compiler
pushd ./ ;
cd L${languageNumber} ;
scl enable devtoolset-4 make ;
popd ;

# Run
$checkScript --L${languageNumber} ./L${languageNumber}/L${languageNumber}c
