#!/bin/bash

if test $# -lt 1 ; then
  echo "USAGE: `basename $0` LANGUAGE_NUMBER" ;
  exit 1
fi
languageNumber=$1 ;

# Compile the compiler
./bin/setup.sh
pushd ./ ;
cd L${languageNumber} ;
make clean ;
scl enable devtoolset-4 make ;
popd ;

# Create the archive
archiveName="simone.spill.tar.gz" ;
pushd ./ ;
cp -r L${languageNumber} temp ;
cd temp ;
rm -rf `find . -name .git` ;
tar cfz ${archiveName} * ;
mv ${archiveName} ../ ;
popd ;

# Clean
rm -r temp ;
