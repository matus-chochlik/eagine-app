#!/bin/bash
function find_dest() {
	find "$(dirname ${0})" \
		-type f \
		-name '*.txt' \
		-size ${2}c \
		-exec grep -H -c -e '\([A-F0-9]\)\1' {} \; |\
	grep -v ':0$' |\
	cut -d: -f1 |\
	head -n 1
}

for f
do
	size=$(stat -c %s "${f}") || exit 1
	mv "${f}" "$(find_dest ${f} ${size})" && echo "${f}"
done
