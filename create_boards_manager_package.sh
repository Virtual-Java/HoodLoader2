#!/bin/bash

PATH=$PATH:/bin:/usr/bin:xxx
export PATH

# $1 is path to HOODLOADER2 github root folder
sourcedir=$1
targetdir=$2
VERSION=$3
if [ "$VERSION" != "" ]; then
	targetdir="${VERSION}-boards_manager"
fi
archivname="$targetdir.zip"

paths=("/avr/bootloaders" \
       "/avr/libraries/HoodLoader2/examples" \
       "/avr/variants" \
       "/avr/boards.txt" \
       "/avr/platform.txt" \
       "/gpl.txt" \
       "/header.jpg" \
       "/Readme.md"
)

if ! [[ -d $targetdir ]]; then
	mkdir $targetdir
else
  echo "Target directory $targetdir is already existing."
fi

cp create_boards_manager_package.sh $targetdir
zip -r ${archivname} ${targetdir}

if [[ -d $sourcedir ]]; then
	for PATH in "${paths[@]}"; do
		if [[ -d $1$PATH ]] || [[ -f $1$PATH ]]; then
      echo "Copy path $PATH to target directory $targetdir"
		  cp -r "${1}${PATH}" "$targetdir"
		else
		  echo "Can not find file \"$PATH\" in the given path."
		fi
	done
else
  echo "The path $2 can not be found."
fi

# pack the created target directory for import in boards manager
zip -r ${archivname} ${targetdir}

# optionally remove the targetdir after the zip archiv has been created
#rm -dr $targetdir
