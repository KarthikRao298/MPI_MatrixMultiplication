#!/bin/sh
# File Name       :qMulStrong.sh
# Description     :Script to run the matrix multiplication with strong scaling on the cluster
# Author          :Karthik Rao
# Date            :Dec 06 2017
# Version         :0.1

#. ./params.sh

# for data of 1GB, N=12k
# for data of 20GB, N=21k
# for data of 80GB, N=102k
NS="12000 51000 102000"

PROCS="4 9 16 25"

RESULTDIR=result/



if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi

#strong scaling

    for N in ${NS};
    do	

	for PROC in ${PROCS}
	do
	
	    FILE=${RESULTDIR}/mulStrong_${N}_${PROC}
	    
	    if [ ! -f ${FILE} ]
	    then
		qsub -d $(pwd) -q mamba -l mem=120GB -l procs=${PROC} -v N=${N},PROC=${PROC} ./runMulStrong.sh
	    fi

	done

    done
