#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

# copy demos to root folder. local changes are preserved
# cp -n demos/*.c .

# rem tests
# clang tools/editor.c -I. -lm -lX11 -g -fsanitize=address,undefined && ./a.out
# cl tools\editor.c -I. -fsanitize=address /DEBUG /Zi && editor

# tidy environment
if [ "$1" = "tidy" ]; then
    rm 00-* 2> /dev/null
    rm fwk.o 2> /dev/null
    rm .art*.zip 2> /dev/null
    rm demos/lua/.art*.zip 2> /dev/null
    rm demos/html5/.art*.zip 2> /dev/null
    rm demos/lua/libfwk* 2> /dev/null
    rm fwk_*.* 2> /dev/null
    rm 3rd_*.* 2> /dev/null
    rm libfwk* 2> /dev/null
    rm -rf *.dSYM 2> /dev/null
    rm *.png 2> /dev/null
    rm *.mp4 2> /dev/null
    rm editor! 2> /dev/null
    exit
fi
# shortcuts for split & join amalgamation scripts
if [ "$1" = "split" ]; then
    sh tools/split.bat
    exit
fi
if [ "$1" = "join" ]; then
    sh tools/join.bat
    exit
fi
# cook
if [ "$1" = "cook" ]; then
    cc -o cook tools/cook.c -I.
    ./cook
    exit
fi

export dll=dll
export build=dbg
export args=
export cc=cc

