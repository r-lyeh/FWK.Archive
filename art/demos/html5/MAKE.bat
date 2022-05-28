#!/bin/bash 2>nul || goto :windows

if [ "$1" = "" ]; then
	sh MAKE.bat demo_collide.c
	exit
fi

## clone emscripten sdk
git clone https://github.com/emscripten-core/emsdk ../../../../emsdk
pushd ../../../../emsdk
	./emsdk install  3.0.0 ## latest
	./emsdk activate 3.0.0 ## latest
	source ./emsdk_env.sh
popd

## cook art
if [ "$(uname)" = "Darwin" ]; then
    chmod +x ../../editor/tools/cook.osx
    ../../editor/tools/cook.osx
else
    chmod +x ../../editor/tools/cook.linux
    ../../editor/tools/cook.linux
fi

## host webserver, compile and launch
python -m http.server --bind 127.0.0.1 8000 1> /dev/null 2> /dev/null &
emcc $@ -g ../../../fwk.c -I../../.. -o index.html -s FULL_ES3 -s USE_GLFW=3 -s SINGLE_FILE=1 -s PRECISE_F32=1 -s TOTAL_MEMORY=256mb -s ENVIRONMENT=worker,web --shell-file template.html -Wfatal-errors --preload-file .art[00].zip -s ALLOW_MEMORY_GROWTH=1 -lidbfs.js && xdg-open http://localhost:8000/index.html

exit

:windows

if "%1"=="" MAKE.bat demo_collide.c

rem clone emscripten sdk
if not exist "..\..\..\..\emsdk" (
	git clone https://github.com/emscripten-core/emsdk ..\..\..\..\emsdk
	pushd ..\..\..\..\emsdk
		call emsdk install  3.0.0 && rem latest
		call emsdk activate 3.0.0 && rem latest
	popd
)
if "%EMSDK%"=="" call ..\..\..\..\emsdk\emsdk_env.bat

rem cook art
..\..\editor\tools\cook.exe

rem host webserver, compile and launch
start python -m http.server --bind 127.0.0.1 8000
emcc %* -g ..\..\..\fwk.c -I..\..\.. -o index.html -s FULL_ES3 -s USE_GLFW=3 -s SINGLE_FILE=1 -s PRECISE_F32=1 -s TOTAL_MEMORY=256mb -s ENVIRONMENT=worker,web --shell-file template.html -Wfatal-errors --preload-file .art[00].zip -s ALLOW_MEMORY_GROWTH=1 -lidbfs.js && start http://localhost:8000/index.html
