#!/usr/bin/python3
# coding: UTF-8
# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt

import os
import io
import sys
import json
import shutil
import urllib
import zipfile
import argparse
import tempfile
import subprocess
# ------------------------------------------------------------------------------
class ArgParser(argparse.ArgumentParser):
    # --------------------------------------------------------------------------
    def __init__(self, **kw):
        # ----------------------------------------------------------------------
        def _valid_input_url(x):
            try:
                p = urllib.parse.urlparse(x)
                return x
            except:
                self.error("`%s' is not a valid URL" % str(x))

        # ----------------------------------------------------------------------
        def _valid_executable(x):
            try:
                p = os.path.realpath(shutil.which(x))
                assert os.path.isfile(p)
                assert os.access(p, os.X_OK)
                return p
            except:
                self.error("`%s' is not an existing executable file" % str(x))

        # ----------------------------------------------------------------------
        argparse.ArgumentParser.__init__(self, **kw)

        self.add_argument(
            '--debug',
            action="store_true",
            default=False)

        self.add_argument(
            '--resource-get', '-g',
            dest="resource_get",
            metavar="FILE-PATH",
            type=_valid_executable,
            action="store",
            default=_valid_executable("eagine-msgbus-resource-get"))

        self.add_argument(
            '--msgbus', '-m',
            metavar='KIND',
            dest='_connection_kind',
            nargs='?',
            choices=[
                "asio-local-stream"],
            action="store",
            default="asio-local-stream")

        self.add_argument(
            '--msgbus-addr', '-a',
            metavar='ADDRESS',
            dest='_connection_address',
            nargs='?',
            action="store",
            default="/tmp/eagibus.socket")

        self.add_argument(
            '--archive-prefix', '-p',
            metavar="PATH-PREFIX",
            dest="archive_prefix",
            action="store",
            default="")

        self.add_argument(
            '--output', '-o',
            metavar="OUTPUT-PATH",
            dest="output_path",
            type=os.path.realpath,
            action="store",
            default=None)

        self.add_argument(
            "--input", "-i",
            metavar='INPUT-URL',
            dest='input_urls',
            nargs='?',
            type=_valid_input_url,
            action="append",
            default=[])

        self.add_argument(
            "_input_urls",
            metavar='INPUT-FILE',
            nargs=argparse.REMAINDER,
            type=_valid_input_url)

    # --------------------------------------------------------------------------
    def processParsedOptions(self, options):
        if options.output_path:
            options.prefix = os.path.dirname(options.output_path)
            if not os.path.isdir(options.prefix):
                pathlib.Path(options.prefix).mkdir(parents=True, exist_ok=True)
        else:
            options.output_path = os.path.realpath("a.zip")

        return options

    # --------------------------------------------------------------------------
    def parseArgs(self, args):
        # ----------------------------------------------------------------------
        class _Options(object):
            # ------------------------------------------------------------------
            def __init__(self, base, parent):
                self.__dict__.update(base.__dict__)
                self._parent = parent

            # ------------------------------------------------------------------
            def fetchResource(self, url):
                dest = tempfile.NamedTemporaryFile(mode="w+")
                cmd = [
                    self.resource_get,
                    "--msgbus-" + self._connection_kind,
                    "--msgbus-router-address", self._connection_address,
                    "--url", url]
                p = subprocess.Popen(cmd, stdout=dest, stderr=None)
                p.wait()
                dest.flush()
                return dest

            # ------------------------------------------------------------------
            def inputs(self):
                for input_url in self.input_urls + self._input_urls:
                    with self.fetchResource(input_url) as fd:
                        try:
                            fd.seek(0)
                            yield input_url, json.load(fd)
                        except:
                            if self.debug:
                                raise
                            else:
                                self._parent.error(f"failed to parse '{input_url}'")

        # ----------------------------------------------------------------------
        options = _Options(self.processParsedOptions(
            argparse.ArgumentParser.parse_args(self, args)),
            self)

        return options

# ------------------------------------------------------------------------------
def makeArgumentParser():
    return ArgParser(
            prog=os.path.basename(__file__),
            description="""
            Reads eagitex files, uses resource provider to create blurred
            higher image-levels of the texture, generates a new eagitex file
            and saves it with all the images into a zip file.
        """)
# ------------------------------------------------------------------------------
def makeOutput(options):
    def _sharpness(level):
        if level == 1:
            return 20
        if level == 2:
            return 16
        if level == 3:
            return 8
        if level == 4:
            return 4
        if level == 5:
            return 2
        if level == 6:
            return 1
        return 0

    with zipfile.ZipFile(options.output_path, mode="w") as output:
        archive_name = os.path.basename(options.output_path)
        for url, eagitex in options.inputs():
            img0 = eagitex["images"][0]["url"]
            size = int(eagitex["width"])
            basename = os.path.basename(urllib.parse.urlparse(img0).path)
            basename = os.path.splitext(basename)[0]

            with options.fetchResource(img0) as fdi0:
                output.write(fdi0.name, arcname=f"{basename}-l0.eagitexi")

            img_prefix = f"eagires:///{options.archive_prefix}{archive_name}/{basename}-l"
            iurl = img_prefix + "0.eagitexi"
            images = [{"url":iurl}]

            for level in range(1, 8):
                size = max(size / 2, 1)
                sharpness = _sharpness(level)
                imgl = "eagitexi:///cube_map_blur?source="+\
                        urllib.parse.quote(url, safe="")+\
                        f"&level={level}&size={size}&sharpness={sharpness}"
                with options.fetchResource(imgl) as fdi:
                    output.write(fdi.name, arcname=f"{basename}-l{level}.eagitexi")
                iurl = img_prefix + f"{level}.eagitexi"
                images.append({"url":iurl})

            eagitex["levels"] = len(images)
            eagitex["min_filter"] = "linear_mipmap_linear"
            eagitex["max_filter"] = "linear"
            eagitex["images"] = images

            with tempfile.NamedTemporaryFile(mode="w+") as fdt:
                json.dump(eagitex, fdt, separators=("\n,",":"))
                fdt.flush()
                output.write(fdt.name, arcname=f"{basename}.eagitex")

# ------------------------------------------------------------------------------
#  Main
# ------------------------------------------------------------------------------
def main():
    debug = True
    try:
        arg_parser = makeArgumentParser()
        options = arg_parser.parseArgs(sys.argv[1:])
        debug = options.debug
        makeOutput(options)
    except Exception as err:
        if debug:
            raise
        else:
            print(err)

# ------------------------------------------------------------------------------
if __name__ == "__main__": main()

