#!/bin/sh

# help output for program
help()
{
    # Display Help
    echo "Helper script to compile a single program using different combinations of LLVM passes and output statistics."
    echo
    echo "Syntax: get_statistics [-h] [-v] source_program [custom_pass]"
    echo "options:"
    echo "   - h     Print this Help."
    echo "   - v     Verbose mode."
    echo "argument:"
    echo "   - source_program    A single .c file to compile" 
    echo "   - custom_pass       If set, a single .cpp file containing an LLVM pass to use as the third pass."
}

# Get command line options
while getopts ":h" option; do
    case $option in
        h) # display help
            help
            exit;;
        \?) # incorrect option
            echo "Error: Invalid option"
            exit 1
    esac
done

# Verify at least one argument (source program) was passed
if [ "$#" -lt 1 ]
then
    echo "*** Missing argument ***"
    echo ""
    help
    exit 1
fi

source_program=$1
custom_pass=${2:-"./nothing.cpp"}

echo $source_program
echo $custom_pass