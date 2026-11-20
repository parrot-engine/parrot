import os
import glob

from functools import partial

import pybuildsys

from .. import BuildContext

from .. import util
from .. import config
from ..driver import drivers as driver_registry

from pybuildsys import Build
from pybuildsys.cc import ar, cc, ld
from pybuildsys.build import shell

def append_drivers(ctx: BuildContext, srcs: list[str], drivers: list[str]) -> None:
    for driver in drivers:
        if driver == "default":
            append_drivers(ctx, srcs, ctx.platform.drivers)
            continue

        srcs.extend(driver_registry[driver](ctx))

def config_target(ctx: BuildContext, path: str) -> Build:
    def generate():
        GUARD = "PARROT_PARROT_INCLUDE_CONFIG_H_"
        with open(path, "w") as f:
            print(f"#ifndef {GUARD}", file=f)

            print(f"#define PARROT_PLATFORM_{ctx.config.platform.upper()}", file=f)

            if ctx.platform.multiarch:
                print(f"#define PARROT_ARCH_{ctx.config.arch}", file=f)

            if ctx.platform.has_libc:
                print("#define PARROT_PLATFORM_HAS_LIBC", file=f)

            if ctx.config.platform == "linux":
                print("#define PARROT_PLATFORM_UNIX", file=f)

            if ctx.config.debug:
                print("#define PARROT_DEBUG", file=f)

            if ctx.config.double_precision:
                print("#define PARROT_DOUBLE_PRECISION", file=f)

            print(f"#define {GUARD}", file=f)
            print(f"#endif // {GUARD}", file=f)

    return Build(generate)

def build_lib(ctx: BuildContext):
    srcs: list[str] = []
    objs: list[str] = []

    srcs.extend(glob.glob(os.path.join(util.CORE_DIR, "src/**/*.c"), recursive=True))
    
    srcs.extend(glob.glob(os.path.join(util.SRC_DIR, "scene/**/*.c"), recursive=True))

    srcs.extend(glob.glob(os.path.join(util.SRC_DIR, "video/**/*.c"), recursive=True))

    append_drivers(ctx, srcs, ctx.config.drivers)

    for src in srcs:
        out = ctx.output_path(src + ".o")
        out_dir = os.path.dirname(out)

        yield Build(partial(os.makedirs, out_dir, exist_ok=True))
        yield (
            cc(ctx.toolchain, out, src)
                .depends_on(partial(os.path.isdir, out_dir))
                .on_finish(lambda out=out: objs.append(out))
        )

    yield (
        Build().depends_on(lambda: len(objs) >= len(srcs))
            .on_finish(lambda: ar(ctx.toolchain, ctx.lib_output_path(), *objs))
    )

def build_config(ctx: BuildContext):
    build_include_dir = ctx.output_path("include")
    ctx.toolchain.cflags.include.append(os.path.realpath(build_include_dir))

    build_include_root = os.path.join(build_include_dir, "parrot")
    yield Build(partial(os.makedirs, build_include_root, exist_ok=True))

    yield (
        config_target(ctx, os.path.join(build_include_root, "config.h"))
            .depends_on(partial(os.path.isdir, build_include_root))
    )

def build(ctx: BuildContext):
    stage = pybuildsys.stage.Stage()

    yield stage.build(Build.target(build_config(ctx)))
    yield stage.build(Build.target(build_lib(ctx)))
