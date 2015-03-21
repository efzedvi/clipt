#!/bin/sh
export XMSG_TIMEOUT=6

export PHASE=$1
export MINUTES=$2
export TTYNAME=$3

if [ $# -ne 3 ]; then
	echo "Invalid number of arguments"
	exit 1;
fi

case $PHASE in
	"b") 
		export TXT="Take a short break for $2 minutes."
		;;
	"w")
		export TXT="Focus on work for $2 minutes, now!"
		;;
	"l")
		export TXT="Take a bit longer break $2 minutes, enjoy!"
		;;
	"q")
		export TXT="Time to go home :)"
		;;
	*)
		#invalid phase indicator
		exit 1
		;;
esac

export XMESSAGE=`which xmessage`

if [ -n "$XMESSAGE" -a -n "$DISPLAY" ]; then
	$XMESSAGE -center -timeout $XMSG_TIMEOUT "$TXT"
else
	mesg y
	
	TXT="----------\n$TXT\n----------";
	echo $TXT | write `whoami` $3  
fi

