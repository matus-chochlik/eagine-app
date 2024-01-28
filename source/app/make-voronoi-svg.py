#!/usr/bin/python3
# coding: UTF-8
# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt

import os
import sys
import math
import numpy
import random
import argparse
import multiprocessing

# ------------------------------------------------------------------------------
def mix(a, b, f):
    return (1.0-f)*a + f*b
# ------------------------------------------------------------------------------
def inverse_logistic(x):
    eps = 0.001
    return math.log(max(x, eps)) - math.log(max(1.0 - x, eps ))
# ------------------------------------------------------------------------------
def logistic(x):
    return 1.0 / (1.0 + math.exp(-x))
# ------------------------------------------------------------------------------
def sigmoid(x, c):
    return logistic(c * inverse_logistic(x))
# ------------------------------------------------------------------------------
def perpendicular(v1):
    v2 = numpy.empty_like(v1)
    v2[0] = -v1[1]
    v2[1] =  v1[0]
    return v2

# ------------------------------------------------------------------------------
def set_center(points):
    return sum(points)/len(points)

# ------------------------------------------------------------------------------
def segment_point(p1, p2, c):
    return (1-c)*p1 + c*p2;

# ------------------------------------------------------------------------------
def segment_midpoint(p1, p2):
    return (p1+p2)*0.5

# ------------------------------------------------------------------------------
def segment_normal(p1, p2):
    return perpendicular(p2-p1)

# ------------------------------------------------------------------------------
def line_intersect_param(l1, l2):
    d1 = l1[1]
    d2 = l2[1]
    dp = l2[0]-l1[0]
    d2p = perpendicular(d2)

    num = numpy.dot(d2p, dp)
    den = numpy.dot(d2p, d1)

    if abs(den) > 0.00001:
        return num / den
    return None

# ------------------------------------------------------------------------------
def is_rim(x, y, w, h, rw):
    return x < rw or y < rw or x+rw >= w or y+rw >= h

# ------------------------------------------------------------------------------
class ImageSampler(object):
    # --------------------------------------------------------------------------
    def __init__(self, image, width, height):
        self._im = image
        self._w = width
        self._h = height

    # --------------------------------------------------------------------------
    @classmethod
    def from_file(cls, path, width, height):
        import PIL.Image
        image = PIL.Image.open(path).convert("RGB")

        if width is None:
            width, unused = image.size
        if height is None:
            unused, height = image.size

        if (width, height) != image.size:
            image = image.resize((width, height), PIL.Image.BICUBIC)
        return cls(image, width, height)

    # --------------------------------------------------------------------------
    def width(self):
        return self._w

    # --------------------------------------------------------------------------
    def height(self):
        return self._h

    # --------------------------------------------------------------------------
    def get_pixel(self, x, y):
        x = max(min(x, self._w-1), 0)
        y = max(min(y, self._h-1), 0)
        c0, c1, c2 = self._im.getpixel((x, y))

        return (c0/255.0, c1/255.0, c2/255.0)

    # --------------------------------------------------------------------------
    def converted(self, mode):
        return ImageSampler(self._im.convert(mode), self._w, self._h)

# ------------------------------------------------------------------------------
class NoImageSampler(object):
    # --------------------------------------------------------------------------
    def __init__(self):
        pass

    # --------------------------------------------------------------------------
    def get_pixel(self, x, y):
        return (0.0, 0.0, 0.0)

    # --------------------------------------------------------------------------
    def converted(self, mode):
        return self

# ------------------------------------------------------------------------------
class RandomGenerator(object):

    # --------------------------------------------------------------------------
    def __init__(self, mrg, rrg):
        self._mrg = mrg
        self._rrg = rrg

    # --------------------------------------------------------------------------
    def get(self, rim):
        if rim:
            try:
                return self._rrg.random()
            except:
                pass
        return self._mrg.random()

# ------------------------------------------------------------------------------
class Randomized(object):
    # --------------------------------------------------------------------------
    def _get_rng0(self):
        try:
            return self.rng0
        except:
            self.rng0 = random.Random(self._mid_seed)
            return self.rng0

    # --------------------------------------------------------------------------
    def _mid_rng(self):
        import random

        if self._mid_seed is None:
            import time
            try: return random.SystemRandom()
            except: return random.Random(time.time())
        else:
            return random.Random(self._get_rng0().randrange(0, sys.maxsize))

    # --------------------------------------------------------------------------
    def _rim_rng(self):
        if self._rim_seed is not None:
            return random.Random(self._rim_seed)
        return None

    # --------------------------------------------------------------------------
    def get_rng(self):
        return RandomGenerator(self._mid_rng(), self._rim_rng())

    # --------------------------------------------------------------------------
    def __init__(self, options):
        self._mid_seed = options.seed
        self._rim_seed = options.rim_seed

