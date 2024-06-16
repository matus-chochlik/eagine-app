#!/usr/bin/env python3
# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt

import os
import sys
import math
import scipy
import argparse
from PIL import Image

# ------------------------------------------------------------------------------
class ArgumentParser(argparse.ArgumentParser):
    # -------------------------------------------------------------------------
    def __init__(self, **kw):
        def _positive_int(x):
            try:
                assert(int(x) > 0)
                return int(x)
            except:
                self.error("`%s' is not a positive integer value" % str(x))

        argparse.ArgumentParser.__init__(self, **kw)

        self.add_argument(
            "--rank", "-R",
            metavar='INTEGER',
            dest='tiling_rank',
            nargs='?',
            type=_positive_int,
            default=4
        )

        self.add_argument(
            "--input", "-i",
            metavar='INPUT-FILE',
            dest='input_paths',
            nargs='?',
            type=os.path.realpath,
            action="append",
            default=[]
        )

        self.add_argument(
            "--output", "-o",
            metavar='OUTPUT-FILE',
            dest='output_path',
            nargs='?',
            type=os.path.realpath,
            default=None
        )

    # -------------------------------------------------------------------------
    def processParsedOptions(self, options):
        # TODO
        assert options.tiling_rank == 4

        if options.output_path is None:
            options.output_path = "/tmp/noise.png"
        return options

    # -------------------------------------------------------------------------
    def parseArgs(self):
        # ----------------------------------------------------------------------
        class _Options(object):
            # ------------------------------------------------------------------
            def __init__(self, base):
                self.__dict__.update(base.__dict__)
            # ------------------------------------------------------------------
            def write(self, data):
                if type(data) is str:
                    self.output.buffer.write(data.encode("utf8"))
                else:
                    self.output.buffer.write(data)

        return _Options(self.processParsedOptions(
            argparse.ArgumentParser.parse_args(self)
        ))

# ------------------------------------------------------------------------------
def getArgumentParser():
    return ArgumentParser(
        prog=os.path.basename(__file__),
        description="""
            Converts a NxN tiling image into eagitexi texture image file.
        """
    )
# ------------------------------------------------------------------------------
def sigmoid(x, c):
    def _logt(z):
        return 1.0 / (1.0 + math.exp(-z))
    def _ilogt(z):
        return math.log(z) - math.log(1.0 - z)
    try:
        return _logt(c * _ilogt(x))
    except:
        return x
# ------------------------------------------------------------------------------
class TilingImage(object):
    # -------------------------------------------------------------------------
    def _open(self, input_path):
        if input_path == "-":
            return sys.stdin
        else:
            return open(input_path, "rt")

    # -------------------------------------------------------------------------
    def __init__(self, options, input_path):
        self._options = options
        self._lines = []
        for line in self._open(input_path):
            line = line.strip()
            if line:
                self._lines.append(line)
                assert len(line) == len(self._lines[0])

    # -------------------------------------------------------------------------
    def width(self):
        return len(self._lines[0])

    # -------------------------------------------------------------------------
    def height(self):
        return len(self._lines)

    # -------------------------------------------------------------------------
    def size(self):
        return (self.width(), self.height())

    # -------------------------------------------------------------------------
    def _dec(self, e):
        return float("0123456789ABCDEF".find(e.upper())) / 16.0

    # -------------------------------------------------------------------------
    def _get(self, x, y):
        w = self.width()
        h = self.height()
        return self._lines[(y+h)%h][(x+w)%w]

    # -------------------------------------------------------------------------
    def get(self, x, y, o, xo, yo):

        x = x / o
        y = y / o

        w = self.width()  // o
        h = self.height() // o
        p= [(int(math.floor(x-0.5)), int(math.floor(y-0.5))),
            (int(math.floor(x+0.5)), int(math.floor(y-0.5))),
            (int(math.floor(x-0.5)), int(math.floor(y+0.5))),
            (int(math.floor(x+0.5)), int(math.floor(y+0.5)))]
        m= (x+0.5-int(x+0.5), y+0.5-int(y+0.5))
        c= [(1.0-m[0],1.0-m[1]),(m[0],1.0-m[1]),(1.0-m[0], m[1]),(m[0], m[1])]
        k= [((px+xo+w)%w, (py+yo+h)%h) for px, py in p]
        e= [self._dec(self._get(kx, ky)) for kx, ky in k]
        f= [sigmoid(cx*cy, 1.41) for cx, cy in c]

        return sum(e * f for e, f in zip(e, f)) / sum(f)

# ------------------------------------------------------------------------------
class OutputImage(object):
    # -------------------------------------------------------------------------
    def __init__(self, options, sz):
        self._options = options
        self._sz = sz
        self._im = Image.new(mode="L", size=sz)
        self._data = [0.0] * (sz[0] * sz[1])

    # -------------------------------------------------------------------------
    def width(self):
        return self._sz[0]

    # -------------------------------------------------------------------------
    def height(self):
        return self._sz[1]

    # -------------------------------------------------------------------------
    def _idx(self, x, y):
        return self.width() * y + x

    # -------------------------------------------------------------------------
    def get(self, x, y):
        return self._data[self._idx(x, y)]

    # -------------------------------------------------------------------------
    def set(self, x, y, v):
        self._data[self._idx(x, y)] = v

    # -------------------------------------------------------------------------
    def generate(self, src):
        w = self.width()
        assert src.width() == w
        h = self.height()
        assert src.height() == h

        def _fib(b, s = 1):
            f1, f2 = 1, 2
            while b >= f1:
                yield f1
                for i in range(s):
                    f1, f2 = (f2, f1 + f2)

        def _wgt(o, p):
            return math.pow(o, p)

        def _oct(p):
            for f in _fib(max(w, h), 2):
                yield _wgt(f, p)

        for y in range(h):
            for x in range(w):
                v = 0.0
                u = 0.0
                for octaves in [list(_oct(p)) for p in [0.25, 0.5, 0.66, 0.75]]:
                    f1, f2 = 1, 2
                    for wg in octaves:
                        v += wg * src.get(x, y, int(wg), f1, f2)
                        u += wg
                        f1, f2 = (f2, f1 + f2)
                self.set(x, y, v / max(u, 1.0))

    # -------------------------------------------------------------------------
    def normalize(self):
        mx = 0.0
        mn = 1.0
        for y in range(self.height()):
            for x in range(self.width()):
                mx = max(mx, self.get(x, y))
                mn = min(mn, self.get(x, y))

        def _f(x):
            return sigmoid(x, 0.9)

        if mx > mn:
            for y in range(self.height()):
                for x in range(self.width()):
                    self.set(x, y, _f((self.get(x, y)-mn)/(mx-mn)))

    # -------------------------------------------------------------------------
    def show(self):
        def _conv(x):
            return int(min(max(x, 0.0), 1.0) * 255.0)
        self._im.putdata([_conv(e) for e in self._data])
        self._im.show();
        self._im.save(self._options.output_path)

# ------------------------------------------------------------------------------
def noise(options):
    src = TilingImage(options, options.input_paths[0])
    im = OutputImage(options, src.size())

    im.generate(src)
    im.normalize()
    im.show()

# ------------------------------------------------------------------------------
def main():
    try:
        options = getArgumentParser().parseArgs()
        noise(options)
        return 0
    except Exception as error:
        print(type(error), error)
        try: os.remove(options.output_path)
        except: pass
        raise
        return 1

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
