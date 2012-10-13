#!/bin/bash
#
echo buildtype.sh

FILENAME=source/loader/alt_ios_gen.h
cat <<EOF > $FILENAME
#define DOL_MAIN_IOS $1
EOF
