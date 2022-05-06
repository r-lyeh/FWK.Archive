#!/bin/bash 2>nul || goto :windows

sh ../../../MAKE.bat dll

exit
:windows

call ..\..\..\make.bat dll
