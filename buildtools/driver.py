import os
import glob

from dataclasses import dataclass
from typing import Callable

from . import BuildContext

from . import util

type Driver = Callable[[BuildContext], list[str]]

def gl11_driver(ctx: BuildContext) -> list[str]:
    if ctx.config.platform == "linux":
        ctx.toolchain.ldflags.libs.append("GL")

    return glob.glob(os.path.join(util.SRC_DIR, "drivers/gl11/**/*.c"), recursive=True)

def x11_driver(ctx: BuildContext) -> list[str]:
    ctx.toolchain.ldflags.libs.append("X11")
    ctx.toolchain.ldflags.libs.append("Xext")

    return glob.glob(os.path.join(util.SRC_DIR, "drivers/x11/**/*.c"), recursive=True)

drivers: dict[str, Driver] = {
    "gl11": gl11_driver,
    "x11": x11_driver,
}
