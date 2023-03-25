cl game.c PureDOOM.c -I ..\..\..\engine ..\..\..\engine\fwk.c ^
    -DWIN32 ^
    %* ^
    /link /SUBSYSTEM:WINDOWS /entry:mainCRTStartup

del *.obj
del *.exp
del *.lib
del *.pdb
