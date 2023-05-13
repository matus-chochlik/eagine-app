#!/bin/bash
function find_dest() {
	find "$(dirname ${0})" \
		-type f \
		-name '*.txt' \
		-size $(stat -c %s "${1:-/dev/zero}")c \
		-exec grep -H -c -e '\([A-F0-9]\)\1' {} \; |\
	grep -v ':0$' |\
	cut -d: -f1 |\
	head -n 1
}

mv "${1}" "$(find_dest ${1})"