# ------------------------------------------------------------------------------
class RandomCellValues(Randomized):
    # --------------------------------------------------------------------------
    def _gen_values(self, w, h, transformable, rw):

        rc = self.get_rng()

        cell_data = list()
        for y in range(h):
            r = list()
            for x in range(w):
                r.append(rc.get(is_rim(x, y, w, h, rw)))
            cell_data.append(r)

        if transformable:
            r = range(int(w/2)+1)
            rv = [rc.get(True) for i in r]
            for i in r:
                v = 0.5 + (rv[i]-0.5)*0.75

                cell_data[i][0] = v
                cell_data[h-i-1][0] = v
                cell_data[i][w-1] = v
                cell_data[h-i-1][w-1] = v
                cell_data[0][i] = v
                cell_data[0][w-i-1] = v
                cell_data[h-1][i] = v
                cell_data[h-1][w-i-1] = v
        return cell_data

    # --------------------------------------------------------------------------
    def __init__(self, options, w, h):
        Randomized.__init__(self, options)
        self._values = self._gen_values(
            w, h, options.transformable, options.rim_width)

    # --------------------------------------------------------------------------
    def get(self, x, y):
        return self._values[y][x]

# ------------------------------------------------------------------------------
class RandomCellOffsets(Randomized):
    # --------------------------------------------------------------------------
    def _gen_offsets(self, w, h, transformable, rw):

        rx = self.get_rng()
        ry = self.get_rng()

        cell_data = list()
        for y in range(h):
            row = list()
            for x in range(w):
                rim = is_rim(x, y, w, h, rw)
                row.append((rx.get(rim), ry.get(rim)))
            cell_data.append(row)

        if transformable:
            r = range(int(w/2)+1)
            rv = [(rx.get(True), ry.get(True)) for i in r]
            for i in r:
                xo, yo = rv[i]
                l = 0.8
                cell_data[i][0] = (l*xo, yo)
                cell_data[h-i-1][0] = (l*xo, 1.0-yo)
                cell_data[i][w-1] = (1.0-l*xo, 1.0-yo)
                cell_data[h-i-1][w-1] = (1.0-l*xo, yo)
                cell_data[0][i] = (xo, l*yo)
                cell_data[0][w-i-1] = (1.0-xo, l*yo)
                cell_data[h-1][i] = (1.0-xo, 1.0-l*yo)
                cell_data[h-1][w-i-1] = (xo, 1.0-l*yo)
        return cell_data
    # --------------------------------------------------------------------------
    def __init__(self, options, w, h):
        Randomized.__init__(self, options)
        self._offsets = self._gen_offsets(
            w, h, options.transformable, options.rim_width)

    # --------------------------------------------------------------------------
    def get(self, x, y):
        return self._offsets[y][x]

# ------------------------------------------------------------------------------
class ImageContourCellOffsets(object):
    # --------------------------------------------------------------------------
    def _gen_offsets(self, im, bg, w, h):

        def _distmod(x, y):
            d = abs(x - y)
            return d if d < 0.5 else 1.0-d

        kernel = [
            (-1, -1),
            ( 0, -1),
            ( 1, -1),
            (-1,  0),
            ( 1,  0),
            (-1,  1),
            ( 0,  1),
            ( 1,  1)
        ]
        kn = 1.0/(len(kernel))


        cell_data = list()
        for y in range(h):
            row = list()
            for x in range(w):
                nx = 0.0
                ny = 0.0

                dispx = 0.0
                dispy = 0.0

                h, s, v = im.get_pixel(x, y)
                for ox, oy in kernel:
                    oh, os, ov = im.get_pixel(x+ox, y+oy)
                    dh = _distmod(h, oh)
                    ds = s - os
                    dv = v - ov
                    adh = abs(dh)
                    ads = abs(ds)
                    adv = abs(dv)
                    dw = dv if adv > ads else ds if ads > adh else dh
                    vx, vy = ox, oy
                    vl = math.sqrt(vx*vx + vy*vy)
                    vx /= vl
                    vy /= vl
                    nx += vx*dw
                    ny += vy*dw
                    dispx += nx*nx
                    dispy += ny*ny

                nx = nx*kn
                ny = ny*kn
                dispx = math.sqrt(dispx)*kn
                dispy = math.sqrt(dispy)*kn
                dispw = sigmoid(
                    math.sqrt(
                        max(abs(nx), abs(ny), abs(dispx-dispy))
                    ),
                    2.5
                )
                nx = 0.5 + 0.5*nx
                ny = 0.5 + 0.5*ny
                bx, by = bg.get(x, y)
                row.append((mix(bx, nx, dispw), mix(by, ny, dispw)))

            cell_data.append(row)
        return cell_data
    # --------------------------------------------------------------------------
    def __init__(self, options, bg, w, h):
        self._offsets = self._gen_offsets(
            options.image.converted("HSV"),
            bg,
            w, h)

    # --------------------------------------------------------------------------
    def get(self, x, y):
        return self._offsets[y][x]

