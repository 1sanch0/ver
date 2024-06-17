#!/bin/bash

# Script to compile and re-open pdf when tex file is modified

compile() {
  pdflatex $1 | biber ${1%.*} | pdflatex $1
}

pdfname=${1%.*}.pdf
# pdflatex $1 #&> /dev/null

compile $1
evince $pdfname &> /dev/null &

while :
do
  inotifywait -e modify $1
  # pdflatex $1
  compile $1
done
