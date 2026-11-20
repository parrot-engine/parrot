import os
import platform

from typing import TypeVar

T = TypeVar("T")

SRC_DIR = "src"
CORE_DIR = "core"

INCLUDE_DIR = "include"

BUILD_DIR = "build"

def default(value, default: T) -> T:
    if isinstance(value, type(default)):
        return value

    return default

def host_arch():
    machine = platform.machine().lower()

    if machine in ("x86_64", "amd64"):
        return "x86_64"
    elif machine in ("i386", "i686", "i586", "x86"):
        return "i386"
    elif machine in ("aarch64", "arm64"):
        return "aarch64"
    elif machine.startswith("arm"):
        return "arm"
    
    return machine
