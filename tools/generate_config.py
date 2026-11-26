import os
import sys

GUARD = "PARROT_PARROT_INCLUDE_PARROT_CONFIG_H_"

def main():
    output = sys.argv[1]

    with open(output, "w") as f:
        print(f"#ifndef {GUARD}", file=f)
        print(f"#define {GUARD}", file=f)

        print(f"#define PARROT_PLATFORM_{os.environ["CONFIG_PLATFORM"].upper()}", file=f)

        if os.environ.get("PLATFORM_MULTIARCH", False):
            print(f"#define PARROT_PLATFORM_{os.environ["CONFIG_ARCH"].upper()}", file=f)

        if os.environ.get("PLATFORM_HAS_LIBC", False):
            print(f"#define PARROT_PLATFORM_HAS_LIBC", file=f)

        if os.environ.get("PLATFORM_UNIX", False):
            print(f"#define PARROT_PLATFORM_HAS_LIBC", file=f)

        if os.environ.get("DEBUG", False):
            print(f"#define PARROT_DEBUG", file=f)

        if os.environ.get("DOUBLE_PRECISION", False):
            print(f"#define PARROT_DOUBLE_PRECISION", file=f)

        print(f"#endif // {GUARD}", file=f)

if __name__ == "__main__":
    main()
