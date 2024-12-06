#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

# copy demos to root folder. local changes are preserved
# cp -n art/demos/*.c .

# rem tests
# clang art/editor/tools/editor.c -I. -lm -lX11 -g -fsanitize=address,undefined && ./a.out
# cl art\editor\tools\editor.c -I. -fsanitize=address /DEBUG /Zi && editor

# tidy environment
if [ "$1" = "tidy" ]; then
    rm demo_* 2> /dev/null
    rm demo 2> /dev/null
    rm fwk.o 2> /dev/null
    rm .art*.zip 2> /dev/null
    rm art/demos/lua/.art*.zip 2> /dev/null
    rm art/demos/html5/.art*.zip 2> /dev/null
    rm art/demos/lua/libfwk* 2> /dev/null
    rm fwk_*.* 2> /dev/null
    rm 3rd_*.* 2> /dev/null
    rm libfwk* 2> /dev/null
    rm -rf *.dSYM 2> /dev/null
    rm *.png 2> /dev/null
    rm *.mp4 2> /dev/null
    rm editor 2> /dev/null
    exit
fi
# shortcuts for split & join amalgamation scripts
if [ "$1" = "split" ]; then
    sh art/editor/tools/split.bat
    exit
fi
if [ "$1" = "join" ]; then
    sh art/editor/tools/join.bat
    exit
fi
# cook
if [ "$1" = "cook" ]; then
    cc -o cook art/editor/tools/cook.c -I.
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
     sudo pacman -S --noconfirm tcc # gcc ffmpeg # -Syu
    # setup (Debian, Ubuntu, etc)
     sudo apt-get -y update
    #sudo apt-get -y install ffmpeg || (sudo apt-get install snapd && sudo snap install ffmpeg)
     sudo apt-get -y install tcc libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev        # absolute minimum
    #sudo apt-get -y install clang xorg-dev                                                               # memorable, around 100 mib
    #sudo apt-get -y install clang xorg-dev libglfw3-dev libassimp-dev gcc                                # initial revision

    # pipeline
    #cc art/editor/tools/ass2iqe.c   -o art/editor/tools/ass2iqe.linux  -lm -ldl -lpthread -w -g -lassimp
    #cc art/editor/tools/iqe2iqm.cpp -o art/editor/tools/iqe2iqm.linux  -lm -ldl -lpthread -w -g -lstdc++
    #cc art/editor/tools/mid2wav.c   -o art/editor/tools/mid2wav.linux  -lm -ldl -lpthread -w -g
    #cc art/editor/tools/xml2json.c  -o art/editor/tools/xml2json.linux -lm -ldl -lpthread -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x art/editor/tools/ass2iqe.linux
    chmod +x art/editor/tools/iqe2iqm.linux
    chmod +x art/editor/tools/mid2wav.linux
    chmod +x art/editor/tools/mod2wav.linux
    chmod +x art/editor/tools/xml2json.linux
    chmod +x art/editor/tools/sfxr2wav.linux
    chmod +x art/editor/tools/ffmpeg.linux
    chmod +x art/editor/tools/cuttlefish.linux
    chmod +x art/editor/tools/PVRTexToolCLI.linux
    chmod +x art/editor/tools/cook.linux
    chmod +x art/editor/tools/xlsx2ini.linux
    chmod +x art/demos/lua/luajit.linux

    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libfwk.so  && $cc -o libfwk.so fwk.c -shared -fPIC -w -g -lX11 $args
        cp libfwk.so art/demos/lua/
        export import="libfwk.so -Wl,-rpath,./"
    else
    # framework (static)
        echo fwk        && $cc -c fwk.c -w -g    $args
        export import=fwk.o
    fi

    # editor
    echo editor         && $cc -o editor         editor.c                   -lm -ldl -lpthread -lX11 -w -g     $args $import &

    # demos
    echo demo           && $cc -o demo           demo.c                     -lm -ldl -lpthread -lX11 -w -g     $args $import &
    echo demo_cubemap   && $cc -o demo_cubemap   art/demos/demo_cubemap.c   -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_collide   && $cc -o demo_collide   art/demos/demo_collide.c   -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_model     && $cc -o demo_model     art/demos/demo_model.c     -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_scene     && $cc -o demo_scene     art/demos/demo_scene.c     -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_shadertoy && $cc -o demo_shadertoy art/demos/demo_shadertoy.c -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_sprite    && $cc -o demo_sprite    art/demos/demo_sprite.c    -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_video     && $cc -o demo_video     art/demos/demo_video.c     -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_script    && $cc -o demo_script    art/demos/demo_script.c    -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_socket    && $cc -o demo_socket    art/demos/demo_socket.c    -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_easing    && $cc -o demo_easing    art/demos/demo_easing.c    -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_font      && $cc -o demo_font      art/demos/demo_font.c      -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_material  && $cc -o demo_material  art/demos/demo_material.c  -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_pbr       && $cc -o demo_pbr       art/demos/demo_pbr.c       -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_instanced && $cc -o demo_instanced art/demos/demo_instanced.c -lm -ldl -lpthread -lX11 -w -g -I. $args $import &
    echo demo_audio     && $cc -o demo_audio     art/demos/demo_audio.c     -lm -ldl -lpthread -lX11 -w -g -I. $args $import
