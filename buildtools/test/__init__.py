import os
import glob

import copy

from functools import partial

from buildtools import util

from .. import BuildContext

from pybuildsys import Build, conditions
from pybuildsys.cc import ar, cc, ld, objcopy
from pybuildsys.build import shell

def build(ctx: BuildContext):
    srcs: list[str] = []
    objs: list[str] = []
    built_objs: list[str] = []

    srcs.extend(glob.glob("test/**/*.c", recursive=True))

    for src in srcs:
        out = ctx.output_path(src + ".o")
        out_dir = os.path.dirname(out)

        yield Build(partial(os.makedirs, out_dir, exist_ok=True))
        yield (
            cc(ctx.toolchain, out, src)
                .depends_on(partial(os.path.isdir, out_dir))
                .on_finish(lambda out=out: built_objs.append(out))
        )

        objs.append(out)

    yield (
        Build().depends_on(lambda: len(built_objs) >= len(srcs))
            .on_finish_builder(link(ctx, objs))
    )

def link(ctx: BuildContext, objs: list[str]):
    INTERMMEDIATE_PLATFORMS = {
    }

    LINKER_SCRIPT = os.path.realpath(os.path.join(util.SRC_DIR, "platform/dos/linker.ld"))

    path = ctx.test_output_path(ext=INTERMMEDIATE_PLATFORMS.get(ctx.config.platform, None))
    link = ld(ctx.toolchain, path, *objs)

    yield link
