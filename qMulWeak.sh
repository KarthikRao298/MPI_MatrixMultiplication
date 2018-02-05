#!/bin/sh
# File Name       :qMulWeak.sh
# Description     :Script to run the matrix multiplication with weak scaling on the cluster
# Author          :Karthik Rao
# Date            :Dec 06 2017
# Version         :0.1


# weak scaling

RESULTDIR=result/
if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi



# to generate values for p=4, N=46K, 23K, 32K

NS_4="23000 32000 46000"
PROCS_4="4"


    for N in ${NS_4};
    do	

	for PROC in ${PROCS_4}
	do
	
	    FILE=${RESULTDIR}/matMulWeak_${N}_${PROC}
	    
	    if [ ! -f ${FILE} ]
	    then
		qsub -d $(pwd) -q mamba -l mem=120GB -l procs=${PROC} -v N=${N},PROC=${PROC} ./runMulWeak.sh
	    fi

	done

    done




NS_9="33000 47700 67083"
PROCS_9="9"


    for N in ${NS_9};
    do	

	for PROC in ${PROCS_9}
	do
	
	    FILE=${RESULTDIR}/matMulWeak_${N}_${PROC}
	    
	    if [ ! -f ${FILE} ]
	    then
		qsub -d $(pwd) -q mamba -l mem=120GB -l procs=${PROC} -v N=${N},PROC=${PROC} ./runMulWeak.sh
	    fi

	done

    done
    



NS_16="46000 64000 90000"
PROCS_16="16"


    for N in ${NS_16};
    do	

	for PROC in ${PROCS_16}
	do
	
	    FILE=${RESULTDIR}/matMulWeak_${N}_${PROC}
	    
	    if [ ! -f ${FILE} ]
	    then
		qsub -d $(pwd) -q mamba -l mem=120GB -l procs=${PROC} -v N=${N},PROC=${PROC} ./runMulWeak.sh
	    fi

	done

    done



NS_25="56000 79060 111805"
PROCS_25="25"


    for N in ${NS_25};
    do	

	for PROC in ${PROCS_25}
	do
	
	    FILE=${RESULTDIR}/matMulWeak_${N}_${PROC}
	    
	    if [ ! -f ${FILE} ]
	    then
		qsub -d $(pwd) -q mamba -l mem=120GB -l procs=${PROC} -v N=${N},PROC=${PROC} ./runMulWeak.sh
	    fi

	done

    done
    