# ------------------------------------------------------------------------------
class HoneycombXCellOffsets(object):
    # --------------------------------------------------------------------------
    def __init__(self, options, bg, w, h):
        self._fact_x = 0.8
        self._fact_y = 0.9
        self._bg = bg
    # --------------------------------------------------------------------------
    def get(self, x, y):
        hx, hy = (0.5, 0.0 if x % 2 == 0 else 0.5)
        bx, by = self._bg.get(x, y)
        return (mix(bx, hx, self._fact_x), mix(by, hy, self._fact_y))
# ------------------------------------------------------------------------------
class HoneycombYCellOffsets(object):
    # --------------------------------------------------------------------------
    def __init__(self, options, bg, w, h):
        self._fact_x = 0.9
        self._fact_y = 0.8
        self._bg = bg
    # --------------------------------------------------------------------------
    def get(self, x, y):
        hx, hy = (0.0 if y % 2 == 0 else 0.5, 0.5)
        bx, by = self._bg.get(x, y)
        return (mix(bx, hx, self._fact_x), mix(by, hy, self._fact_y))
# ------------------------------------------------------------------------------
class VoronoiArgumentParser(argparse.ArgumentParser):
    # --------------------------------------------------------------------------
    def _positive_int(self, x):
        try:
            i = int(x)
            assert i > 0
            return i
        except:
            self.error("`%s' is not a positive integer value" % str(x))
    # --------------------------------------------------------------------------
    def __init__(self, **kw):
        argparse.ArgumentParser.__init__(self, **kw)

        self.add_argument(
            "--print-bash-completion",
            metavar='FILE|-',
            dest='print_bash_completion',
            default=None)

        self.add_argument(
            'output',
            nargs='?',
            type=argparse.FileType('w'),
            default=sys.stdout)

        self.add_argument(
            '--log', '-l',
            type=argparse.FileType('w'),
            default=sys.stderr)

        self.add_argument(
            '--jobs', '-j',
            dest="job_count",
            type=self._positive_int,
            action="store",
            default=1)

        self.add_argument(
            '--x-cells', '-X',
            type=self._positive_int,
            action="store",
            default=None)

        self.add_argument(
            '--y-cells', '-Y',
            type=self._positive_int,
            action="store",
            default=None)

        self.add_argument(
            '--width', '-W',
            type=self._positive_int,
            action="store",
            default=512)

        self.add_argument(
            '--height', '-H',
            type=self._positive_int,
            action="store",
            default=512)

        self.add_argument(
            '--units', '-U',
            action="store",
            choices=["px", "mm", "cm", "in", "pt"],
            default="px")

        self.add_argument(
            '--stroke-width', '-s',
            type=float,
            action="store",
            default=0.5)

        self.add_argument(
            '--value-low', '-vl',
            type=float,
            action="store",
            default=0.05)

        self.add_argument(
            '--value-high', '-vh',
            type=float,
            action="store",
            default=0.95)

        self.add_argument(
            '--cell-z-coord', '-cz',
            type=float,
            action="store",
            default=0.0)

        self.add_argument(
            '--scale', '-S',
            type=float,
            action="store",
            default=0.9)

        self.add_argument(
            '--scale-mode', '-Q',
            type=str,
            choices=["constant", "linear", "sqrt", "pow2", "exp", "sigmoid"],
            action="store",
            default="constant")

        self.add_argument(
            '--seed', '-rs',
            type=float,
            action="store",
            default=None)

        self.add_argument(
            '--rim-seed', '-Rs',
            type=float,
            action="store",
            default=None)

        self.add_argument(
            '--rim-width', '-Rw',
            type=self._positive_int,
            action="store",
            default=2)

        rimmug = self.add_mutually_exclusive_group()

        rimmug.add_argument(
            '--only-rim',
            action="store_true",
            default=False)

        rimmug.add_argument(
            '--skip-rim',
            action="store_true",
            default=False)

        self.add_argument(
            '--transformable', '-T',
            action="store_true",
            default=False)

        self.add_argument(
            '--color-mode', '-M',
            type=str,
            choices=["grayscale", "cell-coord", "cell-coord-value", "image-rgb"],
            action="store",
            default="grayscale")

        self.add_argument(
            '--cell-mode', '-C',
            type=str,
            choices=[
                "full",
                "scaled",
                "flagstone",
                "pebble",
                "worley-nmap",
                "worley-hmap",
                "worley-cvh"
            ],
            action="store",
            default="full")

        self.add_argument(
            '--offs-mode', '-O',
            type=str,
            choices=["default", "honeycomb-x", "honeycomb-y"],
            action="store",
            default="default")

        self.add_argument(
            '--image', '-i',
            dest="image_path",
            type=os.path.realpath,
            action="store",
            default=None)

        self.add_argument(
            '--verbose', '-v',
            action="store_true",
            default=False)
    # --------------------------------------------------------------------------
    def process_parsed_options(self, options):
        if options.transformable:
            if options.width != options.height:
                self.error("width and height must be the same in transformable mode")
            if options.x_cells != options.y_cells:
                self.error("X-cells and Y-cells must be the same in transformable mode")

        if options.image_path is not None:
            options.image = ImageSampler.from_file(
                options.image_path,
                options.x_cells,
                options.y_cells
            )

            if options.x_cells is None:
                options.x_cells = options.image.width()
            if options.y_cells is None:
                options.y_cells = options.image.height()
        else:
            options.image = NoImageSampler()
            if options.x_cells is None:
                options.x_cells = 32
            if options.y_cells is None:
                options.y_cells = 32

        if options.cell_mode in ["worley-nmap", "worley-hmap", "worley-cvh"]:
            options.need_neighbors = True
            options.job_count = 1
        else:
            options.need_neighbors = False

        return options
    # --------------------------------------------------------------------------
    def parse_args(self):
        return self.process_parsed_options(
            argparse.ArgumentParser.parse_args(self)
        )

