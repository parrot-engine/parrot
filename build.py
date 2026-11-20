import shutil

import argparse
import json

import pybuildsys

import buildtools
import buildtools.config

import buildtools.engine
import buildtools.test

def post_engine_builder(ctx: buildtools.BuildContext, args: argparse.Namespace):
    if args.test:
        yield from buildtools.test.build(ctx)

def builder(ctx: buildtools.BuildContext, args: argparse.Namespace):
    stage = pybuildsys.stage.Stage()

    yield (
        stage.build(pybuildsys.Build.target(buildtools.engine.build(ctx)))
            .on_finish(lambda: ctx.toolchain.ldflags.static_libs.append(ctx.lib_output_path()))
    )

    if args.test:
        yield stage.build(pybuildsys.Build.target(buildtools.test.build(ctx)))

def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command")
    subparsers.required = False

    clean_command = subparsers.add_parser("clean")
    clean_command.add_argument("--all", action="store_true")
    
    parser.add_argument("--config", required=False)

    parser.add_argument("--test", action="store_true")

    args = parser.parse_args()

    if args.config:
        with open(args.config, "r") as f:
            build_config = buildtools.config.BuildConfig(**json.load(f))
    else:
        build_config = buildtools.config.BuildConfig()

    ctx = buildtools.BuildContext(build_config)

    match args.command:
        case "clean":
            shutil.rmtree(buildtools.util.BUILD_DIR if args.all else ctx.build_dir())
        case _:
            pybuildsys.execute(builder(ctx, args))

if __name__ == "__main__":
    main()
