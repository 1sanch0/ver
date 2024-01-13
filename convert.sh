#!/bin/bash

# Requires ImageMagick

# input name and output name
input=$1
output=$2

# Make sure the input file exists
if [ ! -f $input ]; then
    echo "Input file $input does not exist"
    exit 1
fi

# Make sure the output file does not exist
if [ -f $output ]; then
    echo "Output file $output already exists"
    exit 1
fi

convert $input -compress none ppm:- | sed '3s/^/#MAX = 1\n/' > $output