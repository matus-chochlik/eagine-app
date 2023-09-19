#!/usr/bin/python
# coding=utf-8
# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt

import os
import re
import sys
import json
import math
import pathlib
import argparse
# ------------------------------------------------------------------------------
class Obj2EAGiMeshArgParser(argparse.ArgumentParser):
    # --------------------------------------------------------------------------
    def __init__(self, **kw):
        argparse.ArgumentParser.__init__(self, **kw)

        self.add_argument(
            '--debug',
            action="store_true",
            default=False
        )
        self.add_argument(
            '--output', '-o',
            dest="output_path",
            metavar="PATH",
            type=os.path.realpath,
            action="store",
            default=None
        )
        self.add_argument(
            '--input', '-i',
            dest="input_path",
            metavar="PATH",
            type=os.path.realpath,
            action="store",
            default=None
        )
        self.add_argument(
            '--name', '-n',
            dest="mesh_name",
            metavar="NAME",
            action="store",
            default=None
        )

        self.add_argument(
            '--patches', '-p',
            dest="patches",
            action="store_true",
            default=False
        )

    # --------------------------------------------------------------------------
    def process_parsed_options(self, options):
        if options.output_path:
            options.prefix = os.path.dirname(options.output_path)
            if not os.path.isdir(options.prefix):
                pathlib.Path(options.prefix).mkdir(parents=True, exist_ok=True)
            options.output = open(options.output_path, "w")
        else:
            options.prefix = None
            options.output = sys.stdout

        if options.mesh_name is None:
            options.mesh_name = os.path.splitext(
                os.path.basename(options.input_path))[0]

        return options

    # --------------------------------------------------------------------------
    def parse_args(self, args):
        return self.process_parsed_options(
            argparse.ArgumentParser.parse_args(self, args)
        )

# ------------------------------------------------------------------------------
def make_argument_parser():
    return Obj2EAGiMeshArgParser(
            prog=os.path.basename(__file__),
            description="""
            OBJ mesh converter script
        """
    )
# ------------------------------------------------------------------------------
class Obj2EAGiMeshOutput(object):
    # --------------------------------------------------------------------------
    def __init__(self, options):
        self._options = options
        self._output = options.output
        self._positions = []
        self._texcoords = []
        self._indices = []

    # --------------------------------------------------------------------------
    def handle_comment(self, text):
        pass

    # --------------------------------------------------------------------------
    def handle_position(self, x, y, z):
        self._positions += [x, y, z]

    # --------------------------------------------------------------------------
    def handle_texcoord(self, u, v):
        self._texcoords += [u, v]

    # --------------------------------------------------------------------------
    def handle_triangle(self, a, b, c):
        self._indices += [a, b, c]

    # --------------------------------------------------------------------------
    def finish(self):
        def _fixnum(n):
            s = str(n)
            return s.rstrip("0").rstrip(".") if s.find(".") else s

        assert len(self._positions) % 3 == 0
        assert len(self._texcoords) % 2 == 0
        assert len(self._indices) % 3 == 0

        vertex_count = len(self._positions) / 3
        assert len(self._texcoords) == 0 or len(self._texcoords) / 2 == vertex_count
        for i in self._indices:
            assert i < vertex_count + 1

        index_count = len(self._indices)
        index_type = "unsigned_16" if index_count < 2^16 else "unsigned_32"
        draw_mode = "patches" if self._options.patches else "triangles"

        self._output.write('{"name":"%s"' % self._options.mesh_name)
        self._output.write('\n,"vertex_count":%d' % vertex_count)
        self._output.write('\n,"index_count":%d' % index_count)
        self._output.write('\n,"index_type":"%s"' % index_type)
        self._output.write('\n,"indices":[%d' % (self._indices[0] - 1))
        for i in self._indices[1:]:
            self._output.write(',%d' % (i - 1))
        self._output.write(']')
        self._output.write('\n,"position":[{"data":[%s' % _fixnum(self._positions[0]))
        for f in self._positions[1:]:
            self._output.write(',%s' % _fixnum(f))
        self._output.write(']}]')
        if len(self._texcoords) > 0:
            self._output.write('\n,"wrap_coord":[{"data":[%s' % _fixnum(self._texcoords[0]))
            for f in self._texcoords[1:]:
                self._output.write(',%s' % _fixnum(f))
            self._output.write(']}]')
        self._output.write('\n,"instructions":[')
        self._output.write('\n{"first":0')
        self._output.write('\n,"count":%d' % index_count)
        self._output.write('\n,"mode":"%s"' % draw_mode)
        self._output.write('\n,"index_type":"%s"' % index_type)
        self._output.write('\n,"cw_face_winding":false')
        self._output.write('\n}]}\n')
        self._output.flush()

# ------------------------------------------------------------------------------
class Obj2EAGiMeshConverter(object):
    # --------------------------------------------------------------------------
    def __init__(self, options):
        self._options = options

        res_float = r"(\s+([+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?))"
        res_unsigned = r"(\s+(\d+))"

        re_position = re.compile("^v" + 3*res_float + "$")
        re_texcoord = re.compile("^vt" + 2*res_float + "$")
        re_triangle = re.compile("^f" + 3*res_unsigned+ "$")
        re_comment = re.compile(r"^#(.*)$")

        self._patterns = [
            (re_position, self.handle_position),
            (re_texcoord, self.handle_texcoord),
            (re_triangle, self.handle_triangle),
            (re_comment,  self.handle_comment)
        ]

    # --------------------------------------------------------------------------
    def handle_position(self, l, m, o):
        o.handle_position(float(m.group(2)), float(m.group(7)), float(m.group(12)))

    # --------------------------------------------------------------------------
    def handle_texcoord(self, l, m, o):
        o.handle_texcoord(float(m.group(2)), float(m.group(7)))

    # --------------------------------------------------------------------------
    def handle_triangle(self, l, m, o):
        o.handle_triangle(int(m.group(2)), int(m.group(4)), int(m.group(6)))

    # --------------------------------------------------------------------------
    def handle_comment(self, l, m, o):
        pass

    # --------------------------------------------------------------------------
    def handle_other(self, l, m, o):
        pass

    # --------------------------------------------------------------------------
    def get_handler(self, line):
        for pattern, handler in self._patterns:
            found = pattern.match(line)
            if found is not None:
                return line, found, handler

        return line, None, self.handle_other

    # --------------------------------------------------------------------------
    def convert(self, input_path, out):
        with open(input_path, "rt") as objin:
            for raw_line in objin.readlines():
                line, mtch, handle = self.get_handler(raw_line.strip())
                handle(line, mtch, out)
            out.finish()

# ------------------------------------------------------------------------------
def main():
    try:
        arg_parser = make_argument_parser()
        options = arg_parser.parse_args(sys.argv[1:])
        if options.debug:
            print(options)
        else:
            converter = Obj2EAGiMeshConverter(options)
            converter.convert(
                options.input_path,
                Obj2EAGiMeshOutput(options))
    except Exception as err:
        print(err)
        raise
# ------------------------------------------------------------------------------
main()


