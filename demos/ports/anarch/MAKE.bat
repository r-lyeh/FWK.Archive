cl game.c -I..\..\..\engine\joint %* /link /SUBSYSTEM:WINDOWS /entry:mainCRTStartup

del *.obj
del *.exp
del *.lib
del *.pdb