# ------------------------------------------------------------------------------
def getArgumentParser():
    return VoronoiArgumentParser(
            prog="voronoi-svg",
            description="""
            Utility annotating lines read from standard input
        """
    )

# ------------------------------------------------------------------------------
class Renderer(object):
    # --------------------------------------------------------------------------
    def grayscale_color_str(self, v):
        c = "%02x" % int(255*v)
        return "#"+3*c

    # --------------------------------------------------------------------------
    def cell_offset(self, x, y):
        cy = (y+self.y_cells)%self.y_cells
        cx = (x+self.x_cells)%self.x_cells
        return self.cell_offsets.get(cx, cy)

    # --------------------------------------------------------------------------
    def cell_value(self, x, y):
        cy = (y+self.y_cells)%self.y_cells
        cx = (x+self.x_cells)%self.x_cells
        return self.cell_values.get(cx, cy)

    # --------------------------------------------------------------------------
    def cell_grayscale_color(self, x, y):
        cv = self.cell_value(x, y)
        v = self.value_low + cv*(self.value_high-self.value_low)
        return self.grayscale_color_str(v)

    # --------------------------------------------------------------------------
    def cell_coord_color(self, x, y):
        x = (x + self.x_cells) % self.x_cells
        y = (y + self.y_cells) % self.y_cells

        r = int((256*x)/self.x_cells)
        g = int((256*y)/self.y_cells)
        b = int((256*self.cell_z_coord))

        return "#%02x%02x%02x" % (r, g, b)

    # --------------------------------------------------------------------------
    def cell_coord_color_value(self, x, y):
        cv = self.cell_value(x, y)
        v = self.value_low + cv*(self.value_high-self.value_low)
        x = (x + self.x_cells) % self.x_cells
        y = (y + self.y_cells) % self.y_cells

        r = int((256*x)/self.x_cells)
        g = int((256*y)/self.y_cells)
        b = int(255*v)

        return "#%02x%02x%02x" % (r, g, b)

    # --------------------------------------------------------------------------
    def cell_image_color(self, x, y):
        r, g, b = self.image.get_pixel(x, y)

        return "#%02x%02x%02x" % (int(r*255), int(g*255), int(b*255))

    # --------------------------------------------------------------------------
    def cell_gradient_id(self, x, y, i, j):
        s = "grad%d_%d" % (
            (y+3) * (self.x_cells + 6) + (x+3),
            (y+j+3) * (self.x_cells + 6) + (x+i+3)
        )
        return s

    # --------------------------------------------------------------------------
    def cell_scale(self, x, y):
        coef = 1.0
        if self.scale_mode == "linear":
            coef = self.cell_value(x, y)
        elif self.scale_mode == "sqrt":
            coef = math.sqrt(self.cell_value(x, y))
        elif self.scale_mode == "pow2":
            coef = math.pow(self.cell_value(x, y), 2)
        elif self.scale_mode == "exp":
            coef = math.exp(self.cell_value(x, y)) / math.exp(1)
        elif self.scale_mode == "sigmoid":
            coef = 0.5 - 0.5*math.cos(self.cell_value(x, y)*math.pi)
        return self.scale * coef

    # --------------------------------------------------------------------------
    def add_viewbox_intersections(self, corners):
        assert len(corners) >= 3
        def _find_intersections(a, b):
            w = self.width
            h = self.height
            wb = [((0,0),(w,0)), ((w,0),(0,h)), ((w,h),(-w,0)), ((0,h),(0,-h))]
            v = (b - a)
            it = []
            for ed in wb:
                t1 = line_intersect_param((a, v), ed)
                if t1 is not None:
                    if t1 > 0.0 and t1 < 1.0:
                        t2 = line_intersect_param(ed, (a, v))
                        if t2 > 0.0 and t2 < 1.0:
                            it.append((t1, (mix(a[0], b[0], t1), mix(a[1], b[1], t1))))
            for t, i in sorted(it, key=lambda x: x[0]):
                yield numpy.array(i)

        a = corners[0]
        for b in corners[1:]:
            yield a
            for i in _find_intersections(a, b):
                yield i
            a = b
        yield b
        for i in _find_intersections(b, corners[0]):
            yield i

    # --------------------------------------------------------------------------
    def full_cell_element_str(self, x, y, unused, corners, offs):
        corners = self.add_viewbox_intersections(corners)
        clist = ["%.3f %.3f" % (c[0], c[1]) for c in corners]
        pathstr = "M"+" L".join(clist)+" Z"
        yield """
        <path d="%(def)s" stroke="%(color)s" fill="%(color)s"/>\n""" % {
            "def": pathstr,
            "color": self.cell_color(x, y)
        }

    # --------------------------------------------------------------------------
    def scaled_cell_element_str(self, x, y, center, corners, offs):
        m = set_center(corners)
        newcorners = [segment_point(m, c, self.cell_scale(x, y)) for c in corners]
        return self.full_cell_element_str(x, y, center, newcorners, offs);

    # --------------------------------------------------------------------------
    def flagstone_cell_element_str(self, x, y, center, corners, offs):
        zcorners = zip(corners, corners[1:] + [corners[0]])
        c = self.cell_value(x, y)
        newcorners = [segment_point(a, b, c) for (a, b) in zcorners]
        return self.scaled_cell_element_str(x, y, center, newcorners, offs);

    # --------------------------------------------------------------------------
    def pebble_cell_element_str(self, x, y, center, corners, offs):
        m = set_center(corners)
        apoints = [segment_point(m, c, self.cell_scale(x, y)) for c in corners]
        bpoints = apoints[1:] + [apoints[0]]
        c = self.cell_value(x, y)
        zpoints = zip(apoints, bpoints)
        cpoints = [segment_point(a, b, c) for (a, b) in zpoints]
        dpoints = cpoints[1:] + [cpoints[0]]

        zpoints = zip(bpoints, dpoints)

        cfmt = lambda c : "%.3f %.3f" % (c[0], c[1])

        clist = ["%s, %s" % (cfmt(b), cfmt(d)) for (b, d) in zpoints]
        pathstr = "M%s Q" % cfmt(cpoints[0])+" Q".join(clist)+" Z"
        yield """<path d="%(def)s" stroke="%(color)s" fill="%(color)s"/>\n""" % {
            "def": pathstr,
            "color": self.cell_color(x, y)
        }

    # --------------------------------------------------------------------------
    def worley_cell_element_str(self, x, y, center, corners, offs):
        n = len(corners)
        for t in range(n):
            i, j = offs[t]
            verts = (center, corners[t], corners[(t+1)%n])
            clist = ["%.3f %.3f" % (v[0], v[1]) for v in verts]
            pathstr = "M"+" L".join(clist)+" Z"
            yield """<path d="%(def)s" stroke="url(#%(gref)s)" fill="url(#%(gref)s)"/>\n""" % {
                "def": pathstr,
                "gref": self.cell_gradient_id(x, y, i, j)
            }

    # --------------------------------------------------------------------------
    def __init__(self, useropts):

        for k, v in useropts.__dict__.items():
            self.__dict__[k] = v

        if self.color_mode == "grayscale":
            self.cell_color = lambda x, y: self.cell_grayscale_color(x, y)
        elif self.color_mode == "cell-coord":
            self.cell_color = lambda x, y: self.cell_coord_color(x, y)
        elif self.color_mode == "cell-coord-value":
            self.cell_color = lambda x, y: self.cell_coord_color_value(x, y)
        elif self.color_mode == "image-rgb":
            self.cell_color = lambda x, y: self.cell_image_color(x, y)

        if self.cell_mode == "full":
            self.cell_element_str = self.full_cell_element_str
        elif self.cell_mode == "scaled":
            self.cell_element_str = self.scaled_cell_element_str
        elif self.cell_mode == "flagstone":
            self.cell_element_str = self.flagstone_cell_element_str
        elif self.cell_mode == "pebble":
            self.cell_element_str = self.pebble_cell_element_str
        elif self.cell_mode in ["worley-nmap", "worley-hmap", "worley-cvh"]:
            self.cell_element_str = self.worley_cell_element_str

        self.cell_values = RandomCellValues(
            self,
            self.x_cells,
            self.y_cells
        )

        rco = RandomCellOffsets(
            self,
            self.x_cells,
            self.y_cells
        )

        if self.offs_mode == "honeycomb-x":
            self.cell_offsets = HoneycombXCellOffsets(
                self,
                rco,
                self.x_cells,
                self.y_cells
            )
        elif self.offs_mode == "honeycomb-y":
            self.cell_offsets = HoneycombYCellOffsets(
                self,
                rco,
                self.x_cells,
                self.y_cells
            )
        else:
            self.cell_offsets = ImageContourCellOffsets(
                self,
                rco,
                self.x_cells,
                self.y_cells
            )

        self.values = dict()
        self.values["width"] = self.width
        self.values["height"] = self.height
        self.values["wunit"] = self.units
        self.values["hunit"] = self.units

        self.cell_fmt = "%%%dd %%%dd\n" % (
            int(math.log10(self.x_cells)+1),
            int(math.log10(self.y_cells)+1)
        )

