cl game.c -I..\..\..\ %* /link /SUBSYSTEM:WINDOWS /entry:mainCRTStartup

del *.obj
del *.exp
del *.lib
del *.pdb