while [ $# -ge 1 ]; do
    if [ "$1" = "help" ]; then 
        echo sh MAKE.bat
        echo sh MAKE.bat [gcc,clang,tcc] [dbg,dev,rel] [dll,static]
        echo sh MAKE.bat [tidy]
        echo sh MAKE.bat [split,join]
        exit
    fi
    if [ "$1" = "dll" ]; then 
        export dll=dll
    fi
    if [ "$1" = "static" ]; then 
        export dll=static
    fi
    if [ "$1" = "dbg" ]; then 
        export build=dbg
    fi
    if [ "$1" = "dev" ]; then 
        export build=dev
    fi
    if [ "$1" = "rel" ]; then 
        export build=rel
    fi
    if [ "$1" = "gcc" ]; then 
        export cc=gcc
    fi
    if [ "$1" = "clang" ]; then 
        export cc=clang
    fi
    if [ "$1" = "tcc" ]; then 
        export cc="tcc -D__STDC_NO_VLA__"
    fi
    if [ "$1" = "--" ]; then 
        shift
        export args=$*
        shift $#
    fi
    if [ $# -ge 1 ]; then
        shift
    fi
done

if [ "$(uname)" != "Darwin" ]; then

    # setup (ArchLinux)
    [ ! -f ".setup" ] && sudo pacman -S --noconfirm tcc && echo>.setup
    # setup (Debian, Ubuntu, etc)
    [ ! -f ".setup" ] && sudo apt-get -y update
    [ ! -f ".setup" ] && sudo apt-get -y install tcc libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev && echo>.setup       # absolute minimum
    #                      sudo apt-get -y install clang xorg-dev                                                                             # memorable, around 100 mib
    #                      sudo apt-get -y install clang xorg-dev libglfw3-dev libassimp-dev gcc                                              # initial revision
    #                      sudo apt-get -y install ffmpeg || (sudo apt-get install snapd && sudo snap install ffmpeg)                         # variant

    # pipeline
    #cc tools/ass2iqe.c   -o tools/ass2iqe.linux  -lm -ldl -lpthread -w -g -lassimp
    #cc tools/iqe2iqm.cpp -o tools/iqe2iqm.linux  -lm -ldl -lpthread -w -g -lstdc++
    #cc tools/mid2wav.c   -o tools/mid2wav.linux  -lm -ldl -lpthread -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x tools/ass2iqe.linux
    chmod +x tools/iqe2iqm.linux
    chmod +x tools/mid2wav.linux
    chmod +x tools/mod2wav.linux
    chmod +x tools/sfxr2wav.linux
    chmod +x tools/ffmpeg.linux
    chmod +x tools/cuttlefish.linux
    chmod +x tools/PVRTexToolCLI.linux
    chmod +x tools/cook.linux
    chmod +x tools/xlsx2ini.linux
    chmod +x demos/lua/luajit.linux

    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libfwk.so  && $cc -o libfwk.so split/fwk.c -shared -fPIC -w -g -lX11 -lm $args
        cp libfwk.so demos/lua/
        export import="libfwk.so -Wl,-rpath,./"
    else
    # framework (static)
        echo fwk        && $cc -c split/fwk.c -w -g    $args
        export import=fwk.o
    fi

    # editor
    echo editor         && $cc -o editor!        editor/editor.c        -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &

    # demos
    echo 00-demo      && $cc -o 00-demo      demos/00-demo.c      -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-hello     && $cc -o 00-hello     demos/00-hello.c     -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-cubemap   && $cc -o 00-cubemap   demos/00-cubemap.c   -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-collide   && $cc -o 00-collide   demos/00-collide.c   -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-anims     && $cc -o 00-anims     demos/00-anims.c     -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-scene     && $cc -o 00-scene     demos/00-scene.c     -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-shadertoy && $cc -o 00-shadertoy demos/00-shadertoy.c -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-sprite    && $cc -o 00-sprite    demos/00-sprite.c    -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-tilemap   && $cc -o 00-tilemap   demos/00-tilemap.c   -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-tiled     && $cc -o 00-tiled     demos/00-tiled.c     -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-video     && $cc -o 00-video     demos/00-video.c     -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-script    && $cc -o 00-script    demos/00-script.c    -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-socket    && $cc -o 00-socket    demos/00-socket.c    -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-easing    && $cc -o 00-easing    demos/00-easing.c    -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-font      && $cc -o 00-font      demos/00-font.c      -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-material  && $cc -o 00-material  demos/00-material.c  -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-pbr       && $cc -o 00-pbr       demos/00-pbr.c       -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-instanced && $cc -o 00-instanced demos/00-instanced.c -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-audio     && $cc -o 00-audio     demos/00-audio.c     -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import &
    echo 00-spine     && $cc -o 00-spine     demos/00-spine.c     -lm -ldl -lpthread -lX11 -w -g -Isplit $args $import
fi

if [ "$(uname)" = "Darwin" ]; then
    # setup (osx)
    export SDKROOT=$(xcrun --show-sdk-path)
    # brew install glfw

    # pipeline
    #cc tools/ass2iqe.c   -o tools/ass2iqe.osx  -w -g -lassimp
    #cc tools/iqe2iqm.cpp -o tools/iqe2iqm.osx  -w -g -lstdc++
    #cc tools/mid2wav.c   -o tools/mid2wav.osx  -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x tools/ass2iqe.osx
    chmod +x tools/iqe2iqm.osx
    chmod +x tools/mid2wav.osx
    chmod +x tools/mod2wav.osx
    chmod +x tools/sfxr2wav.osx
    chmod +x tools/ffmpeg.osx
    chmod +x tools/cuttlefish.osx
    chmod +x tools/PVRTexToolCLI.osx
    chmod +x tools/cook.osx
    chmod +x tools/xlsx2ini.osx
    chmod +x demos/lua/luajit.osx

    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libfwk     && cc -ObjC -dynamiclib -o libfwk.dylib split/fwk.c -framework cocoa -framework iokit -w -g $args
        cp libfwk.dylib demos/lua
        export import=libfwk.dylib
    else
    # framework
        echo fwk        && cc -c -ObjC split/fwk.c -w -g $args
        export import=fwk.o
    fi

    # editor
    echo editor         && cc -o editor! -ObjC  editor/editor.c        -framework cocoa -framework iokit -w -g -Isplit $import $args &

    # demos
    echo 00-demo      && cc -o 00-demo      demos/00-demo.c      -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-hello     && cc -o 00-hello     demos/00-hello.c     -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-cubemap   && cc -o 00-cubemap   demos/00-cubemap.c   -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-collide   && cc -o 00-collide   demos/00-collide.c   -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-anims     && cc -o 00-anims     demos/00-anims.c     -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-scene     && cc -o 00-scene     demos/00-scene.c     -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-shadertoy && cc -o 00-shadertoy demos/00-shadertoy.c -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-sprite    && cc -o 00-sprite    demos/00-sprite.c    -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-tilemap   && cc -o 00-tilemap   demos/00-tilemap.c   -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-tiled     && cc -o 00-tiled     demos/00-tiled.c     -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-video     && cc -o 00-video     demos/00-video.c     -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-script    && cc -o 00-script    demos/00-script.c    -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-socket    && cc -o 00-socket    demos/00-socket.c    -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-easing    && cc -o 00-easing    demos/00-easing.c    -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-font      && cc -o 00-font      demos/00-font.c      -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-material  && cc -o 00-material  demos/00-material.c  -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-pbr       && cc -o 00-pbr       demos/00-pbr.c       -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-instanced && cc -o 00-instanced demos/00-instanced.c -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-audio     && cc -o 00-audio     demos/00-audio.c     -framework cocoa -framework iokit -w -g -Isplit $import $args &
    echo 00-spine     && cc -o 00-spine     demos/00-spine.c     -framework cocoa -framework iokit -w -g -Isplit $import $args
fi

exit


:: -----------------------------------------------------------------------------
:windows

@echo off
setlocal enableDelayedExpansion

rem show help
if "%1"=="help" (
    echo %0
    echo %0 [help]
    echo %0 [info]
    echo %0 [tidy]
    echo %0 [docs]
    echo %0 [cook]
    echo %0 [bindings]
    echo %0 [checkmem]
    echo %0 [split^|join]
    echo %0 [dll^|static] [dbg^|dev^|rel] [cl^|tcc^|cc^|gcc^|clang] [-- compiler args]
    exit /b
)

rem show manual compilation infos
if "%1"=="info" (
    echo :: FWK as static object (compilation-unit^)
    echo cl  demo.c fwk.c
    echo tcc demo.c fwk.c
    echo gcc demo.c fwk.c -o demo.exe -lws2_32 -lgdi32 -lwinmm -ldbghelp -lole32 -lshell32 -lcomdlg32
    echo.
    echo :: FWK as static object (single-header^)
    echo cl  demo.c -DFWK_C
    echo tcc demo.c -DFWK_C
    echo gcc demo.c -DFWK_C -o demo.exe -lws2_32 -lgdi32 -lwinmm -ldbghelp -lole32 -lshell32 -lcomdlg32
    echo.
    echo :: FWK as dll (export^)
    echo cl  fwk.c -DAPI=EXPORT -LD
    echo tcc fwk.c -DAPI=EXPORT -shared
    echo gcc fwk.c -DAPI=EXPORT -shared -o fwk.dll -lws2_32 -lgdi32 -lwinmm -ldbghelp -lole32 -lshell32 -lcomdlg32 -Wl,--out-implib,fwk.a
    echo.
    echo :: FWK as dll (import^)
    echo cl  demo.c fwk.lib -DAPI=IMPORT
    echo tcc demo.c fwk.def -DAPI=IMPORT
    echo gcc demo.c fwk.a   -DAPI=IMPORT -o demo.exe
    echo.
    exit /b
)

rem generate tcc.bat
cd /d "%~dp0"
echo @if     "%%1"=="-impdef" @%~dp0\tools\tcc-win\tcc %%* > tcc.bat
echo @if not "%%1"=="-impdef" @%~dp0\tools\tcc-win\tcc -I %~dp0\tools\tcc-win\include_mingw\winapi -I %~dp0\tools\tcc-win\include_mingw\ %%* >> tcc.bat

rem generate cooker twice: use multi-threaded version if available (cl). then cook.
if "%1"=="cook" (
    call tools\tcc tools\cook.c -I.
                           cl tools\cook.c -I.
    cook

    exit /b
)
rem generate bindings
if "%1"=="bindings" (
    rem luajit
    tools\luajit tools\luajit_make_bindings.lua > fwk.lua
    move /y fwk.lua demos\lua

    exit /b
)
rem generate documentation
if "%1"=="docs" (

    rem set symbols...
    rem git pull
    git describe --tags --abbrev=0 > info.obj
    set /p VERSION=<info.obj
    git rev-list --count --first-parent HEAD > info.obj
    set /p GIT_REVISION=<info.obj
    git rev-parse --abbrev-ref HEAD > info.obj
    set /p GIT_BRANCH=<info.obj
    date /t > info.obj
    set /p LAST_MODIFIED=<info.obj

    rem ...and generate docs
    call tools\tcc editor\docs\docs.c split\fwk.c -Isplit %2
    docs split\fwk.h --excluded=3rd_glad.h,fwk.h,fwk_compat.h, > fwk.html
    move /y fwk.html editor\docs\docs.html

    exit /b
)
rem generate single-header distribution
if "%1"=="amalgamation" (
echo // This C file is a header that you can #include.  Do #define FWK_C  > fwk.h
echo // early in **one** C compilation unit to unroll the implementation >> fwk.h
echo // The FWK_C symbol **must be defined in a C file**; C++ wont work. >> fwk.h
echo // Looking at the FWK APIs? Open any of the split/ folders instead. >> fwk.h
echo #pragma once               >> fwk.h
type split\split\3rd_font_md.h  >> fwk.h
type split\split\3rd_glad.h     >> fwk.h
type split\fwk.h                >> fwk.h
echo #ifdef FWK_C               >> fwk.h
echo #pragma once               >> fwk.h
echo #define FWK_3RD            >> fwk.h
type split\fwk                  >> fwk.h
type split\fwk.c                >> fwk.h
echo #endif // FWK_C            >> fwk.h
exit /b
)

rem generate prior files to a github release
if "%1"=="github" (
    rem call make.bat dll
    call make.bat docs
    call make.bat bindings

    call make.bat amalgamation
    call make.bat split
    rd /q /s split\split
    md split\split
    move /y fwk_*.? split\split\
    move /y 3rd_*.? split\split\

    call make.bat tidy

    exit /b
)

rem shortcuts for split & join amalgamation scripts
if "%1"=="split" (
    call tools\split
    exit /b
)
if "%1"=="join" (
    call tools\join
    exit /b
)

rem check memory api calls
if "%1"=="checkmem" (
    findstr /RNC:"[^_xv]realloc[(]" split\fwk.c split\split\fwk*
    findstr /RNC:"[^_xv]malloc[(]"  split\fwk.c split\split\fwk*
    findstr /RNC:"[^_xv]free[(]"    split\fwk.c split\split\fwk*
    findstr /RNC:"[^_xv]calloc[(]"  split\fwk.c split\split\fwk*
    findstr /RNC:"[^_xv]strdup[(]"  split\fwk.c split\split\fwk*
    exit /b
)

rem copy demos to root folder. local changes are preserved
rem echo n | copy /-y demos\*.c 1> nul 2> nul

rem tidy environment
if "%1"=="tidy" (
    move /y ??-*.png demos
    move /y ??-*.c demos
    del demos\lua\fwk.dll
    del .temp*.*
    del *.zip
    del *.mem
    del *.exp
    del *.lib
    del *.exe
    del *.obj
    del *.o
    del *.a
    del *.pdb
    del *.ilk
    del *.png
    del *.mp4
    del *.def
    del *.dll
    del 3rd_*.*
    del fwk_*.*
rem del ??-*.*
    del temp_*.*
    rd /q /s .vs
    del tcc.bat
    del sh.bat
    exit /b
)

set dll=dll
set build=dbg
set cc=%cc%
set args=

:parse_args
    if "%1"=="--"     shift && goto parse_compiler_args

    if "%1"=="dll"    set dll=%1
    if "%1"=="static" set dll=%1

    if "%1"=="dbg"    set build=%1
    if "%1"=="dev"    set build=%1
    if "%1"=="rel"    set build=%1

    if "%1"=="tcc"    set cc=%1
    if "%1"=="cl"     set cc=%1
    if "%1"=="cc"     set cc=%1
    if "%1"=="gcc"    set cc=%1
    if "%1"=="clang"  set cc=%1

    if not "%1"==""   shift && goto parse_args

:parse_compiler_args
    if not "%1"==""   set "args=!args! %1" && shift && goto parse_compiler_args

rem setup
if "!cc!"=="" (
    echo Detecting VS 2022/2019/2017/2015/2013 x64 ...
    set cc=cl
           if exist "%VS190COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS190COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul
    ) else if exist "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul
    ) else if exist "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul
    ) else if exist "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
              @call "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" > nul
    ) else if exist "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
              @call "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" > nul
    ) else if exist "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul
    ) else (
        echo Detecting Mingw64 ...
        set cc=gcc
        where /q gcc.exe || ( set cc=tcc&&echo Detecting TCC ... )
    )
)

