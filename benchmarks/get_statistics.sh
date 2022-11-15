#!/bin/bash

# help output for program
help()
{
    # Display Help
    echo "Helper script to compile a single program using different combinations of LLVM passes and output statistics."
    echo
    echo "Syntax: get_statistics [-h] [-d] source_program"
    echo "options:"
    echo "   - h     Print this help."
    echo "   - d     Delete compiled executable files."
    echo "argument:"
    echo "   - source_program    A single .c file to compile and run stats on"
    echo "                       ** Note: omit the .c extension, i.e. \"example.c\" should just be \"example\"" 
    echo "   - custom_pass       If set, a single .so file containing an compiled LLVM pass to use as the third pass."
    echo "                       ** Defaults to ../build/ISPRE/ISPRE.so"
}

# helper function to allow for piping input
get_bytes_from_bcanalysis () {
    echo "$1" | sed -n '2p' - | tr '\n' ' ' | sed -e 's/[^0-9]/ /g' -e 's/^ *//g' -e 's/ *$//g' | tr -s ' ' | sed 's/ /\n/g' | sed -n '2p' -  
}

delete_executables=0
# Get command line options
while getopts ":hd" option; do
    case $option in
        h) # display help
            help
            exit;;
        d) # delete executables
            delete_executables=1;;
        \?) # incorrect option
            echo "Error: Invalid option"
            exit 1;;
    esac
done
# Shift cli arguments to ignore options
shift "$((OPTIND-1))"

# Verify at least one argument (source program) was passed
if [ "$#" -lt 1 ]
then
    echo "*** Missing argument ***"
    echo ""
    help
    exit 1
fi

# Get command line arguments
source_program=$1
ispre_pass=${2:-"../build/ISPRE/ISPRE.so"}

# Delete outputs from any previous runs
rm -f default.profraw ${source_program}_prof ${source_program}_fplicm ${source_program}_no_fplicm *.bc ${source_program}.profdata *_output *.ll

# Convert source code to bitcode (IR)
clang -emit-llvm -c ${source_program}.c -o ${source_program}.bc
# Canonicalize natural loops
opt -enable-new-pm=0 -loop-simplify ${source_program}.bc -o ${source_program}.ls.bc

# Apply extra LLVM pass (second argument with default to ISPRE.so)
opt -enable-new-pm=0 -o ${source_program}.ispre.bc -load ${ispre_pass} < ${source_program}.ls.bc > /dev/null

# Generate binary excutable before ISPRE: Unoptimized code
clang ${source_program}.ls.bc -o ${source_program}_no_ispre
# Generate binary executable after ISPRE: Optimized code
clang ${source_program}.ispre.bc -o ${source_program}_ispre

# Produce output from binary to check correctness
./${source_program}_ispre > ispre_output

echo -e "=== Correctness Check ==="
echo ">> Does the custom pass maintain correct program behavior?"
if [ "$(diff correct_output ispre_output)" != "" ]; then
    echo -e ">> FAIL\n"
else
    echo -e ">> PASS\n"

    bcanalyzer_unoptimized="llvm-bcanalyzer ${source_program}.ls.bc"
    bcanalysis_unoptimized="$($bcanalyzer_unoptimized)"
    bytes_unoptimized=$(get_bytes_from_bcanalysis "${bcanalysis_unoptimized}")

    bcanalyzer_optimized="llvm-bcanalyzer ${source_program}.ispre.bc"
    bcanalysis_optimized="$($bcanalyzer_optimized)"
    bytes_optimized=$(get_bytes_from_bcanalysis "${bcanalysis_optimized}")

    raw_difference=$((bytes_optimized - bytes_unoptimized))
    percent_difference=$(bc <<< "scale=3 ; $raw_difference / $bytes_unoptimized")

    # Measure performance and output size stats
    echo -e "=== Performance Check ==="
    echo -e "1. "
    echo -e "   a. Runtime performance of unoptimized code"
    time ./${source_program}_no_ispre > /dev/null
    echo -e ""
    echo -e "   b. Code size (IR) of unoptimized code\n"
    echo -e "      ${bytes_unoptimized} bytes"
    echo -e "2. "
    echo -e "   a. Runtime performance of optimized code"
    time ./${source_program}_ispre > /dev/null
    echo -e ""
    echo -e "   b. Code size (IR) of optimized code\n"
    echo -e "      ${bytes_optimized} bytes, ${percent_difference}% change\n"
fi

# Cleanup
rm -f default.profraw ${source_program}_prof ${source_program}_fplicm ${source_program}_no_fplicm *.bc ${source_program}.profdata *_output *.ll

if [ "$delete_executables" -eq 1 ]; then
    rm -f ${source_program}_ispre ${source_program}_no_ispre
fi