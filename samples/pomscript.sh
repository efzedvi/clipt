#!/bin/bash
echo "$0:$1:$2:$3" >> /tmp/pomo.out
export user=`whoami`

echo "phase $1 for $2 minute starts now" | write $user $3

