# libtooling-windows

This repository contains a self-contained build of clang's libTooling for use with windows targets on visual studio.

## Notes

-   Included are static libs for Win32 and AMD64, intended to be used in an MSVC-oriented context.
-   The debug libs have been built with minor optimizations turned on, because otherwise they're just too chunky.
    This should be fine in the general case. If not, build it yourself 🤷‍♂️

## CMake settings

If you want to reproduce the binaries in this library yourself, the following CMake settings were changed:

| Option                    | Value                     |
| ------------------------- | ------------------------- |
| CMAKE_CONFIGURATION_TYPES | Debug;Release             |
| LLVM_ENABLE_RTTI          | 1                         |
| LLVM_ENABLE_PROJECTS      | clang                     |
| CMAKE_CXX_FLAGS_DEBUG     | /MDd /Zi /O1 /Ob1 /Oy /GF |

## License

Everything in `include/` is mirrored directly from the LLVM repo and is not at all my work.
It's all subject to the terms of the [Apache License v2.0 with LLVM Exceptions](include/LICENSE.txt).

Everything else (build scripts etc) is open-slather. [The Unlicense](LICENSE).