# ------------------------------------------------------------------------------
def cell_world_coord(renderer, x, y):

    c = renderer.cell_offset(x, y)
    return numpy.array((
        (x+c[0])*(renderer.width/renderer.x_cells),
        (y+c[1])*(renderer.height/renderer.y_cells)
    ))

# ------------------------------------------------------------------------------
def cell_value(renderer, x, y):
    return renderer.get_value(x, y)

# ------------------------------------------------------------------------------
def cell_color(renderer, x, y):
    return grayscalestr(
        renderer.value_low+
        cell_value(renderer, x, y)*
        (renderer.value_high-renderer.value_low)
    )

# ------------------------------------------------------------------------------
def offs_cell_world_coord(renderer, x, y, o):
    return cell_world_coord(renderer, x+o[0], y+o[1])

# ------------------------------------------------------------------------------
def make_cell(renderer, x, y):

    owc = cell_world_coord(renderer, x, y)

    offsets = []

    for j in range(-2, 3):
        for i in range(-2, 3):
            if j != 0 or i != 0:
                offsets.append((i, j))

    loffs = len(offsets)
    cuts = []

    for o in offsets:
        cwc = offs_cell_world_coord(renderer, x, y, o)

        sm = segment_midpoint(owc, cwc)
        sn = segment_normal(owc, cwc)
        cuts.append((sm, sn))

    assert loffs == len(cuts)

    intersections = []

    for cj in range(loffs):
        for ci in range(cj+1, loffs):
            t = line_intersect_param(cuts[cj], cuts[ci])
            if t is not None:
                intersections.append((cuts[cj][0]+cuts[cj][1]*t, set([ci, cj])))

    corners_and_cuts = []

    for isc, cus  in intersections:
        seg = (owc, isc-owc)
        eps = 0.001
        skip = False

        for cut in cuts:
            t = line_intersect_param(seg, cut)
            if t is not None and t >= 0 and t < 1-eps:
                skip = True
                break

        if not skip:
            corners_and_cuts.append((isc, cus))

    def corner_angle(p):
        v = p[0] - owc
        return math.atan2(v[1], v[0])

    sorted_corners_and_cuts = sorted(corners_and_cuts, key=corner_angle)

    corners = []
    neighbors = []

    caclen = len(sorted_corners_and_cuts)
    for c in range(caclen):
        co0, cu0 = sorted_corners_and_cuts[c]
        co1, cu1 = sorted_corners_and_cuts[(c+1)%caclen]
        cu = cu0.intersection(cu1)

        corners.append(co0)
        if renderer.need_neighbors:
            assert len(cu) == 1
            neighbors.append(offsets[cu.pop()])

    if renderer.need_neighbors:
        assert len(corners) == len(neighbors)

    return owc, corners, neighbors
    
