#!/bin/bash

# Script to compile and re-open pdf when tex file is modified

pdfname=${1%.*}.pdf
pdflatex $1 #&> /dev/null
evince $pdfname &> /dev/null &

while :
do
  inotifywait -e modify $1
  pdflatex $1
done