fi

if [ "$(uname)" = "Darwin" ]; then
    # setup (osx)
    export SDKROOT=$(xcrun --show-sdk-path)
    # brew install glfw

    # pipeline
    #cc art/editor/tools/ass2iqe.c   -o art/editor/tools/ass2iqe.osx  -w -g -lassimp
    #cc art/editor/tools/iqe2iqm.cpp -o art/editor/tools/iqe2iqm.osx  -w -g -lstdc++
    #cc art/editor/tools/mid2wav.c   -o art/editor/tools/mid2wav.osx  -w -g
    #cc art/editor/tools/xml2json.c  -o art/editor/tools/xml2json.osx -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x art/editor/tools/ass2iqe.osx
    chmod +x art/editor/tools/iqe2iqm.osx
    chmod +x art/editor/tools/mid2wav.osx
    chmod +x art/editor/tools/mod2wav.osx
    chmod +x art/editor/tools/xml2json.osx
    chmod +x art/editor/tools/sfxr2wav.osx
    chmod +x art/editor/tools/ffmpeg.osx
    chmod +x art/editor/tools/cuttlefish.osx
    chmod +x art/editor/tools/PVRTexToolCLI.osx
    chmod +x art/editor/tools/cook.osx
    chmod +x art/editor/tools/xlsx2ini.osx
    chmod +x art/demos/lua/luajit.osx

    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libfwk     && cc -ObjC -dynamiclib -o libfwk.dylib fwk.c -framework cocoa -framework iokit -w -g $args
        cp libfwk.dylib art/demos/lua
        export import=libfwk.dylib
    else
    # framework
        echo fwk        && cc -c -ObjC fwk.c -w -g $args
        export import=fwk.o
    fi

    # editor
    echo editor         && cc -o editor -ObjC   editor.c                   -framework cocoa -framework iokit -w -g     $import $args &

    # demos
    echo demo           && cc -o demo           demo.c                     -framework cocoa -framework iokit -w -g     $import $args &
    echo demo_cubemap   && cc -o demo_cubemap   art/demos/demo_cubemap.c   -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_collide   && cc -o demo_collide   art/demos/demo_collide.c   -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_model     && cc -o demo_model     art/demos/demo_model.c     -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_scene     && cc -o demo_scene     art/demos/demo_scene.c     -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_shadertoy && cc -o demo_shadertoy art/demos/demo_shadertoy.c -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_sprite    && cc -o demo_sprite    art/demos/demo_sprite.c    -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_video     && cc -o demo_video     art/demos/demo_video.c     -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_script    && cc -o demo_script    art/demos/demo_script.c    -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_socket    && cc -o demo_socket    art/demos/demo_socket.c    -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_easing    && cc -o demo_easing    art/demos/demo_easing.c    -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_font      && cc -o demo_font      art/demos/demo_font.c      -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_material  && cc -o demo_material  art/demos/demo_material.c  -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_pbr       && cc -o demo_pbr       art/demos/demo_pbr.c       -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_instanced && cc -o demo_instanced art/demos/demo_instanced.c -framework cocoa -framework iokit -w -g -I. $import $args &
    echo demo_audio     && cc -o demo_audio     art/demos/demo_audio.c     -framework cocoa -framework iokit -w -g -I. $import $args
fi

exit


:: -----------------------------------------------------------------------------
:windows

@echo off
setlocal enableDelayedExpansion

rem show help
if "%1"=="help" (
    echo %0
    echo %0 [tidy]
    echo %0 [docs]
    echo %0 [cook]
    echo %0 [bindings]
    echo %0 [memcheck]
    echo %0 [split^|join]
    echo %0 [demo name "compiler args" "program args"]
    echo %0 [dll^|static] [dbg^|dev^|rel] [cl^|tcc^|cc^|gcc^|clang] [-- compiler args]
    exit /b
)

