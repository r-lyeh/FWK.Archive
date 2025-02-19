@if "%1" == "tidy" (
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
    del *.def
    del *.dll
    del oscedit.ini
    rd /q /s .vs
	exit /b
)

pushd .. && call make amalgamation && popd

taskkill /im "oscedit.exe" > nul 2> nul
call ..\tools\tcc oscgame.c -I .. -DFWK_C %*
call ..\tools\tcc oscsend.c -I .. -DFWK_C %*
call ..\tools\tcc oscedit.c -I .. -DFWK_C %* && start oscedit.exe

:: showcase UI widgets creation on demand, which are requested from this very same batch file
exit /b
oscsend /player/time 5.5
oscsend /player/name "john doe"
oscsend /player/health 100
oscsend /player/is_active true

oscsend /player2/integer 123
oscsend /player2/string world

oscsend /player1/string hello

:: simulate a game running sending OSC commands to our editor
rem oscgame
