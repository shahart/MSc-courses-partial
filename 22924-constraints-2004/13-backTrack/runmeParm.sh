#!/bin/bash

function badOptions
{
 echo "Invalid parameters!"
 echo "usage: ${0} <-d> [-alg algname] -i num <-s filename> -timeout milliseconds -pmin num -pmax num -pinc num -pmin num -pmax num -pinc num"
 echo "-timeout num      - stop after searching for <num> milliseconds"
 echo "-pmin -pmax -pinc - density iterator parameters, -pmin 15 -pmax 60 -pinc 3 will loop from 0.15 to 0.60 incrementing by 0.03"
 echo "-tmin -tmax -tinc - tightness iterator parameters, -pmin 15 -pmax 60 -pinc 3 will loop from 0.15 to 0.60 incrementing by 0.03"
 echo "-i num            - iterate random problem <num> times"
 echo "-s filename       - save all problems to <filename>_d<dens>t<tightness>_<iteration>.txt - optional"
 echo "-oneout           - save all results to one file - results_d_min_max_t_min_max.txt"
 echo "                  - otherwise each density/tightness pair will be saved to the file"
 echo "                    result_d_Density_t_Tightness.txt"
 echo "-d                - use dynamic variable order - actual only for fc*"
 echo "-alg algname      - use <algname> algorithm at least one such option shall be specified"
 echo "                    -alg bt -alg bj -alg cbj -alg fc -alg fc_bj -alg fc_cbj"
 exit -1
}

function parseParameters
{
 while [ -n "$1" ]; do
       case $1 in
            -d )       dynamic='-d' ;;
            -timeout ) shift
                       timeout="-timeout ${1}" ;;
            -i )       shift
                       iterations="-i ${1}" ;;
            -alg )     shift
                       algorithms="$algorithms -alg ${1}" ;;
            -pmin )    shift
                       minDensity="${1}" ;;
            -pmax )    shift
                       maxDensity="${1}"  ;;
            -pinc )    shift
                       densInc="${1}" ;;
            -tmin )    shift
                       minTightness="${1}" ;;
            -tmax )    shift
                       maxTightness="${1}"   ;;
            -tinc )    shift
                       tightInc="${1}"       ;;
            -oneout )  oneout="y" ;;
            -s )       shift
                       problem_file_prefix="${1}" ;;
            * )        echo $1
                       badOptions ;;
       esac
       shift
 done
}

oneout=''
dynamic=''
iterations=''
algorithms=''

if [ $# -lt 12 ]; then
	badOptions
fi

parseParameters $@
base_opt=" ${timeout} ${dynamic} ${iterations} ${algorithms}"
if [ -n ${oneout} ] ; then
   filename=result_v_${dynamic}_d_${minDensity}_${maxDensity}_${densInc}_t_${minTightness}_${maxTightness}_${tightInc}.txt
   cat /dev/null > ${filename}
fi
density=${minDensity}
densDiff=$(echo ${maxDensity} - ${density}|bc)
while [ ${densDiff:0:1} != "-" ] ; do
      tightness=${minTightness}
      tightDiff=$(echo ${maxTightness} - ${tightness}|bc)
      while [ ${tightDiff:0:1} != "-" ]; do

            if [ -n "${problem_file_prefix}" ] ; then
                saveFileOpt=${problem_file_prefix}_d${density}_t${tightness}
            else
                saveFileOpt=''
            fi

            if [ -z ${oneout} ] ; then
               filename=result_v_${dynamic}_d_${density}_t_${tightness}.txt
               cat /dev/null > ${filename}
            fi

            options="${base_opt} ${saveFileOpt} -p ${density} -t ${tightness}"
            #echo "${options} >> $filename"
            ./mmn13 ${options} >> ${filename}
            tightness=$(echo ${tightness} + ${tightInc}|bc)
            tightDiff=$(echo ${maxTightness} - ${tightness}|bc)
      done
	density=$(echo ${density} + ${densInc}|bc)
	densDiff=$(echo ${maxDensity} - ${density}|bc)
done


