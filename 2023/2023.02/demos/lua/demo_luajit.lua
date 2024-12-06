-- this will run on vanilla luajit.exe, provided that fwk.dll and this file are both present in same folder

local fwk=require('fwk')

-- specify location of cookbook
fwk.cook_config("../../tools/cook.ini");

-- create window
fwk.window_create(75.0,0)

-- game loop
while fwk.window_swap() == 1 do
    fwk.ddraw_grid(0)
    fwk.window_title("hello luajit")
    if fwk.ui_panel("luajit", 0) == 1 then
       fwk.ui_panel_end()
    end
end
