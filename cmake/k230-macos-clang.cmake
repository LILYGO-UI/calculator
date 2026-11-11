# Homebrew LLVM can target Linux/riscv64; Apple's Clang cannot.
set(LILYGO_UI_DEBIAN_SYSROOT "$ENV{LILYGO_UI_DEBIAN_SYSROOT}" CACHE PATH
    "Debian riscv64 development sysroot")
if(NOT LILYGO_UI_DEBIAN_SYSROOT)
    get_filename_component(_k230_default_sysroot
        "${CMAKE_CURRENT_LIST_DIR}/../.cache/k230-debian/sysroot" ABSOLUTE)
    if(EXISTS "${_k230_default_sysroot}/usr/include")
        set(LILYGO_UI_DEBIAN_SYSROOT "${_k230_default_sysroot}" CACHE PATH
            "Debian riscv64 development sysroot" FORCE)
    else()
        message(FATAL_ERROR
            "Prepare a Debian riscv64 sysroot with:\n"
            "  python3 tools/prepare-riscv64-sysroot.py\n"
            "Or set LILYGO_UI_DEBIAN_SYSROOT to an existing development sysroot.")
    endif()
endif()

# Preserve a compiler selected explicitly by the caller.
if(LILYGO_UI_K230_USE_CLANG OR CMAKE_C_COMPILER MATCHES "/clang$" OR
   (NOT CMAKE_C_COMPILER AND NOT LILYGO_UI_DEBIAN_CROSS_PREFIX AND
    "$ENV{LILYGO_UI_DEBIAN_CROSS_PREFIX}" STREQUAL ""))
    set(LILYGO_UI_K230_USE_CLANG ON CACHE INTERNAL "Use native LLVM for K230")
    find_program(_k230_llvm_config NAMES llvm-config
        HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin)
    if(NOT _k230_llvm_config)
        message(FATAL_ERROR "Install the macOS cross compiler with: brew install llvm lld")
    endif()
    execute_process(COMMAND "${_k230_llvm_config}" --bindir
        OUTPUT_VARIABLE _k230_llvm_bin OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY)
    find_program(_k230_lld NAMES ld.lld
        HINTS "${_k230_llvm_bin}" /opt/homebrew/opt/lld/bin /usr/local/opt/lld/bin
        REQUIRED)
    set(CMAKE_C_COMPILER "${_k230_llvm_bin}/clang" CACHE FILEPATH "RISC-V C compiler")
    set(CMAKE_CXX_COMPILER "${_k230_llvm_bin}/clang++" CACHE FILEPATH "RISC-V C++ compiler")
    set(CMAKE_AR "${_k230_llvm_bin}/llvm-ar" CACHE FILEPATH "Target archiver")
    set(CMAKE_RANLIB "${_k230_llvm_bin}/llvm-ranlib" CACHE FILEPATH "Target archive indexer")
    set(CMAKE_STRIP "${_k230_llvm_bin}/llvm-strip" CACHE FILEPATH "Target strip tool")
endif()

if(CMAKE_C_COMPILER MATCHES "/clang$")
    set(CMAKE_C_COMPILER_TARGET riscv64-linux-gnu)
    set(CMAKE_CXX_COMPILER_TARGET riscv64-linux-gnu)
    set(CMAKE_ASM_COMPILER_TARGET riscv64-linux-gnu)
    file(GLOB _k230_gcc_dirs LIST_DIRECTORIES TRUE
        "${LILYGO_UI_DEBIAN_SYSROOT}/usr/lib/gcc/riscv64-linux-gnu/*")
    list(SORT _k230_gcc_dirs COMPARE NATURAL ORDER DESCENDING)
    if(NOT _k230_gcc_dirs)
        message(FATAL_ERROR "The sysroot must include libgcc-14-dev and libstdc++-14-dev")
    endif()
    list(GET _k230_gcc_dirs 0 _k230_gcc_dir)
    set(CMAKE_C_FLAGS_INIT "--gcc-install-dir=\"${_k230_gcc_dir}\"")
    set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")
    set(CMAKE_ASM_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")
    set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=\"${_k230_lld}\"")
    set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")
    list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
        LILYGO_UI_K230_USE_CLANG _k230_llvm_config _k230_lld
        CMAKE_AR CMAKE_RANLIB CMAKE_STRIP)
endif()
