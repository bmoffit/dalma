#!/bin/bash
#
#  dalma-end.sh
#
#   end script to end the dalma process
#
#  Add this script to a ROC process.
#  Have it start "after" the End transition.
#
#  script commandline:
#    dalma-end.sh
#
#
echo "************************************************************"
echo "$0: END"
echo "************************************************************"

/bin/pkill -SIGINT dalma
