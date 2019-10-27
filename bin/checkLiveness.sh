#!/bin/bash

# Prepare the sources
./bin/setup.sh

# Find the script
pushd ./ ;
cd ./scripts/322-interps ;
rm -rf tests ;
ln -s ../../tests ./ ;
checkScript="`pwd`/run-test-fests" ;
popd ;

# Compile the compiler
pushd ./ ;
cd L2 ;
make clean ;
scl enable devtoolset-4 make ;
popd ;

# Run
$checkScript --liveness ./L2/liveness -s -t 120 
