#!/bin/bash

# eventdisplay v481 is a 'pure' BDT release, meaning that the code for the regular analysis did not change.
# Thus, the content of the (non-BDT) IRFs did not change, but the filename does (v470 -> v481). 
# To avoid having to re-upload the IRFs to the archive with new names, we provide this script. 
# The user is expected to input 2 directories. The first one is expected to contain the v470 IRFs.
# The second directory will be created if it does not exist. By default, links to the old IRFs will be put in there
# (with the new file name). The user can also specify a file transfer command as the third argument, eg. cp or mv.
# Old and new directory may be identical. The following IRFs are taken care of: Tables EffectiveAreas RadialAcceptances.

if [[ $# < 2 ]]; then
# begin help message
echo "
This is a script to link/copy/mv v470 IRFs to confirm to v481 file name standards.
Usage: $0 <v470_dir> <v481_dir> [command]
where the IRFs are expected in v470_dir/Tables, v470_dir/EffectiveAreas etc and command is eg. \"ln -s\" or \"cp -v\" .
Default is ln -sv.
Old and new directory may be identical.
"
#end help message
exit
fi

OLD=$1
NEW=$2
COM=${3:-"ln -sv"}

for dir in EffectiveAreas RadialAcceptances Tables
do
	mkdir -p $NEW/$dir
	for file in $OLD/$dir/*root 
	do
		name=`basename $file`
		newname=${name/v470/v481}
		if [[ "$name" != "$newname" ]]
		then
			$COM -v $OLD/$dir/$name $NEW/$dir/$newname 
		fi
	done
done

