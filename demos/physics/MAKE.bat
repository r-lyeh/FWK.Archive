cl /nologo -c ..\..\tools\fwk.c
cl /nologo -I ..\..\ demo_car.cpp  fwk.obj
cl /nologo -I ..\..\ demo_dom.cpp  fwk.obj
cl /nologo -I ..\..\ demo_pool.cpp fwk.obj
cl /nologo -I ..\..\ demo_rag.cpp  fwk.obj
del *.exp 
del *.lib 
del *.obj 
