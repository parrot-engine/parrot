from dataclasses import dataclass, field
from typing import Callable, Type

import os
import sys

import glob

from pybuildsys.cc import Toolchain

from . import util

@dataclass
class BuildConfig:
    platform: str = field(default_factory=lambda: sys.platform.lower())
    arch: str = field(default_factory=lambda: util.host_arch())

    debug: bool = False
    optimize: None | bool = None
    debug_info: None | bool = None
    strict: None | bool = None

    build_dir_name: None | str = None
    compile_db: bool = False

    provide_libc: None | bool = False

    double_precision: bool = False

    drivers: list[str] = field(default_factory=lambda: ["default"])

    cflags: list[str] = field(default_factory=list)
    asflags: list[str] = field(default_factory=list)
    ldflags: list[str] = field(default_factory=list)