# ------------------------------------------------------------------------------
def do_make_cell(renderer, options, job, output_lock):
    w = renderer.x_cells + 2
    h = renderer.y_cells + 2
    k = job
    n = w * h

    res = []
    log = []

    def _flush(res, log):
        r = str().join(res)
        if renderer.verbose:
            l = str().join(log)
        try:
            output_lock.acquire()
            renderer.output.write(r)
            if renderer.verbose:
                renderer.log.write(l)
        finally:
            output_lock.release()
        return ([], [])

    try:
        while k < n:
            y = int(k / w) - 1
            x = int(k % w) - 1

            skip = False
            if is_rim(x, y, renderer.x_cells, renderer.y_cells, options.rim_width-1):
                if options.skip_rim:
                    skip = True
            else:
                if options.only_rim:
                    skip = True

            if not skip:
                center, corners, offs = make_cell(renderer, x, y)
                for svg_str in renderer.cell_element_str(x, y, center, corners, offs):
                    res.append(svg_str)
            if renderer.verbose:
                log.append(renderer.cell_fmt % (x, y))
            else:
                log.append(None)

            if len(res) >= renderer.job_count:
                res, log = _flush(res, log)

            k += renderer.job_count
    except KeyboardInterrupt:
        pass

    _flush(res, log)

