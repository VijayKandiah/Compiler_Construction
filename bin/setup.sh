#!/bin/bash

for i in `ls | grep ^L` ; do
  cd $i ;
  if test -d PEGTL ; then
    cd ../ ;
    continue ;
  fi

  mkdir PEGTL ;
  git clone https://github.com/scampanoni/PEGTL.git ;
  cd ../ ;
done
