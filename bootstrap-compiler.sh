#!/bin/bash

CROSSTOOL=crosstool-ng-1.25.0

if [ ! -f "$CROSSTOOL.tar.bz2" ]; then
  wget wget http://crosstool-ng.org/download/crosstool-ng/$CROSSTOOL.tar.bz2
fi

if [ ! -d "$CROSSTOOL" ]; then
  tar -xf $CROSSTOOL.tar.bz2
fi

cp configs/crosstool.config $CROSSTOOL/.config

WORKING_DIR=$PWD/toolchain
mkdir $WORKING_DIR

cd $CROSSTOOL
if [ ! -f "ct-ng" ]; then
  ./configure --enable-local && make
fi

WORKING_DIR=$WORKING_DIR ./ct-ng build.8