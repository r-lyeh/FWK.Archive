#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

# copy demos to root folder. local changes are preserved
# cp -n demos/*.c .

# rem tests
# clang editor.c -I. -lm -lX11 -g -fsanitize=address,undefined && ./a.out
# cl editor.c -I. -fsanitize=address /DEBUG /Zi && editor

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
    rm Editor 2> /dev/null
    rm editor.osx 2> /dev/null
    rm temp_* 2> /dev/null
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
# sync
if [ "$1" = "sync" ]; then
    git reset --hard HEAD^^1 && git pull
    sh MAKE.bat tidy
    exit
fi

export dll=dll
export build=dev
export args=
export cc=cc

while [ $# -ge 1 ]; do
    if [ "$1" = "help" ]; then 
        echo sh MAKE.bat
        echo sh MAKE.bat [gcc,clang,tcc] [dbg,dev,rel] [dll,static]
        echo sh MAKE.bat [tidy]
        echo sh MAKE.bat [split,join]
	echo sh MAKE.bat [cook]
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
        export flags="-O0 -g"
    fi
    if [ "$1" = "dev" ]; then 
        export build=dev
        export flags="-O1 -g"
    fi
    if [ "$1" = "rel" ]; then 
        export build=rel
        export flags="-O3 -DNDEBUG"
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
    chmod +x tools/cook.linux
    chmod +x tools/cuttlefish.linux
    chmod +x tools/ffmpeg.linux
    chmod +x tools/furnace.linux
    chmod +x tools/iqe2iqm.linux
    chmod +x tools/mid2wav.linux
    chmod +x tools/mod2wav.linux
    chmod +x tools/PVRTexToolCLI.linux
    chmod +x tools/sfxr2wav.linux
    chmod +x tools/xlsx2ini.linux
    chmod +x demos/lua/luajit.linux

    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libfwk.so  && $cc -o libfwk.so tools/fwk.c -shared -fPIC -w -lX11 -lm -ldl -lpthread $flags $args
        cp libfwk.so demos/lua/
        export import="libfwk.so -Wl,-rpath,./"
    else
    # framework (static)
        echo fwk        && $cc -c tools/fwk.c -w    $flags $args
        export import=fwk.o
    fi

    # editor
    echo editor       && $cc -o Editor       editor/editor.c      -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &

    # demos
    echo 00-demo      && $cc -o 00-demo      demos/00-demo.c      -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-hello     && $cc -o 00-hello     demos/00-hello.c     -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-cubemap   && $cc -o 00-cubemap   demos/00-cubemap.c   -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-collide   && $cc -o 00-collide   demos/00-collide.c   -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-anims     && $cc -o 00-anims     demos/00-anims.c     -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-scene     && $cc -o 00-scene     demos/00-scene.c     -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-shadertoy && $cc -o 00-shadertoy demos/00-shadertoy.c -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-sprite    && $cc -o 00-sprite    demos/00-sprite.c    -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-tilemap   && $cc -o 00-tilemap   demos/00-tilemap.c   -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-tiled     && $cc -o 00-tiled     demos/00-tiled.c     -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-video     && $cc -o 00-video     demos/00-video.c     -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-script    && $cc -o 00-script    demos/00-script.c    -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-socket    && $cc -o 00-socket    demos/00-socket.c    -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-easing    && $cc -o 00-easing    demos/00-easing.c    -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-font      && $cc -o 00-font      demos/00-font.c      -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-material  && $cc -o 00-material  demos/00-material.c  -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-pbr       && $cc -o 00-pbr       demos/00-pbr.c       -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-instanced && $cc -o 00-instanced demos/00-instanced.c -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-audio     && $cc -o 00-audio     demos/00-audio.c     -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import &
    echo 00-spine     && $cc -o 00-spine     demos/00-spine.c     -lm -ldl -lpthread -lX11 -w -Itools/ $flags $args $import
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
    chmod +x tools/cook.osx
    chmod +x tools/cuttlefish.osx
    chmod +x tools/ffmpeg.osx
    chmod +x tools/furnace.osx
    chmod +x tools/iqe2iqm.osx
    chmod +x tools/mid2wav.osx
    chmod +x tools/mod2wav.osx
    chmod +x tools/PVRTexToolCLI.osx
    chmod +x tools/sfxr2wav.osx
    chmod +x tools/xlsx2ini.osx
    chmod +x demos/lua/luajit.osx

    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libfwk     && cc -ObjC -dynamiclib -o libfwk.dylib tools/fwk.c -framework cocoa -framework iokit -framework audiotoolbox -w $flags $args
        cp libfwk.dylib demos/lua
        export import=libfwk.dylib
    else
    # framework
        echo fwk        && cc -c -ObjC tools/fwk.c -w $flags $args
        export import=fwk.o
    fi

    # editor
    echo editor       && cc -o editor.osx -ObjC editor/editor.c  -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &

    # demos
    echo 00-demo      && cc -o 00-demo      demos/00-demo.c      -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-hello     && cc -o 00-hello     demos/00-hello.c     -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-cubemap   && cc -o 00-cubemap   demos/00-cubemap.c   -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-collide   && cc -o 00-collide   demos/00-collide.c   -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-anims     && cc -o 00-anims     demos/00-anims.c     -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-scene     && cc -o 00-scene     demos/00-scene.c     -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-shadertoy && cc -o 00-shadertoy demos/00-shadertoy.c -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-sprite    && cc -o 00-sprite    demos/00-sprite.c    -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-tilemap   && cc -o 00-tilemap   demos/00-tilemap.c   -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-tiled     && cc -o 00-tiled     demos/00-tiled.c     -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-video     && cc -o 00-video     demos/00-video.c     -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-script    && cc -o 00-script    demos/00-script.c    -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-socket    && cc -o 00-socket    demos/00-socket.c    -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-easing    && cc -o 00-easing    demos/00-easing.c    -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-font      && cc -o 00-font      demos/00-font.c      -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-material  && cc -o 00-material  demos/00-material.c  -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-pbr       && cc -o 00-pbr       demos/00-pbr.c       -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-instanced && cc -o 00-instanced demos/00-instanced.c -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-audio     && cc -o 00-audio     demos/00-audio.c     -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args &
    echo 00-spine     && cc -o 00-spine     demos/00-spine.c     -framework cocoa -framework iokit -framework audiotoolbox -w -Itools/ $import $flags $args
fi

exit


:: -----------------------------------------------------------------------------
:windows

@echo off
setlocal enableDelayedExpansion
cd /d "%~dp0"

rem show help
if "%1"=="-?" goto showhelp
if "%1"=="-h" goto showhelp
if "%1"=="help" (
    :showhelp
    echo %0                   ; compile everything: `make dll dev` alias
    echo %0 [help]            ; show this screen
    echo %0 [docs]            ; generate tools/docs/docs.html file
    echo %0 [cook]            ; cook .zipfiles with tools/cook.ini cookbook
    echo %0 [sync]            ; sync repo to latest
    echo %0 [tidy]            ; clean up temp files
    echo %0 [bindings]        ; generate demos/lua bindings
    echo %0 [checkmem]        ; check untracked allocators in FWK
    echo %0 [split^|join]      ; split tools/fwk* into tools/split/*, or join /fwk* into tools/fwk*
    echo %0 [cl^|tcc^|cc^|gcc^|clang^|clang-cl] [dbg^|dev^|rel] [static^|dll] [nofwk^|nodemos^|noeditor] [vis] [-- args]
    echo    cl       \ select compiler in PATH (autodetected when no compiler is provided^)
    echo    tcc      ^| select compiler in PATH (autodetected when no compiler is provided^)
    echo    cc       ^| select compiler in PATH (autodetected when no compiler is provided^)
    echo    gcc      ^| select compiler in PATH (autodetected when no compiler is provided^)
    echo    clang    ^| select compiler in PATH (autodetected when no compiler is provided^)
    echo    clang-cl / select compiler in PATH (autodetected when no compiler is provided^)
    echo    dbg      \     debug build: [x] ASAN [x] poison [x] asserts [x] debug symbols [ ] no  optimizations
    echo    dev      ^| developer build: [ ] ASAN [x] poison [x] asserts [x] debug symbols [*] mid optimizations (default^)
    echo    rel      /   release build: [ ] ASAN [ ] poison [ ] asserts [x] debug symbols [x] all optimizations
    echo    static   \ build and link fwk as static library
    echo    dll      / build and link fwk as dll (default^)
    echo    nofwk    \ do not export framework
    echo    nodemos  ^| do not compile demos
    echo    noeditor / do not compile editor
    echo    vis      ^> visualize invokation cmdline.
    echo    args     ^> after `--` separator is found, pass all remaining arguments to compiler as-is
    echo.
    exit /b
)

rem sync repo to latest
if "%1"=="sync" (
    call MAKE.bat tidy
    git reset --hard HEAD~1 && git pull
    exit /b
)

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
    cl   tools\docs\docs.c tools\fwk.c -Itools %2
    docs tools\fwk.h --excluded=3rd_glad.h,fwk.h,fwk_compat.h, > fwk.html
    rem move /y fwk.html tools\docs\docs.html

    exit /b
)
rem generate single-header distribution
if "%1"=="amalgamation" (
echo // This C file is a header that you can #include. Add #define FWK_C  > fwk.h
echo // early in **one** C compilation unit to unroll the implementation >> fwk.h
echo // The FWK_C symbol **must be defined in a C file**; C++ wont work. >> fwk.h
echo // This file is intended to be consumed by a compiler. Do not read. >> fwk.h
echo // **Browse to any of the sources in tools/split/ folder instead**. >> fwk.h
echo #pragma once                                                        >> fwk.h
type tools\split\3rd_font_md.h                                           >> fwk.h
type tools\split\3rd_glad.h                                              >> fwk.h
type tools\fwk.h                                                         >> fwk.h
echo #ifdef FWK_C                                                        >> fwk.h
echo #define FWK_3RD                                                     >> fwk.h
type tools\fwk                                                           >> fwk.h
type tools\fwk.c                                                         >> fwk.h
echo #endif // FWK_C                                                     >> fwk.h
exit /b
)

rem generate prior files to a github release
if "%1"=="github" (
    rem call make.bat dll
    call make.bat docs
    call make.bat bindings

    call make.bat amalgamation
    call make.bat split
    rd /q /s tools\split
    md tools\split
    move /y fwk_*.? tools\split\
    move /y 3rd_*.? tools\split\

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
    findstr /RNC:"[^_xv]realloc[(]" tools\fwk.c tools\split\fwk*
    findstr /RNC:"[^_xv]malloc[(]"  tools\fwk.c tools\split\fwk*
    findstr /RNC:"[^_xv]free[(]"    tools\fwk.c tools\split\fwk*
    findstr /RNC:"[^_xv]calloc[(]"  tools\fwk.c tools\split\fwk*
    findstr /RNC:"[^_xv]strdup[(]"  tools\fwk.c tools\split\fwk*
    exit /b
)

rem copy demos to root folder. local changes are preserved
rem echo n | copy /-y demos\*.c 1> nul 2> nul

rem tidy environment
if "%1"=="tidy" (
    move /y ??-*.png demos          > nul 2> nul
    move /y ??-*.c demos            > nul 2> nul
    del demos\lua\fwk.dll           > nul 2> nul
    del .temp*.*                    > nul 2> nul
    del *.zip                       > nul 2> nul
    del *.mem                       > nul 2> nul
    del *.exp                       > nul 2> nul
    del *.lib                       > nul 2> nul
    del *.exe                       > nul 2> nul
    del *.obj                       > nul 2> nul
    del *.o                         > nul 2> nul
    del *.a                         > nul 2> nul
    del *.pdb                       > nul 2> nul
    del *.ilk                       > nul 2> nul
    del *.png                       > nul 2> nul
    del *.mp4                       > nul 2> nul
    del *.def                       > nul 2> nul
    del *.dll                       > nul 2> nul
    del 3rd_*.*                     > nul 2> nul
    del fwk_*.*                     > nul 2> nul
rem del ??-*.*                      > nul 2> nul
    del temp_*.*                    > nul 2> nul
    rd /q /s .vs                    > nul 2> nul
    del tcc.bat                     > nul 2> nul
    del sh.bat                      > nul 2> nul
    exit /b
)

set cc=%cc%
set dll=dll
set build=dev
set args=-Itools
set other=
set fwk=yes
set demos=yes
set editor=yes
set vis=no

:parse_args
    if "%1"=="--"       shift && goto parse_compiler_args

    if "%1"=="dll"      set "dll=%1" && goto loop
    if "%1"=="static"   set "dll=%1" && goto loop

    if "%1"=="dbg"      set "build=%1" && goto loop
    if "%1"=="dev"      set "build=%1" && goto loop
    if "%1"=="rel"      set "build=%1" && goto loop

    if "%1"=="vis"      set "vis=yes" && goto loop

    if "%1"=="nofwk"    set "fwk=no" && goto loop
    if "%1"=="nodemos"  set "demos=no" && goto loop
    if "%1"=="noeditor" set "editor=no" && goto loop

    if "%1"=="tcc"      set "cc=call tools\tcc" && goto loop
    if "%1"=="cl"       set "cc=%1" && goto loop
    if "%1"=="vc"       set "cc=cl" && goto loop
    if "%1"=="cc"       set "cc=%1" && goto loop
    if "%1"=="gcc"      set "cc=%1" && goto loop
    if "%1"=="clang"    set "cc=%1" && goto loop
    if "%1"=="clang-cl" set "cc=%1" && goto loop

    if not "%1"==""     set "other=!other! %1" && set "editor=no" && set "demos=no"

:loop
    if not "%1"==""     shift && goto parse_args

:parse_compiler_args
    if not "%1"==""     set "args=!args! %1" && shift && goto parse_compiler_args

rem detect setup
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

rem --- pipeline
rem cl tools/ass2iqe.c   /Fetools/ass2iqe.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG assimp.lib
rem cl tools/iqe2iqm.cpp /Fetools/iqe2iqm.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG
rem cl tools/mid2wav.c   /Fetools/mid2wav.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG
rem cl tools/xml2json.c  /Fetools/xml2json.exe /nologo /openmp /O2 /Oy /MT /DNDEBUG
rem --- pipeline
rem gcc tools/ass2iqe.c   -o tools/ass2iqe.exe  -w -lassimp
rem gcc tools/iqe2iqm.cpp -o tools/iqe2iqm.exe  -w -lstdc++
rem gcc tools/mid2wav.c   -o tools/mid2wav.exe  -w
rem gcc tools/xml2json.c  -o tools/xml2json.exe -w
rem --- different strategies for release builds
rem  4.6s 6.9MiB (default)
rem 33.7s 6.6MiB             /Ox     /Oy /MT /DNDEBUG
rem 35.8s 5.3MiB                 /O2 /Oy /MT /DNDEBUG
rem 17.9s 4.6MiB                 /O1     /MT /DNDEBUG /GL /GF     /arch:AVX2
rem 17.8s 4.6MiB         /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF     /arch:AVX2
rem 18.8s 4.6MiB         /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw            /link /OPT:ICF /LTCG
rem 18.0s 4.6MiB /openmp /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG

if "!cc!"=="cl" (

    if "!dll!"=="static" (
        set export=/c
        set import=fwk.obj
    ) else (
        set export=/DAPI=EXPORT /LD
        set import=/DAPI=IMPORT fwk.lib
    )

    if "!build!"=="rel" (
        set args=/nologo     /MT /openmp /DNDEBUG !args!        /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
    )
    if "!build!"=="dev" (
        set args=/nologo /Zi /MT /openmp /DEBUG   !args! && REM /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2
    )
    if "!build!"=="dbg" (
        set args=/nologo /Zi /MT         /DEBUG   !args!        /Od  /fsanitize=address
        rem make -- /RTC1, or make -- /Zi /fsanitize=address /DEBUG
    )

    set o=/Fe:
    set echo=REM

) else if "!cc!"=="clang-cl" (

    if "!dll!"=="static" (
        set export=/c
        set import=fwk.obj
    ) else (
        set export=/DAPI=EXPORT /LD
        set import=/DAPI=IMPORT fwk.lib
    )

    set warnings_fwkc=-Wno-deprecated-declarations -Wno-tautological-constant-out-of-range-compare
    set warnings_demos=-Wno-empty-body -Wno-format-security -Wno-pointer-sign
    set warnings=!warnings_fwkc! !warnings_demos!

    if "!build!"=="rel" (
        set args=!warnings! /nologo     /MT /openmp /DNDEBUG !args!        /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
    )
    if "!build!"=="dev" (
        set args=!warnings! /nologo /Zi /MT /openmp /DEBUG   !args! && REM /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2
    )
    if "!build!"=="dbg" (
        set args=!warnings! /nologo /Zi /MT         /DEBUG   !args!        /Od  /fsanitize=address
    )

    set o=-o
    set echo=echo

) else if "!cc!"=="call tools\tcc" (

    if "!dll!"=="static" (
        set export=-c
        set import=fwk.o
    ) else (
        set export=-DAPI=EXPORT -shared
        set import=-DAPI=IMPORT fwk.def
    )

    if "!build!"=="rel" (
        set args=-O3 -DNDEBUG !args!
    )
    if "!build!"=="dev" (
        set args=-O2 -g !args!
    )
    if "!build!"=="dbg" (
        set args=-O0 -g !args!
    )

    set o=-o
    set echo=echo

) else ( rem if "!cc!"=="gcc" or "clang"

    set libs=-lws2_32 -lgdi32 -lwinmm -ldbghelp -lole32 -lshell32 -lcomdlg32

    if "!dll!"=="static" (
        set export=-c
        set import=fwk.o !libs! -Wl,--allow-multiple-definition
    ) else (
        set export=-DAPI=EXPORT -shared -o fwk.dll !libs! -Wl,--out-implib,fwk.a
        set import=-DAPI=IMPORT fwk.a
    )

    set args=-Wno-implicit-function-declaration !args!

    if "!build!"=="rel" (
        set args=-g -O3 -DNDEBUG !args!
    )
    if "!build!"=="dev" (
        set args=-g -O1 !args!
    )
    if "!build!"=="dbg" (
        set args=-g -O0 !args!
    )

    set o=-o
    set echo=echo
)

echo build=!build!, type=!dll!, cc=!cc!, other=!other!, args=!args!
echo import=!import!, export=!export!

rem framework
if "!fwk!"=="yes" (
if "!vis!"=="yes" echo !cc! tools\fwk.c !export! !args! ^&^& if "!dll!"=="dll" copy /y fwk.dll demos\lua ^> nul
!echo! fwk          && !cc! tools\fwk.c !export! !args!   && if "!dll!"=="dll" copy /y fwk.dll demos\lua  > nul
)

rem editor
if "!editor!"=="yes" (
!echo! editor       && !cc! !o! editor.exe       editor\editor.c      !import! !args!
)

rem demos
if "!demos!"=="yes" (
!echo! 00-demo      && !cc! !o! 00-demo.exe      demos\00-demo.c      !import! !args!
!echo! 00-hello     && !cc! !o! 00-hello.exe     demos\00-hello.c     !import! !args!
!echo! 00-cubemap   && !cc! !o! 00-cubemap.exe   demos\00-cubemap.c   !import! !args!
!echo! 00-collide   && !cc! !o! 00-collide.exe   demos\00-collide.c   !import! !args!
!echo! 00-anims     && !cc! !o! 00-anims.exe     demos\00-anims.c     !import! !args!
!echo! 00-scene     && !cc! !o! 00-scene.exe     demos\00-scene.c     !import! !args!
!echo! 00-shadertoy && !cc! !o! 00-shadertoy.exe demos\00-shadertoy.c !import! !args!
!echo! 00-sprite    && !cc! !o! 00-sprite.exe    demos\00-sprite.c    !import! !args!
!echo! 00-tilemap   && !cc! !o! 00-tilemap.exe   demos\00-tilemap.c   !import! !args!
!echo! 00-tiled     && !cc! !o! 00-tiled.exe     demos\00-tiled.c     !import! !args!
!echo! 00-video     && !cc! !o! 00-video.exe     demos\00-video.c     !import! !args!
!echo! 00-script    && !cc! !o! 00-script.exe    demos\00-script.c    !import! !args!
!echo! 00-socket    && !cc! !o! 00-socket.exe    demos\00-socket.c    !import! !args!
!echo! 00-easing    && !cc! !o! 00-easing.exe    demos\00-easing.c    !import! !args!
!echo! 00-font      && !cc! !o! 00-font.exe      demos\00-font.c      !import! !args!
!echo! 00-material  && !cc! !o! 00-material.exe  demos\00-material.c  !import! !args!
!echo! 00-pbr       && !cc! !o! 00-pbr.exe       demos\00-pbr.c       !import! !args!
!echo! 00-instanced && !cc! !o! 00-instanced.exe demos\00-instanced.c !import! !args!
!echo! 00-audio     && !cc! !o! 00-audio.exe     demos\00-audio.c     !import! !args!
!echo! 00-spine     && !cc! !o! 00-spine.exe     demos\00-spine.c     !import! !args!
)

rem user-defined apps
if not "!other!"=="" (
if "!vis!"=="yes" echo !cc! !other! !import! !args!
!echo! !other!      && !cc! !other! !import! !args!
)

rem PAUSE only if double-clicked from Windows explorer
(((echo.%cmdcmdline%)|%WINDIR%\system32\find.exe /I "%~0")>nul)&&pause

exit /b
