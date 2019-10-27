#!/bin/bash

if test $# -lt 1 ; then
  echo "USAGE: `basename $0` LANGUAGE_NUMBER" ;
  exit 1
fi
languageNumber=$1 ;

pushd ./ ;
cd ../../322-interps ;
interpreter="`pwd`/L${languageNumber}" ;
popd ;

# Fetch the set of source files
cd L${languageNumber}/tests ;
rm -rf ${languageNumber}-test ;
filesToConsider=`ls *` ;

# Create the output directory
mkdir ${languageNumber}-test ;

# Generate the output
for i in ${filesToConsider} ; do
  if test -d $i ; then
    continue ;
  fi

  echo $i ;
  sourceFile="${i}.L${languageNumber}" ;
  cp $i $sourceFile ;
  $interpreter $sourceFile > ${languageNumber}-test/${i}.res 2> /dev/null;
  mv $sourceFile ${languageNumber}-test ;
done

# Create the package
tar cfz simone.${languageNumber}-test.tar.gz ${languageNumber}-test ;

# Clean
rm -rf ${languageNumber}-test ;