# ------------------------------------------------------------------------------
def make_gradients(renderer):
    w = renderer.x_cells
    h = renderer.y_cells

    grad_fmt = """<linearGradient gradientUnits="userSpaceOnUse" id="%(gref)s" """+\
                """x1="%(x1)f" y1="%(y1)f" x2="%(x2)f" y2="%(y2)f">\n"""
    stop_fmt = """<stop offset="%(soffs)d%%" stop-color="%(color)s" stop-opacity="%(opacity)s"/>\n"""

    offsets = []
    for j in range(-2, 3):
        for i in range(-2, 3):
            if j != 0 or i != 0:
                offsets.append((i, j))

    cell_size = math.sqrt(pow(renderer.width / w, 2)+pow(renderer.height / h, 2))

    for y in range(-1, h+2):
        for x in range(-1, w+2):
            for i, j in offsets:
                cwc = cell_world_coord(renderer, x, y)
                owc = cell_world_coord(renderer, x+i, y+j)

                renderer.output.write(grad_fmt % {
                        "gref": renderer.cell_gradient_id(x, y, i, j),
                        "x1": cwc[0],
                        "y1": cwc[1],
                        "x2": owc[0],
                        "y2": owc[1] 
                })
                if renderer.cell_mode == "worley-nmap":
                    val = renderer.cell_value(x, y)
                    vec = cwc - owc
                    vec = (
                        math.sqrt(val)*vec[0],
                        math.sqrt(val)*vec[1],
                        -cell_size*math.sqrt(1.0-val*0.5)
                    )
                    vln = math.sqrt(sum(pow(v, 2) for v in vec))
                    vec = [v / vln for v in vec]
                    vec = [0.5 - 0.5*v for v in vec]
                    renderer.output.write(stop_fmt % {
                        "soffs": 0.0,
                        "opacity": "1.0",
                        "color": "#%(cx)02x%(cy)02x%(cz)02x" % {
                            "cx": int(255*vec[0]),
                            "cy": int(255*vec[1]),
                            "cz": int(255*vec[2])
                        }
                    })
                    renderer.output.write(stop_fmt % {
                        "soffs": 50.0,
                        "opacity": "1.0",
                        "color": "#%(cx)02x%(cy)02x%(cz)02x" % {
                            "cx": int(255*vec[0]),
                            "cy": int(255*vec[1]),
                            "cz": int(255*vec[2])
                        }
                    })
                if renderer.cell_mode == "worley-hmap":
                    val = renderer.cell_value(x, y)
                    renderer.output.write(stop_fmt % {
                        "soffs": 0.0,
                        "opacity": "1.0",
                        "color": "#%(cx)02x%(cy)02x%(cz)02x" % {
                            "cx": int(255*val),
                            "cy": int(255*val),
                            "cz": int(255*val)
                        }
                    })
                    renderer.output.write(stop_fmt % {
                        "soffs": 50.0,
                        "opacity": "1.0",
                        "color": "black"
                    })
                if renderer.cell_mode == "worley-cvh":
                    renderer.output.write(stop_fmt % {
                        "soffs": 0.0,
                        "opacity": "1.0",
                        "color": "#%(cx)02x%(cy)02x%(cv)02x" % {
                            "cx": int(255*float((x+w) % w)/w),
                            "cy": int(255*float((y+h) % h)/h),
                            "cv": int(255*renderer.cell_value(x, y))
                        }
                    })
                    renderer.output.write(stop_fmt % {
                        "soffs": 50.0,
                        "opacity": "0.0",
                        "color": "#%(cx)02x%(cy)02x%(cv)02x" % {
                            "cx": int(255*float((x+w) % w)/w),
                            "cy": int(255*float((y+h) % h)/h),
                            "cv": int(255*renderer.cell_value(x, y))
                        }
                    })
                renderer.output.write("""</linearGradient>\n""")
