#!/bin/bash
#
#  dalma-prestart.sh runnumber
#
#   prestart script to start up a dalma process
#
#  Add this script to a ROC process.
#  Have it start "before" the Prestart transition.
#
#  script commandline:
#    dalma-prestart.sh %(rt)
#
#
DALMA_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DALMA_EXE=${DALMA_PATH}/dalma
DALMA_TMPDIR=/tmp/dalma
runnumber=$1
echo "************************************************************"
echo "$0: $1"
echo "************************************************************"

/bin/mkdir -p ${DALMA_TMPDIR}
${DALMA_EXE} -f ${DALMA_TMPDIR}/$runnumber.log
