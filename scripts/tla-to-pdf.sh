#!/bin/bash

SPEC_PATH=spec
DOCS_PATH=docs/${SPEC_PATH}/

SPECS=$(find ${SPEC_PATH} -name "*.tla" | sed "s .*\/\(.*\)\.tla \1 g")

PDFLATEX=$(which pdflatex)
JAVA=$(which java)
TLA_PATH=$(pwd)/.tla

if [ ! -f ${PDFLATEX} ]; then
  echo "Make sure pdflatex is on your path!"
  exit 0
fi

if [ ! -d ${OUTDIR} ]; then
  echo "TLA+ not found at ${pwd}/.tla. Please run: make deps"
  exit 0
fi

echo "Printing TLA+ files to TeX..."

for spec in ${SPECS}; do
  ${JAVA} -cp ${TLA_PATH} tla2tex.TLA ${SPEC_PATH}/${spec}.tla
  ${PDFLATEX} ${spec}.tex
  mv ${spec}.pdf ${DOCS_PATH}
  rm ${spec}.*
done

echo "Done!"
