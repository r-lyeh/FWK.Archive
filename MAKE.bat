#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

cp demos/*.c -n

if [ "$(uname)" != "Darwin" ]; then
    # setup (linux)
     sudo apt-get update
     sudo apt-get install g++ ffmpeg libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev  # absolute minimum
    #sudo apt-get install g++ ffmpeg xorg-dev                                                           # memorable, around 100 mib
    #sudo apt-get install g++ ffmpeg xorg-dev libglfw3-dev libassimp-dev clang                          # initial revision

    # pipeline
    #cc art/tools/ass2iqe.c   -o art/tools/ass2iqe  -lm -ldl -lpthread -w -g -lassimp
    #cc art/tools/iqe2iqm.cpp -o art/tools/iqe2iqm  -lm -ldl -lpthread -w -g -lstdc++
    #cc art/tools/mid2wav.c   -o art/tools/mid2wav  -lm -ldl -lpthread -w -g
    #cc art/tools/xml2json.c  -o art/tools/xml2json -lm -ldl -lpthread -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x art/tools/ass2iqe
    chmod +x art/tools/iqe2iqm
    chmod +x art/tools/mid2wav
    chmod +x art/tools/xml2json
    chmod +x art/tools/ffmpeg

    # framework
    echo fwk            && cc -c fwk.c -w -g

    # demos
    echo demo           && cc -o demo           demo.c           fwk.o -lm -ldl -lpthread -w -g &
    echo demo_cubemap   && cc -o demo_cubemap   demo_cubemap.c   fwk.o -lm -ldl -lpthread -w -g &
    echo demo_collide   && cc -o demo_collide   demo_collide.c   fwk.o -lm -ldl -lpthread -w -g &
    echo demo_model     && cc -o demo_model     demo_model.c     fwk.o -lm -ldl -lpthread -w -g &
    echo demo_scene     && cc -o demo_scene     demo_scene.c     fwk.o -lm -ldl -lpthread -w -g &
    echo demo_shadertoy && cc -o demo_shadertoy demo_shadertoy.c fwk.o -lm -ldl -lpthread -w -g &
    echo demo_sprite    && cc -o demo_sprite    demo_sprite.c    fwk.o -lm -ldl -lpthread -w -g &
    echo demo_video     && cc -o demo_video     demo_video.c     fwk.o -lm -ldl -lpthread -w -g &
    echo demo_script    && cc -o demo_script    demo_script.c    fwk.o -lm -ldl -lpthread -w -g &
    echo demo_socket    && cc -o demo_socket    demo_socket.c    fwk.o -lm -ldl -lpthread -w -g &
fi

if [ "$(uname)" = "Darwin" ]; then
    # setup (osx)
    export SDKROOT=$(xcrun --show-sdk-path)

    # pipeline
    #cc art/tools/ass2iqe.c   -o art/tools/ass2iqe.osx  -w -g -lassimp
    #cc art/tools/iqe2iqm.cpp -o art/tools/iqe2iqm.osx  -w -g -lstdc++
    #cc art/tools/mid2wav.c   -o art/tools/mid2wav.osx  -w -g
    #cc art/tools/xml2json.c  -o art/tools/xml2json.osx -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x art/tools/ass2iqe.osx
    chmod +x art/tools/iqe2iqm.osx
    chmod +x art/tools/mid2wav.osx
    chmod +x art/tools/xml2json.osx
    chmod +x art/tools/ffmpeg.osx

    # framework
    echo fwk            && cc -c -ObjC fwk.c -w -g

    # demos
    echo demo           && cc -o demo           demo.c           fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_cubemap   && cc -o demo_cubemap   demo_cubemap.c   fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_collide   && cc -o demo_collide   demo_collide.c   fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_model     && cc -o demo_model     demo_model.c     fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_scene     && cc -o demo_scene     demo_scene.c     fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_shadertoy && cc -o demo_shadertoy demo_shadertoy.c fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_sprite    && cc -o demo_sprite    demo_sprite.c    fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_video     && cc -o demo_video     demo_video.c     fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_script    && cc -o demo_script    demo_script.c    fwk.o -framework cocoa -framework iokit -w -g &
    echo demo_socket    && cc -o demo_socket    demo_socket.c    fwk.o -framework cocoa -framework iokit -w -g &
fi

exit



:windows -----------------------------------------------------------------------
@echo off

rem setup
if "%Platform%"=="" (
    echo Warning: Trying VS 2013/2015/2017/2019 x64 ...
    set Platform=x64
           if exist "%VS190COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS190COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else if exist "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else if exist "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else if exist "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
              @call "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat"
    ) else if exist "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
              @call "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat"
    ) else if exist "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat"
    ) else (
        echo Warning: Trying Mingw64 ...
        set Platform=mingw64
        where /q gcc.exe || ( set Platform=tcc&&echo Warning: Trying TCC ... )
    )
)

cd "%~dp0"
echo @%~dp0\art\tools\tcc-win\tcc -I %~dp0\art\tools\tcc-win\include_mingw\winapi -I %~dp0\art\tools\tcc-win\include_mingw\ %%* > tcc.bat

rem generate documentation
if "%1"=="docs" (
    if not exist "fwk_window.h" call art\tools\split.bat

    rem create fwk.html: css + intro
    type art\tools\docs.css  > fwk.html
    type art\tools\docs.md  >> fwk.html

    echo # API >> fwk.html

    rem append parsed fwk headers
    setlocal enabledelayedexpansion
    for %%i in (fwk_*.h) do (
        set file=%%i
        set header=!file:fwk_=!
        set section=!header:.h=!
        if "!section!"=="compat" (
        rem skipped
        ) else if "!section!"=="main" (
        rem skipped
        ) else (
        rem extract markdown from headers
        echo ## !section! >> fwk.html
        art\tools\src2doc.exe %%i >> fwk.html
        rem functions
        echo ```C linenumbers >> fwk.html
        type %%i | find "API " >> fwk.html
        echo ```>> fwk.html
        )
    )
    exit /b
)

rem copy demos to root folder. local changes are preserved
echo n | copy /-y demos\*.c 1> nul 2> nul

rem tidy environment
if "%1"=="tidy" (
    del .temp*.*
    del *.zip
    del *.mem
    del *.exp
    del *.lib
    del *.exe
    del *.obj
    del *.o
    del *.pdb
    del *.ilk
    del *.png
    del *.def
    del *.dll
    del demo_*.c
    rd /q /s .vs
    exit /b
)

if "%Platform%"=="x64" (
    rem pipeline
    rem cl art/tools/ass2iqe.c   /Feart/tools/ass2iqe.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL assimp.lib
    rem cl art/tools/iqe2iqm.cpp /Feart/tools/iqe2iqm.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
    rem cl art/tools/mid2wav.c   /Feart/tools/mid2wav.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL
    rem cl art/tools/xml2json.c  /Feart/tools/xml2json.exe /nologo /openmp /O2 /Oy /MT /DNDEBUG /DFINAL

    rem framework
    cl fwk.c /nologo /openmp /Zi /c

    rem demos
    cl demo.c           fwk.obj /nologo /openmp /Zi
    cl demo_cubemap.c   fwk.obj /nologo /openmp /Zi
    cl demo_collide.c   fwk.obj /nologo /openmp /Zi
    cl demo_model.c     fwk.obj /nologo /openmp /Zi
    cl demo_scene.c     fwk.obj /nologo /openmp /Zi
    cl demo_shadertoy.c fwk.obj /nologo /openmp /Zi
    cl demo_sprite.c    fwk.obj /nologo /openmp /Zi
    cl demo_video.c     fwk.obj /nologo /openmp /Zi
    cl demo_script.c    fwk.obj /nologo /openmp /Zi
    cl demo_socket.c    fwk.obj /nologo /openmp /Zi

    rem fwk.dll demos
    cl fwk.c /LD /DAPI=EXPORT          /nologo /openmp /Zi 
    cl demo_dll.c fwk.lib /DAPI=IMPORT /nologo /openmp /Zi

) else if "%Platform%"=="mingw64" (
    rem pipeline
    rem gcc art/tools/ass2iqe.c   -o art/tools/ass2iqe.exe  -w -lassimp
    rem gcc art/tools/iqe2iqm.cpp -o art/tools/iqe2iqm.exe  -w -lstdc++
    rem gcc art/tools/mid2wav.c   -o art/tools/mid2wav.exe  -w
    rem gcc art/tools/xml2json.c  -o art/tools/xml2json.exe -w

    rem framework
    echo fwk            && gcc -c fwk.c -std=c99 -w -g

    rem demos
    echo demo           && gcc -o demo           demo.c           fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_cubemap   && gcc -o demo_cubemap   demo_cubemap.c   fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_collide   && gcc -o demo_collide   demo_collide.c   fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_model     && gcc -o demo_model     demo_model.c     fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_scene     && gcc -o demo_scene     demo_scene.c     fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_shadertoy && gcc -o demo_shadertoy demo_shadertoy.c fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_sprite    && gcc -o demo_sprite    demo_sprite.c    fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_video     && gcc -o demo_video     demo_video.c     fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_script    && gcc -o demo_script    demo_script.c    fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
    echo demo_socket    && gcc -o demo_socket    demo_socket.c    fwk.o -lws2_32 -lgdi32 -lwinmm -ldbghelp -std=c99 -w -g
) else (
    rem pipeline
    rem gcc art/tools/ass2iqe.c   -o art/tools/ass2iqe.exe  -w -lassimp
    rem gcc art/tools/iqe2iqm.cpp -o art/tools/iqe2iqm.exe  -w -lstdc++
    rem gcc art/tools/mid2wav.c   -o art/tools/mid2wav.exe  -w
    rem gcc art/tools/xml2json.c  -o art/tools/xml2json.exe -w

    rem framework
    echo fwk            && tcc -c fwk.c -w

    rem demos
    echo demo           && tcc demo.c           fwk.o -w
    echo demo_cubemap   && tcc demo_cubemap.c   fwk.o -w
    echo demo_collide   && tcc demo_collide.c   fwk.o -w
    echo demo_model     && tcc demo_model.c     fwk.o -w
    echo demo_scene     && tcc demo_scene.c     fwk.o -w
    echo demo_shadertoy && tcc demo_shadertoy.c fwk.o -w
    echo demo_sprite    && tcc demo_sprite.c    fwk.o -w
    echo demo_video     && tcc demo_video.c     fwk.o -w
    echo demo_script    && tcc demo_script.c    fwk.o -w
    echo demo_socket    && tcc demo_socket.c    fwk.o -w
)

pause
exit /b