echo build=!build!, type=!dll!, cc=!cc!, args=!args!

rem --- pipeline
rem cl tools/ass2iqe.c   /Fetools/ass2iqe.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL assimp.lib
rem cl tools/iqe2iqm.cpp /Fetools/iqe2iqm.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
rem cl tools/mid2wav.c   /Fetools/mid2wav.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
rem cl tools/xml2json.c  /Fetools/xml2json.exe /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
rem --- pipeline
rem gcc tools/ass2iqe.c   -o tools/ass2iqe.exe  -w -lassimp
rem gcc tools/iqe2iqm.cpp -o tools/iqe2iqm.exe  -w -lstdc++
rem gcc tools/mid2wav.c   -o tools/mid2wav.exe  -w
rem gcc tools/xml2json.c  -o tools/xml2json.exe -w
rem --- different strategies for release builds
rem 4.61s 6.9MiB (default)
rem 33.7s 6.6MiB             /Ox     /Oy /MT /DNDEBUG /DFINAL
rem 35.8s 5.3MiB                 /O2 /Oy /MT /DNDEBUG /DFINAL
rem 17.9s 4.6MiB                 /O1     /MT /DNDEBUG /DFINAL /GL /GF     /arch:AVX2
rem 17.8s 4.6MiB         /Os /Ox /O2 /Oy /MT /DNDEBUG /DFINAL /GL /GF     /arch:AVX2
rem 18.8s 4.6MiB         /Os /Ox /O2 /Oy /MT /DNDEBUG         /GL /GF /Gw            /link /OPT:ICF /LTCG
rem 18.0s 4.6MiB /openmp /Os /Ox /O2 /Oy /MT /DNDEBUG /DFINAL /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG

