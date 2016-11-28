#!/bin/bash
###############################################################################
# install.sh
# Cory R. Thornsberry
# November 28th, 2016
# Install paass executables and shared object files.
#
# Syntax:
#  install.sh [-l <libDirectory> -b <binDirectory> -u -c -h]
#   Available Options:
#    -l : Explicitly specify the directory in which to install .so files.
#          Defaults to ~/lib/.
#    -b : Explicitly specify the directory in which to install executables.
#          Defaults to ~/bin/.
#    -u : Uninstall files.
#    -c : Copy files instead of creating symlinks.
#    -h : Display help dialogue.
#   Error Codes:
#   -1 : Could not find install directories.
#    0 : Executable was built successfully
#    1 : An invalid argument was passed to the script
#    2 : Bin directory does not exist
#    3 : Lib directory does not exist
#    4 : Unknown error.  Control reached the end of the script without exiting
###############################################################################

UNINSTALL=false
MAKECOPIES=false

BIN_DIR="$HOME/bin"
LIB_DIR="$HOME/lib"

LOCAL_BIN_DIR="$PWD/install/bin"
LOCAL_LIB_DIR="$PWD/install/lib"

if [ ! -d $LOCAL_BIN_DIR ]
then
	echo " ERROR! Could not find \""$LOCAL_LIB_DIR"\"!"
	exit -1
fi

if [ ! -d $LOCAL_LIB_DIR ]
then
	echo " ERROR! Could not find \""$LOCAL_LIB_DIR"\"!"
	exit -1
fi

displayHelp() {
	echo " Syntax:"
	echo "  "$0" [-l <libDirectory> -b <binDirectory> -u -c -h]"
	echo "   Available Options:"
	echo "    -l : Explicitly specify the directory in which to install .so files. Defaults to ~/lib/."
	echo "    -b : Explicitly specify the directory in which to install executables. Defaults to ~/bin/."
	echo "    -u : Uninstall files."
	echo "    -c : Copy files instead of creating symlinks."
	echo "    -h : Display this dialogue."
}

installFile() { 
	filename=$(basename $2)
	extension="${filename##*.}"

	if [ "$extension" == "a" ]
	then
		return
	fi

	if [ -f $1/$filename ]
	then
		echo "Warning: "$filename" already exists in \""$1"\"."
		return
	fi

	echo "Installing "$filename" to \""$1"\"."

	if [ "$MAKECOPIES" == "false" ]
	then
		ln -s $2 $1/$filename
	else
		cp $2 $1/$filename
	fi
} 

uninstallFile() { 
	filename=$(basename $2)
	extension="${filename##*.}"

	if [ "$extension" == "a" ]
	then
		return
	fi

	if [ ! -f $1/$filename ]
	then
		echo "Warning: "$filename" does not exist in \""$1"\"."
		return
	fi

	echo "Removing "$filename" from \""$1"\"."

	rm -f $1/$filename
} 

while getopts l:b:uch option
do
	case "${option}"
	in
		l) LIB_DIR=${OPTARG};;
		b) BIN_DIR=${OPTARG};;
		u) UNINSTALL=true;;
		c) MAKECOPIES=true;;
		h) displayHelp; exit 0;;
		*) displayHelp; exit 1;;
        esac
done

if [ ! -d $BIN_DIR ]
then
	echo " ERROR! Directory \""$BIN_DIR"\" does not exist!"
	exit 2
fi

if [ ! -d $LIB_DIR ]
then
	echo " ERROR! Directory \""$LIB_DIR"\" does not exist!"
	exit 3
fi

bin_files=($LOCAL_BIN_DIR/*)
lib_files=($LOCAL_LIB_DIR/*)

if [ "$UNINSTALL" == "false" ]
then
	echo "Installing paass executables to \""$BIN_DIR"\"."
	echo "Installing paass shared libraries to \""$LIB_DIR"\"."

	for file in "${bin_files[@]}"
	do
		installFile $BIN_DIR $file
	done

	for file in "${lib_files[@]}"
	do
		installFile $LIB_DIR $file
	done
else
	echo "Removing paass executables from \""$BIN_DIR"\"."
	echo "Removing paass shared libraries from \""$LIB_DIR"\"."

	for file in "${bin_files[@]}"
	do
		uninstallFile $BIN_DIR $file
	done

	for file in "${lib_files[@]}"
	do
		uninstallFile $LIB_DIR $file
	done
fi

exit 6
