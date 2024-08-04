# coding=utf-8
# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
# ------------------------------------------------------------------------------
import datetime
import json
import math
# ------------------------------------------------------------------------------
import bpy
import bmesh
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, EnumProperty, BoolProperty
from bpy.types import Operator
# ------------------------------------------------------------------------------
bl_info = {
    "name": "EAGimesh export add-on",
    "author": "Matus Chochlik",
    "version": (0, 1),
    "blender": (3, 0, 0),
    "support": "COMMUNITY",
    "category": "Import-Export"}
# ------------------------------------------------------------------------------
#  Helper function
# ------------------------------------------------------------------------------
def fixvec(v):
    return (v.x, v.z,-v.y)
# ------------------------------------------------------------------------------
def fixnum(x, d = None):
    return (round(x, d) if d is not None else x) if x != round(x) else int(x)
# ------------------------------------------------------------------------------
def vdiff(u, v):
    return tuple(uc-vc for uc, vc in zip(u, v))
# ------------------------------------------------------------------------------
def dotp(u, v):
    return sum(tuple(uc*vc for uc, vc in zip(u, v)))
# ------------------------------------------------------------------------------
def crossp(u, v):
    return (u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0])
# ------------------------------------------------------------------------------
def vlen(v):
    return math.sqrt(dotp(v, v))
# ------------------------------------------------------------------------------
def triangle_arms(a, b, c):
    return (vdiff(b, a), vdiff(c, a))
# ------------------------------------------------------------------------------
def degenerate_triangle(a, b, c):
    ab, ac = triangle_arms(a, b, c)
    return vlen(crossp(ab, ac)) < max(vlen(ab), vlen(ac)) * 0.000001
