#!/bin/bash

if [ -f relay.txt ]
then
    rm relay.txt
fi

for i in `ls dat/*`
do
    echo $i | grep Relay | grep -v ".bin" | cut -d 'y' -f 2 | cut -d ':' -f 1 >> relay.txt
done

# unset array
# declare -a array
# nof_run=0
# while read Relay
# do
#     array[$nof_run]=${Relay},
#     nof_run=$[$nof_run+1]
# done < relay.txt
# echo -e "The relay numbers are : \n  { ${array[@]} }"

# echo -e "Select a relay number "
# read Relay

# prevrun=0
# for ifile in `ls dat/Relay$Relay/*Link0*`
# do
#     rname=`echo $ifile | cut -f 3 -d '/' | cut -d 'n' -f 2 | cut -d '_' -f 1`
#     if [ "$rname" -ne "$prevrun" ] ; then
# 	echo -e "\n\n ============== Relay : $Relay Run : $rname ================= "
# 	./econt_data_validation.exe $Relay $rname
# 	prevrun=$rname
#     fi
# done


prevrun=0
while read Relay
do
    for ifile in `ls dat/Relay$Relay/*Link0*`
    do
	rname=`echo $ifile | cut -f 3 -d '/' | cut -d 'n' -f 2 | cut -d '_' -f 1`
	if [ "$rname" -ne "$prevrun" ] ; then
	    echo -e "\n\n ============== Relay : $Relay Run : $rname ================= "
	    if [ ! -d log ] ; then
		mkdir -p log
	    fi
	    ./econt_data_validation.exe $Relay $rname > log/Relay$Relay_Run$rname.log 2>&1
	    prevrun=$rname
	fi
    done
done < relay.txt