rem generate tcc.bat
cd /d "%~dp0"
echo @if     "%%1"=="-impdef" @%~dp0\art\editor\tools\tcc-win\tcc %%* > tcc.bat
echo @if not "%%1"=="-impdef" @%~dp0\art\editor\tools\tcc-win\tcc -I %~dp0\art\editor\tools\tcc-win\include_mingw\winapi -I %~dp0\art\editor\tools\tcc-win\include_mingw\ %%* >> tcc.bat

rem generate cooker twice: use multi-threaded version if available (cl). then cook.
if "%1"=="cook" (
    call art\editor\tools\tcc art\editor\tools\cook.c -I.
                           cl art\editor\tools\cook.c -I.
    cook

    exit /b
)
rem generate bindings
if "%1"=="bindings" (
    rem luajit
    art\editor\tools\luajit art\editor\tools\luajit_make_bindings.lua > fwk.lua
    move /y fwk.lua art\demos\lua

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
    call art\editor\tools\tcc art\docs\docs.c fwk.c -I. %2
    docs fwk.h --excluded=3rd_glad.h,fwk.h,fwk_compat.h, > fwk.html
    move /y fwk.html art\docs\docs.html

    exit /b
)
rem generate prior files to a github release
if "%1"=="github" (
    rem call make.bat dll
    call make.bat docs
    call make.bat bindings

    call make.bat split
    rd /q /s art\editor\tools\fwk
    md art\editor\tools\fwk
    md art\editor\tools\fwk\3rd
    move /y fwk_*.? art\editor\tools\fwk\
    move /y 3rd_*.? art\editor\tools\fwk\3rd\
    echo.> "art\editor\tools\fwk\;. for browsing purposes only. do not compile these"
    echo.> "art\editor\tools\fwk\3rd\;. for browsing purposes only. do not compile these"

    call make.bat tidy

    exit /b
)

rem build & execute single demo. usage: make demo name "compiler args" "program args"
if "%1"=="demo" (
    setlocal enableDelayedExpansion
    set "FILENAME=demo"
    if exist "demo_%2.c" (
        set "FILENAME=demo_%2"
    )
    copy /y !FILENAME!.c art\demos\!FILENAME!.c
    cl !FILENAME!.c /nologo /openmp /Zi fwk.c %~3%
    if %ERRORLEVEL%==0 (call !FILENAME!.exe %~4%) else (echo Compilation error: !FILENAME!)
    exit /b
)

rem shortcuts for split & join amalgamation scripts
if "%1"=="split" (
    call art\editor\tools\split
    exit /b
)
if "%1"=="join" (
    call art\editor\tools\join
    exit /b
)

rem check memory api calls
if "%1"=="checkmem" (
    findstr /RNC:"[^_xv]realloc[(]" fwk.c
    findstr /RNC:"[^_xv]malloc[(]"  fwk.c
    findstr /RNC:"[^_xv]free[(]"    fwk.c
    findstr /RNC:"[^_xv]calloc[(]"  fwk.c
    findstr /RNC:"[^_xv]strdup[(]"  fwk.c
    exit /b
)

rem copy demos to root folder. local changes are preserved
rem echo n | copy /-y art\demos\*.c 1> nul 2> nul