# ------------------------------------------------------------------------------
#  Add-on implementation
# ------------------------------------------------------------------------------
class EAGimeshExport(Operator, ExportHelper):
    """Exports selected Blender mesh(es) into EAGimesh JSON format"""

    # --------------------------------------------------------------------------
    def _items_meshes(self, context):
        def _list_meshes(context):
            for obj in context.scene.objects:
                if obj.type == 'MESH':
                    yield obj.data.name

        return tuple((name, name, f"Mesh '{name}'") for name in _list_meshes(context))

    # --------------------------------------------------------------------------
    def _items_attrib_type(self, context):
        return (
            ('float', 'float', 'floating point value'),
            ('int_16', 'int 16-bit', '16-bit signed integer value'),
            ('int_32', 'int 32-bit', '32-bit signed integer value'),
            ('uint_16', 'uint 16-bit', '16-bit unsigned integer value'),
            ('uint_32', 'uint 32-bit', '32-bit unsigned integer value'),
            ('ubyte', 'byte', 'byte (unsigned 8-bit integer) value'))

    # --------------------------------------------------------------------------
    def _items_num_precision(self, context):
        return tuple((i, str(i), f"{i} decimals") for i in [3,4,2,5,1,6])

    # --------------------------------------------------------------------------
    bl_idname = "export.eagimesh"
    bl_label = "EAGimesh export"

    filename_ext = ".json"

    filter_glob: StringProperty(
        default="*.json",
        options={'HIDDEN'},
        maxlen=255)

    export_mesh: EnumProperty(
        items=_items_meshes,
        name="Export mesh",
        description="Select the exported mesh",
        default=None)

    position_precision: EnumProperty(
        items=_items_num_precision,
        name="Position precision",
        description="Numeric precision of position vertex attribute",
        default=None)

    export_normal: BoolProperty(
        name="Normal",
        description="Export normal vectors",
        default=True)

    normal_type: EnumProperty(
        items=_items_attrib_type,
        name="Normal type",
        description="Normal value type",
        default=0)

    export_tangent: BoolProperty(
        name="Tangent",
        description="Export tangent vectors",
        default=True)

    tangent_type: EnumProperty(
        items=_items_attrib_type,
        name="Tangent type",
        description="Tangent value type",
        default=0)

    export_bitangent: BoolProperty(
        name="Bitangent",
        description="Export bitangent vectors",
        default=True)

    bitangent_type: EnumProperty(
        items=_items_attrib_type,
        name="Bitangent type",
        description="Bitangent value type",
        default=0)

    keep_degenerate: BoolProperty(
        name="Keep degenerate",
        description="Keep degenerate mesh triangles",
        default=False)

    # --------------------------------------------------------------------------
    def _triangulate(self, context, obj):
        bmsh = bmesh.new()
        bmsh.from_mesh(obj.data)
        bmesh.ops.triangulate(
            bmsh,
            faces=bmsh.faces[:],
            quad_method="BEAUTY",
            ngon_method="BEAUTY")
        mesh = bpy.data.meshes.new("temp")
        bmsh.to_mesh(mesh)
        bmsh.free()
        return mesh

    # --------------------------------------------------------------------------
    def _get_mesh(self, context):
        for obj in context.scene.objects:
            if obj.type == 'MESH':
                if self.export_mesh == obj.data.name:
                    return obj.data.name, self._triangulate(context, obj), obj
        return None, None

    # --------------------------------------------------------------------------
    def _add_metadata(self, data):
        meta = data.setdefault("metadata", {})
        meta["date"] = datetime.datetime.now(datetime.timezone.utc).isoformat()

        return data

    # --------------------------------------------------------------------------
    def _same_vert_values(self, obj, mesh, l_key, r_key, emit_info):
        l_face_index, l_loop_index, l_vert_index = l_key
        r_face_index, r_loop_index, r_vert_index = r_key

        l_meshloop = mesh.loops[l_loop_index]
        r_meshloop = mesh.loops[r_loop_index]
        l_meshvert = mesh.vertices[l_meshloop.vertex_index]
        r_meshvert = mesh.vertices[r_meshloop.vertex_index]

        if l_meshvert.co != r_meshvert.co:
            return False

        return True

    # --------------------------------------------------------------------------
    def _get_mesh_data(self, context, obj, mesh):
        result = {}
        emitted = {}
        emitted_info = {}
        for meshvert in mesh.vertices:
            emitted[meshvert.index] = set()

        vertex_index = 0
        indices = []
        positions = []

        for meshface in mesh.polygons:
            start_index = meshface.loop_start
            assert meshface.loop_total == 3

            tri_pos = tuple(
                mesh.vertices[mesh.loops[start_index+i].vertex_index].co
                for i in range(3))

            if not self.keep_degenerate:
                if degenerate_triangle(*tri_pos):
                    continue

            for tri_vert_index in range(3):
                loop_index = start_index + tri_vert_index
                meshloop = mesh.loops[loop_index]
                meshvert = mesh.vertices[meshloop.vertex_index]

                new_vert_key = (meshface.index, meshloop.index, vertex_index)

                emitted_vert = emitted[meshvert.index]
                reused_vertex = False

                for old_face_index, old_loop_index, emit_index in emitted_vert:
                    old_vert_key = (old_face_index, old_loop_index, emitted_vert)

                    if self._same_vert_values(
                            obj, mesh,
                            new_vert_key, old_vert_key,
                            emitted_info):
                        reused_vertex = True
                        indices.append(emit_index)

                if not reused_vertex:
                    positions += [
                        fixnum(x, self.position_precision)
                        for x in fixvec(meshvert.co)]

                    emitted_vert.add(new_vert_key)
                    indices.append(vertex_index)

                    vertex_index += 1

        result["vertex_count"] = vertex_index
        result["index_count"] = len(indices)
        if vertex_index < 2**16:
            result["index_type"] = "unsigned_16"
        else:
            result["index_type"] = "unsigned_32"
        result["indices"] = indices

        result.setdefault("position", []).append({
            "values_per_vertex": 3,
            "type": "float",
            "data": positions
        })

        result["instructions"] = [{
            "mode": "triangles",
            "first": 0,
            "count": len(indices),
            "index_type": result["index_type"],
            "cw_face_winding": False
        }]
        return result

    # --------------------------------------------------------------------------
    def _make_eagimesh(self, context):
        mesh_name, mesh, obj = self._get_mesh(context)
        if mesh_name is not None and mesh is not None:
            data = self._get_mesh_data(context, obj, mesh)
            data["name"] = mesh_name
            return data

    # --------------------------------------------------------------------------
    def _save(self, context):
        eagimesh = self._make_eagimesh(context)
        if eagimesh is not None:
            with open(self.filepath, 'w', encoding="utf-8") as jsonfd:
                eagimesh = self._add_metadata(eagimesh)
                json.dump(eagimesh, jsonfd, separators=(',', ':'), sort_keys=False)

        return {'FINISHED'}

    # --------------------------------------------------------------------------
    def execute(self, context):
        return self._save(context)

# ------------------------------------------------------------------------------
def menu_func_export(self, context):
    self.layout.operator(EAGimeshExport.bl_idname, text="EAGimesh Export Operator")

# ------------------------------------------------------------------------------
def register():
    bpy.utils.register_class(EAGimeshExport)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

# ------------------------------------------------------------------------------
def unregister():
    bpy.utils.unregister_class(EAGimeshExport)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)

# ------------------------------------------------------------------------------
if __name__ == "__main__":
    register()
    bpy.ops.export.eagimesh('INVOKE_DEFAULT')

