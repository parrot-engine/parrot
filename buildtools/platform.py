import os
import glob

from dataclasses import dataclass, field
from typing import Callable

from . import util
from .config import BuildConfig

from pybuildsys.cc import Toolchain

@dataclass
class Platform:
    toolchain_factory: Callable[[BuildConfig], Toolchain]

    sources: list[str] = field(default_factory=list)
    drivers: list[str] = field(default_factory=list)

    multiarch: bool = False

    has_libc: bool = True

platforms: dict[str, Platform] = {
    "linux": Platform(
        toolchain_factory=lambda config: Toolchain().with_c_prefix(f"{config.arch}-linux-gnu-"),

        sources=[
            *glob.glob(os.path.join(util.SRC_DIR, "platform/linux/**/*.c"), recursive=True),
            *glob.glob(os.path.join(util.SRC_DIR, "platform/unix/**/*.c"), recursive=True),
        ],
        drivers=["gl11", "x11"],

        multiarch=True,
    ),
}
