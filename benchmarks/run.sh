PATH2LIB=~/final_proj/build/ISPRE/ISPRE.so        # Specify your build directory in the project
PASS=-ispre                 # Choose either -fplicm-correctness or -fplicm-performance

# Delete outputs from previous run.
rm -f default.profraw ${1}_prof ${1}_ispre ${1}_no_ispre *.bc ${1}.profdata *_output *.ll

# Convert source code to bitcode (IR)
clang -emit-llvm -c ${1}.c -o ${1}.bc
# Canonicalize natural loops
opt -enable-new-pm=0 -loop-simplify ${1}.bc -o ${1}.ls.bc
# Instrument profiler
opt -enable-new-pm=0 -pgo-instr-gen -instrprof ${1}.ls.bc -o ${1}.ls.prof.bc
# Generate binary executable with profiler embedded
clang -fprofile-instr-generate ${1}.ls.prof.bc -o ${1}_prof

# Generate profiled data
./${1}_prof > correct_output
llvm-profdata merge -o ${1}.profdata default.profraw

# Apply FPLICM
opt -enable-new-pm=0 -o ${1}.ispre.bc -pgo-instr-use -pgo-test-profile-file=${1}.profdata -load ${PATH2LIB} ${PASS} < ${1}.ls.bc > /dev/null

# Generate binary excutable before FPLICM: Unoptimzied code
clang ${1}.ls.bc -o ${1}_no_ispre
# Generate binary executable after FPLICM: Optimized code
clang ${1}.ispre.bc -o ${1}_ispre

# Produce output from binary to check correctness
./${1}_ispre > ispre_output

get_bytes_from_bcanalysis () {
    echo "$1" | sed -n '2p' - | tr '\n' ' ' | sed -e 's/[^0-9]/ /g' -e 's/^ *//g' -e 's/ *$//g' | tr -s ' ' | sed 's/ /\n/g' | sed -n '2p' -  
}


echo -e "\n=== Correctness Check ==="
if [ "$(diff correct_output ispre_output)" != "" ]; then
    echo -e ">> FAIL\n"
else
    echo -e ">> PASS\n"
    # Measure performance
    bcanalyzer_unoptimized="llvm-bcanalyzer ${1}.ls.bc"
    bcanalysis_unoptimized="$($bcanalyzer_unoptimized)"
    bytes_unoptimized=$(get_bytes_from_bcanalysis "${bcanalysis_unoptimized}")

    bcanalyzer_optimized="llvm-bcanalyzer ${1}.ispre.bc"
    bcanalysis_optimized="$($bcanalyzer_optimized)"
    bytes_optimized=$(get_bytes_from_bcanalysis "${bcanalysis_optimized}")

    raw_difference=$((bytes_optimized - bytes_unoptimized))
    percent_difference=$(bc <<< "scale=3 ; $raw_difference / $bytes_unoptimized")
    echo -e "1. Performance of unoptimized code"
    time ./${1}_no_ispre > /dev/null
    echo -e ""
    echo -e "   b. Code size (IR) of unoptimized code\n"
    echo -e "      ${bytes_unoptimized} bytes"
    echo -e "\n\n"  
    echo -e "2. Performance of optimized code"
    time ./${1}_ispre > /dev/null
    echo -e ""
    echo -e "   b. Code size (IR) of optimized code\n"
    echo -e "      ${bytes_optimized} bytes, ${percent_difference}% change\n"
    echo -e "\n\n"
fi

# Cleanup
rm -f default.profraw ${1}_prof ${1}_fplicm ${1}_no_fplicm *.bc ${1}.profdata *_output *.ll
