#!/bin/bash

OUTDIR=.tla

if [ -d ${OUTDIR} ]; then
  echo "TLA+ found at ${pwd}/.tla"
  exit 0
else
  echo "TLA+ not found. Installing at ${pwd}/.tla"
fi

TLA_REPO=https://github.com/tlaplus/tlaplus/releases/download
TLA_VERSION=v1.5.7
TLA_ZIP=tla.zip

TLA_URL=${TLA_REPO}/${TLA_VERSION}/${TLA_ZIP}

echo "Downloading from ${TLA_URL}..."
wget --quiet ${TLA_URL} -O ${TLA_ZIP}

if [ -f ${TLA_ZIP} ]; then
  echo "Extracting..."
  unzip -qq ${TLA_ZIP}
  mv tla .tla
  rm ${TLA_ZIP}
  echo "Done!"
else
  echo "Could not download TLA ${TLA_VERSION}. Try again?"
  exit 99
fi
