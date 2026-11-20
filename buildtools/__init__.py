import os

import dataclasses
import json

import pybuildsys

from . import util

from .config import BuildConfig
from .platform import platforms

class BuildContext:
    def __init__(self, build_config: BuildConfig):
        self.config = build_config
        self.platform = platforms[build_config.platform]

        self.toolchain = self.platform.toolchain_factory(build_config)

        self.toolchain.cflags.strict = util.default(self.config.strict, self.config.debug)
        self.toolchain.cflags.include.append(os.path.realpath(util.INCLUDE_DIR))
        self.toolchain.cflags.include.append(os.path.realpath(os.path.join(util.CORE_DIR, "include")))
        self.toolchain.cflags.include.append(os.path.realpath("."))

    def build_name(self) -> str:
        if self.config.build_dir_name:
            return self.config.build_dir_name

        name = f"{self.config.platform}"

        if self.platform.multiarch:
            name += f"-{self.config.arch}"

        if self.config.debug:
            name += "+debug"
        if util.default(self.config.optimize, not self.config.debug):
            name += "+optimized"
        if util.default(self.config.debug_info, self.config.debug):
            name += "+dbginfo"

        return name

    def build_dir(self) -> str:
        return os.path.join(util.BUILD_DIR, self.build_name())

    def output_path(self, file: str) -> str:
        base = self.build_dir()

        return os.path.join(base, file)

    def lib_output_path(self) -> str:
        return os.path.join(self.build_dir(), "libparrot.a")

    def test_output_path(self, ext: str | None = None) -> str:
        path = self.output_path("test/test")

        if ext:
            path += f".{ext}"

        return path