rem tidy environment
if "%1"=="tidy" (
    move /y demo_*.png art\demos
    move /y demo_*.c art\demos
    del art\demos\lua\fwk.dll
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
    del demo_*.*
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
rem cl art/editor/tools/ass2iqe.c   /Feart/editor/tools/ass2iqe.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL assimp.lib
rem cl art/editor/tools/iqe2iqm.cpp /Feart/editor/tools/iqe2iqm.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
rem cl art/editor/tools/mid2wav.c   /Feart/editor/tools/mid2wav.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
rem cl art/editor/tools/xml2json.c  /Feart/editor/tools/xml2json.exe /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
rem --- pipeline
rem gcc art/editor/tools/ass2iqe.c   -o art/editor/tools/ass2iqe.exe  -w -lassimp
rem gcc art/editor/tools/iqe2iqm.cpp -o art/editor/tools/iqe2iqm.exe  -w -lstdc++
rem gcc art/editor/tools/mid2wav.c   -o art/editor/tools/mid2wav.exe  -w
rem gcc art/editor/tools/xml2json.c  -o art/editor/tools/xml2json.exe -w
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
    cl fwk.c                          !export! !args!
    if "!dll!"=="dll"                 copy /y fwk.dll art\demos\lua > nul

    rem editor
    cl editor.c                       !import! !args!

    rem demos
    cl demo.c                         !import! !args!
    cl art\demos\demo_cubemap.c   -I. !import! !args!
    cl art\demos\demo_collide.c   -I. !import! !args!
    cl art\demos\demo_model.c     -I. !import! !args!
    cl art\demos\demo_scene.c     -I. !import! !args!
    cl art\demos\demo_shadertoy.c -I. !import! !args!
    cl art\demos\demo_sprite.c    -I. !import! !args!
    cl art\demos\demo_video.c     -I. !import! !args!
    cl art\demos\demo_script.c    -I. !import! !args!
    cl art\demos\demo_socket.c    -I. !import! !args!
    cl art\demos\demo_easing.c    -I. !import! !args!
    cl art\demos\demo_font.c      -I. !import! !args!
    cl art\demos\demo_material.c  -I. !import! !args!
    cl art\demos\demo_pbr.c       -I. !import! !args!
    cl art\demos\demo_instanced.c -I. !import! !args!
    cl art\demos\demo_audio.c     -I. !import! !args!

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
    echo fwk            && call tcc fwk.c                          !export! !args!
    if "!dll!"=="dll"   copy /y fwk.dll art\demos\lua > nul

    rem editor
    echo editor         && call tcc editor.c                       !import! !args!

    rem demos
    echo demo           && call tcc demo.c                         !import! !args!
    echo demo_cubemap   && call tcc art\demos\demo_cubemap.c   -I. !import! !args!
    echo demo_collide   && call tcc art\demos\demo_collide.c   -I. !import! !args!
    echo demo_model     && call tcc art\demos\demo_model.c     -I. !import! !args!
    echo demo_scene     && call tcc art\demos\demo_scene.c     -I. !import! !args!
    echo demo_shadertoy && call tcc art\demos\demo_shadertoy.c -I. !import! !args!
    echo demo_sprite    && call tcc art\demos\demo_sprite.c    -I. !import! !args!
    echo demo_video     && call tcc art\demos\demo_video.c     -I. !import! !args!
    echo demo_script    && call tcc art\demos\demo_script.c    -I. !import! !args!
    echo demo_socket    && call tcc art\demos\demo_socket.c    -I. !import! !args!
    echo demo_easing    && call tcc art\demos\demo_easing.c    -I. !import! !args!
    echo demo_font      && call tcc art\demos\demo_font.c      -I. !import! !args!
    echo demo_material  && call tcc art\demos\demo_material.c  -I. !import! !args!
    echo demo_pbr       && call tcc art\demos\demo_pbr.c       -I. !import! !args!
    echo demo_instanced && call tcc art\demos\demo_instanced.c -I. !import! !args!
    echo demo_audio     && call tcc art\demos\demo_audio.c     -I. !import! !args!

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
    echo fwk            && !cc! fwk.c -w !export! !args!
    if "!dll!"=="dll"   copy /y fwk.dll art\demos\lua > nul

    rem editor
    echo editor         && !cc! -o editor         editor.c                       !import! !args!

    rem demos
    echo demo           && !cc! -o demo           demo.c                         !import! !args!
    echo demo_cubemap   && !cc! -o demo_cubemap   art\demos\demo_cubemap.c   -I. !import! !args!
    echo demo_collide   && !cc! -o demo_collide   art\demos\demo_collide.c   -I. !import! !args!
    echo demo_model     && !cc! -o demo_model     art\demos\demo_model.c     -I. !import! !args!
    echo demo_scene     && !cc! -o demo_scene     art\demos\demo_scene.c     -I. !import! !args!
    echo demo_shadertoy && !cc! -o demo_shadertoy art\demos\demo_shadertoy.c -I. !import! !args!
    echo demo_sprite    && !cc! -o demo_sprite    art\demos\demo_sprite.c    -I. !import! !args!
    echo demo_video     && !cc! -o demo_video     art\demos\demo_video.c     -I. !import! !args!
    echo demo_script    && !cc! -o demo_script    art\demos\demo_script.c    -I. !import! !args!
    echo demo_socket    && !cc! -o demo_socket    art\demos\demo_socket.c    -I. !import! !args!
    echo demo_easing    && !cc! -o demo_easing    art\demos\demo_easing.c    -I. !import! !args!
    echo demo_font      && !cc! -o demo_font      art\demos\demo_font.c      -I. !import! !args!
    echo demo_material  && !cc! -o demo_material  art\demos\demo_material.c  -I. !import! !args!
    echo demo_pbr       && !cc! -o demo_pbr       art\demos\demo_pbr.c       -I. !import! !args!
    echo demo_instanced && !cc! -o demo_instanced art\demos\demo_instanced.c -I. !import! !args!
    echo demo_audio     && !cc! -o demo_audio     art\demos\demo_audio.c     -I. !import! !args!
)

rem PAUSE only if double-clicked from Windows explorer
rem (((echo.%cmdcmdline%)|%WINDIR%\system32\find.exe /I "%~0")>nul)&&pause

exit /b