if "!cc!"=="cl" (

    if "!dll!"=="static" (
        set export=/c
        set import=fwk.obj
    ) else (
        set export=/DAPI=EXPORT /LD
        set import=/DAPI=IMPORT fwk.lib
    )

    if "!build!"=="rel" (
        set args=/nologo /Zi /MT /DNDEBUG /DFINAL /openmp /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2 !args! /link /OPT:ICF /LTCG
    )
    if "!build!"=="dev" (
        set args=/nologo /Zi /MT                  /openmp /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2 !args!
    )
    if "!build!"=="dbg" (
        set args=/nologo /Zi /MT                          /Od                                    !args!
    )

    rem framework
    cl split\fwk.c                   !export! !args!
    if "!dll!"=="dll"   copy /y fwk.dll demos\lua > nul

    rem editor
    cl editor\editor.c      -Isplit !import! !args!

    rem demos
    cl demos\00-demo.c      -Isplit !import! !args!
    cl demos\00-hello.c     -Isplit !import! !args!
    cl demos\00-cubemap.c   -Isplit !import! !args!
    cl demos\00-collide.c   -Isplit !import! !args!
    cl demos\00-anims.c     -Isplit !import! !args!
    cl demos\00-scene.c     -Isplit !import! !args!
    cl demos\00-shadertoy.c -Isplit !import! !args!
    cl demos\00-sprite.c    -Isplit !import! !args!
    cl demos\00-tilemap.c   -Isplit !import! !args!
    cl demos\00-tiled.c     -Isplit !import! !args!
    cl demos\00-video.c     -Isplit !import! !args!
    cl demos\00-script.c    -Isplit !import! !args!
    cl demos\00-socket.c    -Isplit !import! !args!
    cl demos\00-easing.c    -Isplit !import! !args!
    cl demos\00-font.c      -Isplit !import! !args!
    cl demos\00-material.c  -Isplit !import! !args!
    cl demos\00-pbr.c       -Isplit !import! !args!
    cl demos\00-instanced.c -Isplit !import! !args!
    cl demos\00-audio.c     -Isplit !import! !args!
    cl demos\00-spine.c     -Isplit !import! !args!

) else if "!cc!"=="tcc" (

    if "!dll!"=="static" (
        set export=-c
        set import=fwk.o
    ) else (
        set export=-DAPI=EXPORT -shared
        set import=-DAPI=IMPORT fwk.def
    )

    if "!build!"=="rel" (
        set args=-DFINAL -DNDEBUG !args!
    )
    if "!build!"=="dev" (
        set args=-g !args!
    )
    if "!build!"=="dbg" (
        set args=-g !args!
    )

    rem framework
    echo fwk            && call tcc split\fwk.c                    !export! !args!
    if "!dll!"=="dll"   copy /y fwk.dll demos\lua > nul

    rem editor
    echo editor         && call tcc editor\editor.c    -Isplit !import! !args!

    rem demos
    echo 00-demo      && call tcc demos\00-demo.c      -Isplit !import! !args!
    echo 00-hello     && call tcc demos\00-hello.c     -Isplit !import! !args!
    echo 00-cubemap   && call tcc demos\00-cubemap.c   -Isplit !import! !args!
    echo 00-collide   && call tcc demos\00-collide.c   -Isplit !import! !args!
    echo 00-anims     && call tcc demos\00-anims.c     -Isplit !import! !args!
    echo 00-scene     && call tcc demos\00-scene.c     -Isplit !import! !args!
    echo 00-shadertoy && call tcc demos\00-shadertoy.c -Isplit !import! !args!
    echo 00-sprite    && call tcc demos\00-sprite.c    -Isplit !import! !args!
    echo 00-tilemap   && call tcc demos\00-tilemap.c   -Isplit !import! !args!
    echo 00-tiled     && call tcc demos\00-tiled.c     -Isplit !import! !args!
    echo 00-video     && call tcc demos\00-video.c     -Isplit !import! !args!
    echo 00-script    && call tcc demos\00-script.c    -Isplit !import! !args!
    echo 00-socket    && call tcc demos\00-socket.c    -Isplit !import! !args!
    echo 00-easing    && call tcc demos\00-easing.c    -Isplit !import! !args!
    echo 00-font      && call tcc demos\00-font.c      -Isplit !import! !args!
    echo 00-material  && call tcc demos\00-material.c  -Isplit !import! !args!
    echo 00-pbr       && call tcc demos\00-pbr.c       -Isplit !import! !args!
    echo 00-instanced && call tcc demos\00-instanced.c -Isplit !import! !args!
    echo 00-audio     && call tcc demos\00-audio.c     -Isplit !import! !args!
    echo 00-spine     && call tcc demos\00-spine.c     -Isplit !import! !args!

) else ( rem if "!cc!"=="gcc" or "clang"

    set libs=-lws2_32 -lgdi32 -lwinmm -ldbghelp -lole32 -lshell32 -lcomdlg32

    if "!dll!"=="static" (
        set export=-c
        set import=fwk.o !libs! -Wl,--allow-multiple-definition
    ) else (
        set export=-DAPI=EXPORT -shared -o fwk.dll !libs! -Wl,--out-implib,fwk.a
        set import=-DAPI=IMPORT fwk.a
    )

    if "!build!"=="rel" (
        set args=-g -O3 -DFINAL -DNDEBUG !args!
    )
    if "!build!"=="dev" (
        set args=-g -O2 !args!
    )
    if "!build!"=="dbg" (
        set args=-g -O0 !args!
    )

    rem framework
    echo fwk            && !cc! split\fwk.c -w !export! !args!
    if "!dll!"=="dll"   copy /y fwk.dll demos\lua > nul

    rem editor
    echo editor       && !cc! -o editor       editor\editor.c      -Isplit !import! !args!

    rem demos
    echo 00-demo      && !cc! -o 00-demo      demos\00-demo.c      -Isplit !import! !args!
    echo 00-hello     && !cc! -o 00-hello     demos\00-hello.c     -Isplit !import! !args!
    echo 00-cubemap   && !cc! -o 00-cubemap   demos\00-cubemap.c   -Isplit !import! !args!
    echo 00-collide   && !cc! -o 00-collide   demos\00-collide.c   -Isplit !import! !args!
    echo 00-anims     && !cc! -o 00-anims     demos\00-anims.c     -Isplit !import! !args!
    echo 00-scene     && !cc! -o 00-scene     demos\00-scene.c     -Isplit !import! !args!
    echo 00-shadertoy && !cc! -o 00-shadertoy demos\00-shadertoy.c -Isplit !import! !args!
    echo 00-sprite    && !cc! -o 00-sprite    demos\00-sprite.c    -Isplit !import! !args!
    echo 00-tilemap   && !cc! -o 00-tilemap   demos\00-tilemap.c   -Isplit !import! !args!
    echo 00-tiled     && !cc! -o 00-tiled     demos\00-tiled.c     -Isplit !import! !args!
    echo 00-video     && !cc! -o 00-video     demos\00-video.c     -Isplit !import! !args!
    echo 00-script    && !cc! -o 00-script    demos\00-script.c    -Isplit !import! !args!
    echo 00-socket    && !cc! -o 00-socket    demos\00-socket.c    -Isplit !import! !args!
    echo 00-easing    && !cc! -o 00-easing    demos\00-easing.c    -Isplit !import! !args!
    echo 00-font      && !cc! -o 00-font      demos\00-font.c      -Isplit !import! !args!
    echo 00-material  && !cc! -o 00-material  demos\00-material.c  -Isplit !import! !args!
    echo 00-pbr       && !cc! -o 00-pbr       demos\00-pbr.c       -Isplit !import! !args!
    echo 00-instanced && !cc! -o 00-instanced demos\00-instanced.c -Isplit !import! !args!
    echo 00-audio     && !cc! -o 00-audio     demos\00-audio.c     -Isplit !import! !args!
    echo 00-spine     && !cc! -o 00-spine     demos\00-spine.c     -Isplit !import! !args!
)

rem PAUSE only if double-clicked from Windows explorer
rem (((echo.%cmdcmdline%)|%WINDIR%\system32\find.exe /I "%~0")>nul)&&pause

exit /b
