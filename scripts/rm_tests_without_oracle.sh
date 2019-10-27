#!/bin/bash

if test $# -lt 1 ; then
  echo "USAGE: `basename $0` EXTENSION_FILE" ;
  exit 1;
fi
extFile=$1 ;

cd tests ; 
for i in *.${extFile} ; do
  if test -f ${i}.out ; then
    continue ;
  fi

  echo "Removing $i" ;
  rm ${i} ;
done
