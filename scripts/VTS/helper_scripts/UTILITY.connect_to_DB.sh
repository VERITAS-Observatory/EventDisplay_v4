#!/bin/bash
# Simple script which connects you to the database
# For use by developers when manually modifying database entries
# written by Nathan Kelley-Hoskins Aug 2013

MYSQLDB=`grep '^\*[ \t]*DBSERVER[ \t]*mysql://' $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter | egrep -o '[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}'`

if [ ! -n "$MYSQLDB" ] ; then
    echo "* DBSERVER param not found in \$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter!"
    exit
#else
#   echo "MYSQLDB: $MYSQLDB"
fi

mysql -u readonly -h $MYSQLDB -A
