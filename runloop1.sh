#!/bin/bash

if [ -f relay1.txt ]
then
    rm relay1.txt
fi

for i in `ls dat/*`
do
    echo $i | grep Relay | grep -v ".bin" | cut -d 'y' -f 2 | cut -d ':' -f 1 >> relay1.txt
done

unset array
declare -a array
nof_run=0
while read Relay
do
    array[$nof_run]=${Relay},
    nof_run=$[$nof_run+1]
done < relay1.txt
echo -e "The relay numbers are : \n  { ${array[@]} }"

echo -e "Select a relay number from which you want to start the scan"
read RefRelay

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
	if [ "$Relay" -ge "$RefRelay" ] ; then
	    rname=`echo $ifile | cut -f 3 -d '/' | cut -d 'n' -f 2 | cut -d '_' -f 1`
	    if [ "$rname" -ne "$prevrun" ] ; then
		echo -e "\n\n ============== Relay : $Relay Run : $rname ================= "
		if [ ! -d log1 ] ; then
		    mkdir -p log1
		fi
		./serenity_version.exe $Relay $rname > log1/Relay${Relay}_Run${rname}.log 2>&1
		prevrun=$rname
	    fi
	fi
    done
done < relay1.txt
