#!/bin/sh
# File Name       :plotMulWeak.sh
# Description     :Script to plot the speedup of matrix multiplication with weak scaling
# Author          :Karthik Rao
# Date            :Dec 06 2017
# Version         :0.1



PROCS="1"
INTENSITIES="1"
RESULTDIR=result/

if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi


# 1st plot, every node has 1GB of data

for INTENSITY in ${INTENSITIES};
do
	
	for PROC in ${PROCS}
	do  

	    FILE_4=${RESULTDIR}/matMulWeak_23000_4
	    if [ ! -f ${FILE_4} ]
	    then
		echo missing matMulWeak result file "${FILE_4}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_23000_4)
	    echo 4 ${partime}
        
	    FILE_9=${RESULTDIR}/matMulWeak_33000_9
	    if [ ! -f ${FILE_9} ]
	    then
		echo missing matMulWeak result file "${FILE_9}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_33000_9)
	    echo 9 ${partime}

	    FILE_16=${RESULTDIR}/matMulWeak_46000_16
	    if [ ! -f ${FILE_16} ]
	    then
		echo missing matMulWeak result file "${FILE_16}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_46000_16)
	    echo 16 ${partime}


	    FILE_25=${RESULTDIR}/matMulWeak_56000_25
	    if [ ! -f ${FILE_25} ]
	    then
		echo missing matMulWeak result file "${FILE_25}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_56000_25)
	    echo 25 ${partime}

	done > ${RESULTDIR}/time_matMulWeak_ni_1GB

	GNUPLOTWEAK="${GNUPLOTWEAK} set title 'weak scaling. 500MB at every node '; plot '${RESULTDIR}/time_matMulWeak_ni_1GB' u 1:2;"
done





# 2nd plot, every node has 2GB of data

for INTENSITY in ${INTENSITIES};
do
	
	for PROC in ${PROCS}
	do  

	    FILE_4=${RESULTDIR}/matMulWeak_32000_4
	    if [ ! -f ${FILE_4} ]
	    then
		echo missing matMulWeak result file "${FILE_4}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_32000_4)
	    echo 4 ${partime}
        
	    FILE_9=${RESULTDIR}/matMulWeak_47700_9
	    if [ ! -f ${FILE_9} ]
	    then
		echo missing matMulWeak result file "${FILE_9}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_47700_9)
	    echo 9 ${partime}

	    FILE_16=${RESULTDIR}/matMulWeak_64000_16
	    if [ ! -f ${FILE_16} ]
	    then
		echo missing matMulWeak result file "${FILE_16}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_64000_16)
	    echo 16 ${partime}


	    FILE_25=${RESULTDIR}/matMulWeak_79060_25
	    if [ ! -f ${FILE_25} ]
	    then
		echo missing matMulWeak result file "${FILE_25}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_79060_25)
	    echo 25 ${partime}

	done > ${RESULTDIR}/time_matMulWeak_ni_2GB

	GNUPLOTWEAK="${GNUPLOTWEAK} set title 'weak scaling. 1GB at every node '; plot '${RESULTDIR}/time_matMulWeak_ni_2GB' u 1:2;"
done




# 3rd plot, every node has 4GB of data

for INTENSITY in ${INTENSITIES};
do
	
	for PROC in ${PROCS}
	do  

	    FILE_4=${RESULTDIR}/matMulWeak_46000_4
	    if [ ! -f ${FILE_4} ]
	    then
		echo missing matMulWeak result file "${FILE_4}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_46000_4)
	    echo 4 ${partime}
        
	    FILE_9=${RESULTDIR}/matMulWeak_67083_9
	    if [ ! -f ${FILE_9} ]
	    then
		echo missing matMulWeak result file "${FILE_9}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_67083_9)
	    echo 9 ${partime}

	    FILE_16=${RESULTDIR}/matMulWeak_90000_16
	    if [ ! -f ${FILE_16} ]
	    then
		echo missing matMulWeak result file "${FILE_16}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_90000_16)
	    echo 16 ${partime}


	    FILE_25=${RESULTDIR}/matMulWeak_111805_25
	    if [ ! -f ${FILE_25} ]
	    then
		echo missing matMulWeak result file "${FILE_25}". Did you run qHeatWeak and wait for completion?
	    fi
	    partime=$(cat ${RESULTDIR}/matMulWeak_111805_25)
	    echo 25 ${partime}

	done > ${RESULTDIR}/time_matMulWeak_ni_4GB

	GNUPLOTWEAK="${GNUPLOTWEAK} set title 'weak scaling. 2GB at every node '; plot '${RESULTDIR}/time_matMulWeak_ni_4GB' u 1:2;"
done




gnuplot <<EOF
set terminal pdf
set output 'matMul_weak_plots.pdf'

set style data linespoints

set key top left

set xlabel 'Proc'
set ylabel 'Time (in s)'

${GNUPLOTWEAK}



EOF

