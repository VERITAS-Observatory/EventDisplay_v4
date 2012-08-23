#!/bin/zsh

source $EVNDISPSYS/setObservatory.sh VTS

cd $EVNDISPSYS/bin/

./VTS.getRunListFromDB $*
