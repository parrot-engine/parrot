import os
import glob

import copy

from functools import partial

from .. import BuildContext

from pybuildsys import Build
from pybuildsys.cc import ar, cc, ld
from pybuildsys.build import shell

def build(ctx: BuildContext):
    srcs: list[str] = []
    objs: list[str] = []
    
    srcs.extend(glob.glob("test/**/*.c", recursive=True))

    for src in srcs:
        out = ctx.output_path(src + ".o")
        out_dir = os.path.dirname(out)

        yield Build(partial(os.makedirs, out_dir, exist_ok=True))
        yield (
            cc(ctx.toolchain, out, src)
                .depends_on(partial(os.path.isdir, out_dir))
                .on_finish(lambda: objs.append(out))
        )

    yield (
        Build().depends_on(lambda: len(objs) >= len(srcs))
            .on_finish(lambda: ld(ctx.toolchain, ctx.test_output_path(), *objs))
    )
