# coding=utf-8
# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
# ------------------------------------------------------------------------------
import datetime
import json
# ------------------------------------------------------------------------------
import bpy
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

    # --------------------------------------------------------------------------
    def _add_metadata(self, data):
        meta = data.setdefault("metadata", {})
        meta["date"] = datetime.datetime.now(datetime.timezone.utc).isoformat()

        return data

    # --------------------------------------------------------------------------
    def _make_eagimesh(self, context):
        data = {}
        return self._add_metadata(data)

    # --------------------------------------------------------------------------
    def _save(self, context):
        with open(self.filepath, 'w', encoding='utf-8') as jsonfd:
            json.dump(self._make_eagimesh(context), jsonfd)

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

