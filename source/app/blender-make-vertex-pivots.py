#!/usr/bin/env -S blender --python /usr/bin/eagine-blender-make-vertex-pivots
# coding=utf-8
# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
# usage: blender -b myfile.blend -P make_vertex_pivots.py [-- [--help|options...]]

import os
import sys
import math
import numpy
import argparse
# ------------------------------------------------------------------------------
class PivotMeshArgParser(argparse.ArgumentParser):
    # --------------------------------------------------------------------------
    def _positive_int(self, x):
        try:
            i = int(x)
            assert(i > 0)
            return i
        except:
            self.error("`%s' is not a positive integer value" % str(x))
    # --------------------------------------------------------------------------
    def __init__(self, **kw):
        argparse.ArgumentParser.__init__(self, **kw)

        self.add_argument(
            '--debug', '-d',
            action="store_true",
            default=False)

        self.add_argument(
            '--do-not-save', '-D',
            dest="do_not_save",
            action="store_true",
            default=False)

        self.add_argument(
            '--mesh', '-m',
            required=True,
            dest="position_mesh",
            default=None)

        self.add_argument(
            '--epsilon', '-e',
            dest="epsilon",
            type=float,
            default=0.001)

        self.add_argument(
            '--pivot-mesh', '-p',
            dest="pivot_mesh",
            default=None)
    # --------------------------------------------------------------------------
    def process_parsed_options(self, options):
        def _strip(s, w):
            return s[:-len(w)] if s[-len(w):] == w else s

        if options.pivot_mesh is None:
            options.pivot_mesh =\
                _strip(options.position_mesh, "_position") + "_vertex_pivot"
        return options

    # --------------------------------------------------------------------------
    def parse_args(self, args):
        return self.process_parsed_options(
            argparse.ArgumentParser.parse_args(self, args))

# ------------------------------------------------------------------------------
def make_argument_parser():
    return PivotMeshArgParser(
            prog=os.path.basename(__file__),
            description="""
            Blender vertex pivot mesh generator
        """)
# ------------------------------------------------------------------------------
def find_nearest_point_t(options, pa, va, pb, vb):
    vc = numpy.cross(vb, va)
    lc = numpy.linalg.norm(vc)
    if math.fabs(lc) < options.epsilon:
        return 0.0

    try:
        return numpy.linalg.solve(numpy.array([va,-vb, vc]).T, pb - pa)[0]
    except numpy.linalg.LinAlgError:
        return 0.0

# ------------------------------------------------------------------------------
def do_generate(options, src, dst):
    vert_offs_ts = {}
    dst_polys = []
    for poly in src.polygons:
        n = poly.loop_total
        poly_vert_indices = []
        for i in range(n):
            li = poly.loop_start + i
            ii = src.loops[li].vertex_index
            vi = src.vertices[ii]
            pi = numpy.array(vi.co)
            ni = -numpy.array(vi.normal)
            ni /= numpy.linalg.norm(ni)
            ts = []
            for j in range(n):
                if i != j:
                    dist = min(math.fabs(i-j), math.fabs(i+n-j), math.fabs(i-n-j))
                    lj = poly.loop_start + j
                    ij = src.loops[lj].vertex_index
                    vj = src.vertices[ij]
                    pj = numpy.array(vj.co)
                    nj = -numpy.array(vj.normal)
                    nj /= numpy.linalg.norm(nj)
                    t = find_nearest_point_t(options, pi, ni, pj, nj)
                    ts.append((t, 1.0 / dist))

            if ii in vert_offs_ts:
                vert_offs_ts[ii][2] += ts
            else:
                vert_offs_ts[ii] = [pi, ni, ts]

            poly_vert_indices.append(ii)
        dst_polys.append(poly_vert_indices)

    new_verts = []
    for ii in range(len(src.vertices)):
        pi, ni, ts = vert_offs_ts[ii]
        ti = sum(t * w for t, w in ts) / sum(w for t, w in ts)
        new_verts.append(dst.verts.new(pi + ni * ti))

    for poly_verts in dst_polys:
        dst.faces.new(new_verts[ii] for ii in poly_verts)

# ------------------------------------------------------------------------------
def generate(options):
    import bpy
    import bmesh
    pomesh = bpy.data.objects[options.position_mesh].data
    pvmesh = bmesh.new()

    do_generate(options, pomesh, pvmesh)

    msh = bpy.data.meshes.new(options.pivot_mesh)
    pvmesh.to_mesh(msh)
    pvmesh.free()
    obj = bpy.data.objects.new(options.pivot_mesh, msh)
    col = bpy.data.collections.get("Collection")
    col.objects.link(obj)
    if not options.do_not_save:
        bpy.ops.wm.save_as_mainfile()

# ------------------------------------------------------------------------------
def main():
    try:
        arg_parser = make_argument_parser()
        try: args = sys.argv[sys.argv.index("--") + 1:]
        except ValueError: args = []
        options = arg_parser.parse_args(args)
        if options.debug:
            print(options)
        else:
            generate(options)
    finally:
        try:
            import bpy
            bpy.ops.wm.quit_blender()
        except: pass
# ------------------------------------------------------------------------------
main()

