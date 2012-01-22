#!/bin/bash

FILENAME=source/loader/alt_ios_gen.c
GENERATE=0
VERSION=249

if [ ! -z "$1" ];
then
	VERSION=$1
fi

if [ ! -f $FILENAME ];
then
	GENERATE=1
else
	CURRENT_VERSION=`grep mainIOS\ = $FILENAME | awk '{printf "%d", $4}'`
	if [ $CURRENT_VERSION -ne $VERSION ];
	then
		GENERATE=1
	fi
fi

if [ $GENERATE -eq 1 ];
then

	cat <<EOF > $FILENAME
#include "alt_ios.h"
int mainIOS = $VERSION;
EOF
fi