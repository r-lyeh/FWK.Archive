@if     "%1"=="-impdef" @C:\prj\fwk\\art\editor\tools\tcc-win\tcc %* 
@if not "%1"=="-impdef" @C:\prj\fwk\\art\editor\tools\tcc-win\tcc -I C:\prj\fwk\\art\editor\tools\tcc-win\include_mingw\winapi -I C:\prj\fwk\\art\editor\tools\tcc-win\include_mingw\ %* 
