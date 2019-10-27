#!/bin/bash

for i in `ls | grep ^L` ; do
  pushd ./ ;
  cd $i ;
  make clean ;
  popd ;
done

rm -rf ./scripts/322-interps/tests ;
