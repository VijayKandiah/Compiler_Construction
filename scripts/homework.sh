#!/bin/bash

dirName="322_submission" ;
origDir=`pwd` ;

# Copy all files to an empty directory
if test -d ../$dirName ; then
  echo "ERROR: Please delete ../$dirName" ;
  exit 1 ;
fi
mkdir ../$dirName ;
cd ../$dirName ;
cp -r "${origDir}"/* ./ ;

# Remove unnecessary files
rm -rf .[a-z]* ;
rm -rf .[A-Z]* ;
rm -f */* &> /dev/null ;
rm -rf lib Makefile scripts bin ;
rm -r */tests ;
rm -fr */obj ;
rm -fr */bin ;
rm -r */scripts ;
rm -f */src/*.zip ;
rm -rf `find ./ -name .DS_Store`
rm -f `find ./ -iname *.swp`
for i in `ls` ; do
  if ! test -d $i ; then
    continue ;
  fi

  pushd ./ ;
  cd $i ;
  for j in `ls` ; do
    if test -d $j -a "$j" == "src" ; then
      continue ;
    fi
    rm -rf $j ;
  done
  popd ;
done
find ./ -empty -type d -delete &> /dev/null ;

# Change permissions
chmod 644 */src/*.cpp &> /dev/null ;
chmod 644 -f */src/*.hpp &> /dev/null ;
chmod 644 -f */src/*.h &> /dev/null ;

# Create the package
echo "e149bbb20cd5604b3a7d8c89c2871a802fc2e0d3" > signature.txt ;
tar cfj ../${dirName}.tar.bz2 ./ ;
cd ../ ;
mv ${dirName}.tar.bz2 "${origDir}"/ ;
rm -r ${dirName} ;
