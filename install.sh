#!/usr/bin/env bash

ME=`whoami`
ROOT="root"
PREFIX="/usr/local"
EXEC="qsismix"
EXECDIR="bin"
ICON="icons/hicolor/qsismix.png"
ICONDIR="share/icons/hicolor/512x512/apps"
DESKTOP="qsismix.desktop"
DESKTOPDIR="/usr/share/applications"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

if [ ! -e $DIR/$EXEC ]; then
    echo $DIR/$EXEC
    echo "could not find compiled program qsismix, has it been built? commands:"
    echo "qmake"
    echo "make"
    echo
    echo "else, did you move this script? it should be in program directory"
    exit
fi

if [ $# -ge "1" ]; then
    if  [ $1 == "-h" ] || [ $1 == "--help" ]
    then
	echo "to change the install prefix, call like"
	echo "sudo $0 \$PREFIX"
	exit
    else
	PREFIX=$1
    fi
    
fi

if [ ! -d $PREFIX ]; then
   echo "$PREFIX is not a valid directory, cannot install there"
   exit
fi

if [ ! -d /usr/$ICONDIR ]; then
    if [ ! -e /usr/$ICONDIR ]; then
	if [ -d /usr/share/icons ]; then
	    mkdir -p /usr/$ICONDIR
	else
	    ICONDIR=share/icons
	    mkdir -p /usr/$ICONDIR
	fi
    else
	ICONDIR=share/icons/hicolor/512x512
    fi
fi

if [ ! -e $DIR/$ICON ]; then
    echo "icon could not be found in $DIR/$ICON"
    echo "will still go ahead and copy executable"
else
    cp $DIR/$ICON /usr/$ICONDIR
fi


if [ ! -d $PREFIX/$EXECDIR ]; then
    mkdir -p $PREFIX/$EXECDIR
fi

cp $DIR/$EXEC $PREFIX/$EXECDIR

if [ -d $DESKTOPDIR ]; then
    cp $DIR/$DESKTOP $DESKTOPDIR
fi

echo
echo "QSixisixmix has been installed successfully"
echo
exit
