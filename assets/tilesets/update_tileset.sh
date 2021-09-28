#!/bin/bash
# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
tileset=${1:-blocks16}

for svg in $(dirname ${0})/${tileset}/*.svg
do inkscape $(< $(dirname ${0})/${tileset}/inkscape_opts) \
	--pipe \
	--export-area-page \
	--export-overwrite \
	--export-type=png \
	--export-filename=${svg%.svg}.png \
	< ${svg}
done