# ------------------------------------------------------------------------------
def print_svg(renderer, options):
    renderer.output.write("""<?xml version="1.0" encoding="utf8"?>\n""")
    renderer.output.write("""<svg xmlns="http://www.w3.org/2000/svg"
    xmlns:svg="http://www.w3.org/2000/svg"
    width="%(width)s%(wunit)s" height="%(height)s%(hunit)s"
    viewBox="0 0 %(width)s %(height)s"
    version="1.1"
    contentScriptType="text/ecmascript"
    contentStyleType="text/css"\n>\n""" % renderer.values)
    renderer.output.write(
        """<g class="voronoi" stroke-width="%(stroke_width)f">\n""" % {
            "stroke_width": renderer.stroke_width
        }
    )

    renderer.output.write("<defs>\n")
    if renderer.cell_mode in ["worley-nmap", "worley-hmap", "worley-cvh"]:
        make_gradients(renderer)
    renderer.output.write("</defs>\n")
    renderer.output.flush()

    try:
        output_lock = multiprocessing.Lock()

        def call_do_make_cell(renderer, job, output_lock):
            try:
                do_make_cell(renderer, options, job, output_lock)
            except AssertionError:
                sys.stderr.write("failed to generate SVG, please retry\n")
                raise SystemExit(1)
        if renderer.job_count > 1:
            tasks = []
            for job in range(renderer.job_count):
                t = multiprocessing.Process(
                    target=call_do_make_cell,
                    args=(renderer, job, output_lock)
                )
                t.start()
                tasks.append(t)

            for t in tasks:
                t.join()
                if t.exitcode is not None and t.exitcode != 0:
                    return 1
        else:
            call_do_make_cell(renderer, 0, output_lock)
    except KeyboardInterrupt:
        pass

    renderer.output.write("""\n""")

    renderer.output.write("""</g>\n""")
    renderer.output.write("""</svg>\n""")
    return 0

# ------------------------------------------------------------------------------
#  bash completion
# ------------------------------------------------------------------------------
def printBashCompletion(argparser, options):
    from eagine.argparseUtil import printBashComplete
    def _printIt(fd):
        printBashComplete(
            argparser,
            "_eagine_make_voronoi_svg",
            "eagine-make-voronoi-svg",
            ["--print-bash-completion"],
            fd)
    if options.print_bash_completion == "-":
        _printIt(sys.stdout)
    else:
        with open(options.print_bash_completion, "wt") as fd:
            _printIt(fd)

# ------------------------------------------------------------------------------
#  Main
# ------------------------------------------------------------------------------
def main():
    argparser = getArgumentParser()
    options = argparser.parse_args()
    if options.print_bash_completion:
        printBashCompletion(argparser, options)
        return 0
    else:
        renderer = Renderer(options)
    sys.exit(print_svg(renderer, options))
    
# ------------------------------------------------------------------------------
if __name__ == "__main__": main()

