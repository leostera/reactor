#!/bin/bash

readonly PKG=ReActor

readonly DOCS=docs
readonly ODOC=$(which odoc)
readonly LIB=./lib/bs/src

readonly CMT_FILES=$(find ${LIB} -name "*.cmt")
readonly ODOC_FILES=$(echo ${CMT_FILES} | sed "s/cmt/odoc/g")

echo "<< Compiling docs..."
for file in ${CMT_FILES}; do
  ${ODOC} compile \
    -I ${LIB} \
    --pkg=${PKG} \
    ${file}
done
echo ">> Done!"

echo "<< Generating HTML..."
for file in ${ODOC_FILES}; do
  ${ODOC} html \
    -I ${LIB} \
    -o ${DOCS} \
    --semantic-uris \
    ${file}
done
echo ">> Done!"
