#!/bin/bash

OUTDIR=.tla

if [ -d ${OUTDIR} ]; then
  echo "TLA+ found at ${pwd}/.tla."
  exit 0
fi

TLA_REPO=https://github.com/tlaplus/tlaplus/releases/download
TLA_VERSION=v1.5.7
TLA_ZIP=tla.zip

TLA_URL=${TLA_REPO}/${TLA_VERSION}/${TLA_ZIP}

wget ${TLA_URL} -O ${TLA_ZIP}

if [ -f ${TLA_ZIP} ]; then
  unzip ${TLA_ZIP}
  mv tla .tla
  rm ${TLA_ZIP}
else
  echo "Could not download TLA ${TLA_VERSION}. Try again?"
  exit 99
fi
