# libtooling-windows

This repository contains a self-contained build of clang's libTooling for use with windows targets on visual studio.

## Notes

-   Included are static libs for x86 and x64, intended to be used in an MSVC-oriented context.
-   Libs have been built with options prioritizing smallest binary size, regardless of debug/release mode,
    otherwise they're just too chunky.
-   The exact commit of [llvm/llvm-project] from which the libs were built can be found in [`LLVM_COMMIT`].

## CMake settings

If you want to reproduce the binaries in this library yourself, the following CMake settings were changed:

| Option                            | Value                                                                                                                       |
| --------------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| CMAKE_CONFIGURATION_TYPES         | Debug;Release                                                                                                               |
| LLVM_ENABLE_RTTI                  | 1                                                                                                                           |
| LLVM_ENABLE_PROJECTS              | clang                                                                                                                       |
| CMAKE_CXX_FLAGS                   | /DWIN32 /D_WINDOWS /W3 /GR /EHsc /Oy /GF /Gy /GS- /GL- /Zo- /sdl- /permissive- /volatile:iso /Zc:inline /Zc:externConstexpr |
| CMAKE_CXX_FLAGS_DEBUG             | /MDd /O1 /Ob1                                                                                                               |
| CMAKE_CXX_FLAGS_RELEASE           | /MD /O2 /Ob2 /DNDEBUG                                                                                                       |
| CMAKE_STATIC_LINKER_FLAGS (x86)   | /machine:X86 /LTCG:OFF                                                                                                      |
| CMAKE_STATIC_LINKER_FLAGS (x64)   | /machine:X64 /LTCG:OFF                                                                                                      |
| CMAKE_STATIC_LINKER_FLAGS_DEBUG   | _none_                                                                                                                      |
| CMAKE_STATIC_LINKER_FLAGS_RELEASE | _none_                                                                                                                      |

## License

Everything in `include/` is mirrored directly from the LLVM repo and is not my work.
It's subject to the terms of the [Apache License v2.0 with LLVM Exceptions](include/LICENSE.txt).

Everything else (build scripts etc) is open-slather. [The Unlicense](LICENSE).

[llvm/llvm-project]: https://github.com/llvm/llvm-project
[`llvm_commit`]: LLVM_COMMIT
