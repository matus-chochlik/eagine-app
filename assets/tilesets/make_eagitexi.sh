#!/bin/bash
# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
tileset=${1:-$(basename $(realpath .))}
rank=4
toolname="png_to_eagitexi.py"
tooldir="$(realpath $(dirname ${0})/../../../../submodules/eagine-oglplus/source/app)"
if [[ -f "${tooldir}/${toolname}" ]]
then convtool="${tooldir}/${toolname}"
else
	tooldir="$(realpath $(dirname ${0})/../../submodules/eagine-oglplus/source/app)"
	if [[ -f "${tooldir}/${toolname}" ]]
	then convtool="${tooldir}/${toolname}"
	else
		echo "Could not find the '${toolname}' tool"
		exit 1
	fi
fi

if [[ "${rank}" -eq 4 ]]
then
	${convtool} \
		-z \
		-i 0.png \
		-i 1.png \
		-i 2.png \
		-i 3.png \
		-i 4.png \
		-i 5.png \
		-i 6.png \
		-i 7.png \
		-i 8.png \
		-i 9.png \
		-i A.png \
		-i B.png \
		-i C.png \
		-i D.png \
		-i E.png \
		-i F.png \
		-o ${tileset}.eagitexi
fi
