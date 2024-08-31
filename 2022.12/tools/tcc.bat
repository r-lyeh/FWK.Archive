@if     "%1"=="-impdef" @%~dp0\tcc-win\tcc %*
@if not "%1"=="-impdef" @%~dp0\tcc-win\tcc -I %~dp0\tcc-win\include_mingw\winapi -I %~dp0\tcc-win\include_mingw\ %*
