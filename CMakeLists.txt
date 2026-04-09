cmake_minimum_required(VERSION 3.16)
project(sspipes CXX C RC)

# -------------------------------------------------------------------------
# Compiler flags
# -------------------------------------------------------------------------

# C++11 for modern for-loop scoping, nullptr, etc.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows subsystem (GUI, not console); statically link the MinGW C/C++ runtime
# so the .scr is self-contained and doesn't need libgcc_s_seh-1.dll / libstdc++-6.dll
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -static-libgcc -static-libstdc++")

# Suppress common compatibility warnings from the NT SDK source
add_compile_options(
    -Wno-narrowing
    -Wno-write-strings
    -Wno-unused-result
    -Wno-deprecated-declarations
    -Wno-format
    -fpermissive          # allow old-style C++ constructs
)

# -------------------------------------------------------------------------
# Include directories
# -------------------------------------------------------------------------

include_directories(
    src/common       # sscommon.h, sscommon.hxx, glscrnsv.h, glscrnsv.hxx, etc.
    src/pipes        # sspipes.h, dialog.h, resource.h, etc.
    glaux            # teapot.h (local data arrays)
    resources        # for RC file to find BMP and ICO
)

# -------------------------------------------------------------------------
# Common screensaver framework (SCRSAVE/COMMON)
# -------------------------------------------------------------------------

set(COMMON_SOURCES
    src/common/scrnsave.cxx
    src/common/glscrnsv.cxx
    src/common/ssinit.cxx
    src/common/ssutil.cxx
    src/common/sswindow.cxx
    src/common/sswproc.cxx
    src/common/palette.cxx
    src/common/clear.cxx
    src/common/dlgdraw.cxx
    src/common/util.cxx
    src/common/color.c
    src/common/dialog.c
    src/common/fastdib.c
    src/common/material.c
    src/common/math.c
    src/common/ssa8.c
    src/common/ssdib.c
    src/common/ssimage.c
    src/common/texture.c
)

# -------------------------------------------------------------------------
# Pipes screensaver (SCRSAVE/PIPES)
# -------------------------------------------------------------------------

set(PIPES_SOURCES
    src/pipes/sspipes.cxx
    src/pipes/state.cxx
    src/pipes/nstate.cxx
    src/pipes/fstate.cxx
    src/pipes/npipe.cxx
    src/pipes/fpipe.cxx
    src/pipes/pipe.cxx
    src/pipes/objects.cxx
    src/pipes/eval.cxx
    src/pipes/node.cxx
    src/pipes/view.cxx
    src/pipes/xc.cxx
    src/pipes/dialog.c
)

# -------------------------------------------------------------------------
# GLAUX teapot (replaces GLAUX.LIB)
# -------------------------------------------------------------------------

set(GLAUX_SOURCES
    glaux/teapot.c
    glaux/glaux_image.c
)

# -------------------------------------------------------------------------
# Resource file
# -------------------------------------------------------------------------

set(RC_FILE resources/sspipes.rc)

# windres needs to find the headers and BMPs
set(CMAKE_RC_FLAGS
    "-I${CMAKE_SOURCE_DIR}/src/pipes \
     -I${CMAKE_SOURCE_DIR}/src/common \
     -I${CMAKE_SOURCE_DIR}/resources \
     -I${CMAKE_SYSROOT}/include \
     -I/usr/x86_64-w64-mingw32/include"
)

# -------------------------------------------------------------------------
# Executable (the screensaver is just a renamed .exe)
# -------------------------------------------------------------------------

add_executable(sspipes WIN32
    ${COMMON_SOURCES}
    ${PIPES_SOURCES}
    ${GLAUX_SOURCES}
    ${RC_FILE}
)

# -------------------------------------------------------------------------
# Link libraries
# -------------------------------------------------------------------------

target_link_libraries(sspipes
    opengl32
    glu32
    gdi32
    user32
    comdlg32
    winmm
    advapi32    # for registry functions
    comctl32    # for trackbar common control
    imm32       # for IMM (Input Method Manager, used in scrnsave.cxx)
)

# -------------------------------------------------------------------------
# Output as .scr (same as .exe, just renamed)
# -------------------------------------------------------------------------

set_target_properties(sspipes PROPERTIES
    OUTPUT_NAME "sspipes"
    SUFFIX ".scr"
)
