#!/bin/sh

compiler='bin/tpcc'
logfile='test/results.log'
passed=0
failed=0
# ANSI escape sequences (taken from a script by Fabian Reiter)
reset='\033[0m'
bold='\033[1m'
red='\033[31m'

# Test all files of a folder
test_folder() {
    folder=$1
    expval=$2 # expected exit code
    for file in ${folder}/*; do
        echo $file
        $compiler <$file
        retval=$?
        if [ $retval -eq $expval ]; then
            passed=$(($passed + 1))
            result='OK'
        else
            failed=$(($failed + 1))
            result='FAILED'
            echo "${bold}${red}FAILED${reset}"
        fi
        echo "${file} :\t${result} (got ${retval})" >>$logfile
    done
}

truncate --size=0 $logfile

echo "${bold}== TEST GOOD ==${reset}"
test_folder 'test/good' 0

echo "${bold}\n== TEST SYNTAX ERROR ==${reset}"
test_folder 'test/syn-err' 1

echo "${bold}\n== TEST SEMANTIC ERROR ==${reset}"
test_folder 'test/sem-err' 2

echo "${bold}\n== TEST WARNING ==${reset}"
test_folder 'test/warn' 0

echo "${bold}\n== DETAIL ==${reset}"
cat $logfile

echo "${bold}\n== SUMMARY ==${reset}"
echo "${passed} passed"
echo "${bold}${red}${failed} failed"
echo "${reset}"
