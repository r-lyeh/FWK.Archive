<h1 align="center"><a href="https://bit.ly/fwk2023">F·W·K</a></h1>
<p align="center">
3D game framework in C, with Luajit and Python bindings now.<br/>
Archived repo. Development moved to https://github.com/r-lyeh/FWK
</p>

<p align="center">
<img src="https://i.imgur.com/sInbRoA.gif"/><br/>
</p>

## Gallery
<p align="center">
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/00-hello.png"     width="204px" title="Hello"/>     
<!--img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/00-demo.png"      width="204px" title="Demo"/-->    
<!--img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-audio.png"     width="204px" title="Audio"/-->     
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-cubemap.png"   width="204px" title="Cubemap"/>       
<!--img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-easing.png"    width="204px" title="Easing"/-->      
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-font.png"      width="204px" title="Font"/>    
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-instanced.png" width="204px" title="Instanced"/>         
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-material.png"  width="204px" title="Material"/>        
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-script.png"    width="204px" title="Script"/>      
<!--img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-spine.png"     width="204px" title="Spine"/-->     
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-sprite.png"    width="204px" title="Sprite"/>      
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/99-video.png"     width="204px" title="Video"/>     
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/04-actor.png"     width="204px" title="Actor"/>     

<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/00-ui.png"        width="204px" title="Hello UI: config, window, system, ui, video"/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/01-sprite.png"    width="204px" title="Sprites: window, audio, camera, font, tiled, render, fx, spritesheet, input, ui."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/02-ddraw.png"     width="204px" title="DebugDraw: camera, renderdd, collide, math, ui, boids"/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/03-anims.png"     width="204px" title="Anims."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/05-scene.png"     width="204px" title="Scene."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/06-pbr.png"       width="204px" title="PBR."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/demos/07-network.png"   width="204px" title="Network."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/tools/editor/editor.png" width="204px" title="Editor."/>
<br/>
</p>
<!--
editor1 | editor2
:-: | :-:
<video src='https://user-images.githubusercontent.com/35402248/174457347-f787a6a2-aac8-404c-a5da-f44310c3d432.mp4' width=180></video> | <video src='https://user-images.githubusercontent.com/35402248/174457144-40098726-e78f-4284-be68-783ff12f350f.mp4' width=180></video>
-->

## Goals
- [x] ~~C++~~. C.
- [x] ~~Fast~~. Naive.
- [x] ~~Modern~~. Simple.
- [x] ~~Full featured~~. Small.
- [x] ~~Rich build system~~. Single file.
- [x] ~~Royaltie fee~~. Free and unlicensed.

## Features ᕦ(ᐛ)ᕤ
- [x] Pipeline: configurable and integrated [asset pipeline](tools/cook.ini).
- [x] Embedded: [single-file header](engine/joint/fwk.h), all dependencies included.
- [x] Projects: Visual Studio, XCode, Ninja, gmake.
- [x] Compiler: MSVC, MINGW64, TCC, GCC, clang, clang-cl and emscripten.
- [x] Linkage: Both static linkage and dynamic .dll/.so/.dylib support. 
- [x] Platform: Windows, Linux and OSX. Partial HTML5/Web support.
- [x] DS: hash, sort, array/vector, map, set.
- [x] Math: rand, noise, ease, vec2/3/4, mat33/34/44, quat.
- [x] Geometry: ray, line, plane, aabb, sphere, capsule, triangle, poly and frustum.
- [x] Window: windowed, soft/hard fullscreen, msaa, icon, cursor handling.
- [x] Input: keyboard, mouse and gamepads.
- [x] Script: Lua scripting, Luajit bindings.
- [x] Network: downloads (HTTPS) and sockets (TCP/UDP). <!-- [*] Object, GameObject, W/ECS -->
- [x] AI: Swarm/Boids.
- [x] UI: button, list, slider, toggle, checkbox, editbox, dialog, color, image, menu, window, notify...
- [x] Font: TTF, OTF and TTC. Basic syntax highlighter. Glyph ranges. Atlasing.
- [x] Localization/I18N: XLSX and INI. Unicode.
- [x] Image: JPG, PNG, BMP, PSD, PIC, PNM, ICO.
- [x] Texture: KTX/2, PVR, DDS, ASTC, BASIS, HDR, TGA.
- [x] Texel: Depth, R, RG, RGB, RGBA, BC1/2/3/4/5/6/7, PVRI/II, ETC1/2, ASTC.
- [x] Audio: WAV/FLAC, OGG/MP1/MP3, FUR, MOD/XM/S3M/IT, SFXR and MID+SF2/SF3.
- [x] Video: MP4, MPG, OGV, MKV, WMV and AVI. Also, MP4 recording with MPEG-1 fallback.
- [x] Model: IQM/E, GLTF/2, GLB, FBX, OBJ, DAE, BLEND, MD3/5, MS3D, SMD, X, 3DS, BVH, DXF, LWO.
- [x] Render: PBR (metallic-roughness) workflow. <!-- @todo: merge demo_pbr.c rendering code into fwk_render.c -->
- [x] Render: Cubemaps, panoramas and spherical harmonics. Rayleigh/Mie scattering.
- [x] Render: Post-effects (SSAO,FXAA1/3,CRT,Contrast,Grain,Outline,Vignette...).
- [x] Render: 3D Anims, skeletal anims, hardware skinning and instanced rendering.
- [x] Render: 3D Debugdraw, batching and vectorial font.
- [x] Render: 2D Sprites, spritesheets, AA zooming and batching.
- [x] Render: 2D Tilemaps and tilesets: TMX, TSX.
- [x] Compression: DEFLATE, LZMA, LZ4, ULZ, BALZ, BCM, CRUSH, LZW3, LZSS and PPP.
- [x] Virtual filesystem: ZIP, PAK, TAR and DIR.
- [x] Level data: JSON, JSON5, SJSON, XML, INI.
- [x] Disk cache.
- [x] Scene handling.
- [x] Profiler, stats and leaks finder.
- [x] [Editor (wip)](https://user-images.githubusercontent.com/35402248/174457347-f787a6a2-aac8-404c-a5da-f44310c3d432.mp4).
- [x] [Documentation (wip)](https://bit.ly/fwk2023).

## Hello FWK
```C
#include "fwk.h" // Minimal C sample
int main() {
    window_create(75.0, 0); // 75% size, no extra flags
    while( window_swap() && !input(KEY_ESC) ) { // game loop
        puts("hello FWK from C!");
    }
}
```

```C
#include "fwk.h" // Minimal HTML5 sample
void render(void *arg) {
    if( !input(KEY_ESC) ) puts("hello FWK from HTML5!");
}
int main() {
    window_create(75.0, 0); // 75% size, no extra flags
    window_loop(render, NULL); // game loop
}
```

```lua
local fwk = require("fwk") -- Minimal Lua sample
fwk.window_create(75.0,0) -- 75% size, no extra flags
while fwk.window_swap() and fwk.input(fwk.KEY_ESC) == 0 do -- game loop
    print("hello FWK from Lua!")
end
```

## Quickstart
- Double-click `MAKE.bat` (Win) or `sh MAKE.bat` (Linux/OSX) to compile everything.
- `MAKE.bat sln` (Win) or `sh MAKE.bat sln` (Linux/OSX) to generate solutions/makefiles.
- `MAKE.bat help` (Win) or `sh MAKE.bat help` (Linux/OSX) for a bunch of options.
- `MAKE.bat hello.c` (Win) or `sh MAKE.bat hello.c` (Linux/OSX) to build a single executable.
- Alternatively,
```bat
echo win/vc       && cl hello.c
echo win/clang-cl && clang-cl hello.c
echo win/tcc      && tcc   hello.c -m64
echo win/mingw    && gcc   hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
echo win/clang    && clang hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
echo linux        && cc  hello.c -lm -ldl -lpthread -lX11
echo linux/tcc    && tcc hello.c -lm -ldl -lpthread -lX11 -D__STDC_NO_VLA__
echo osx          && cc -ObjC hello.c -framework cocoa -framework iokit -framework audiotoolbox
```

## Cook
- Most asset types need to be cooked before being used in your application. Some other assets like `.png` do not.
- Cooked assets will be written into .zipfiles close to your executable, and mounted before entering game loop.
- Cooked .zipfiles and your executable are the only required assets when releasing your game.
- Cook manually your assets by invoking supplied [`tools/cook` standalone binary](tools/).
- Cook automatically your assets by just playing your game: a runtime cook is already embedded into your binary.
  - In order to achieve this, ensure the [`tools/` folder](tools/) is close to your executable.
  - This folder contains all the related binaries to perform any asset conversion plus the [cookbook](tools/cook.ini) to do so.

## Extra tips
- Any ico/png file named after the executable name will be automatically used as app icon.
- Similar to the ico/png case above, the cooked .zipfiles can be named after the main executable as well.
- Dropped files into game window will be imported & saved into [`import/`](engine/art/import) folder.
- Update the gamepad controller database by upgrading the [`gamecontrollerdb.txt`](engine/art/input) file.
- Depending on your IDE, you might need to browse to [`engine/split/`](engine/split/) sources when debugging FWK.
- Cook assets on demand, as opposed to cook all existing assets on depot, by using `--cook-on-demand` flag.
- Linux/OSX users can optionally install wine and use the Windows tools instead (by using `--cook-wine` flag).
- Disable automatic cooking by using `--cook-jobs=0` flag (not recommended).
- Generate a project solution by dropping `engine/fwk.h, fwk.c and fwk` files into it, or `MAKE.bat sln`.
- Much faster builds by typing `MAKE.bat tcc` (Win/Linux). Beware: compiler has no thread-locals support.
<!-- - On windows + vc, you can use `make bindings` or `make docs` to generate everything prior to a release -->
<!-- - Note: Windows: Assimp.dll may need [this package installed](https://www.microsoft.com/en-us/download/confirmation.aspx?id=30679).-->

## Bindings
- Luajit: Luajit bindings are provided in the [auto-generated fwk.lua file](demos/lua/fwk.lua).
- Python: Python bindings are provided in the [auto-generated fwk.py file](demos/lua/fwk.py).
- Nelua: [Nelua bindings](https://github.com/Rabios/nelua-fun/tree/main/fwk) provided by Rabia Alhaffar.

## Credits (Artwork + demos)
- [DavidLam](https://en.wikipedia.org/wiki/Tokamak_(software)), for tokamak physics engine (ZLIB). 
- [Dean Evans + Raijin](https://youtu.be/RRvYkrrpMKo?t=147), for the Map song (c).
- [FMS_Cat](https://gist.github.com/FMS-Cat/a1ccea3ce866c34706084e3526204f4f), for nicest VHS/VCR shader around (MIT).
- [Goblin165cm](https://sketchfab.com/3d-models/halloween-little-witch-ccc023590bfb4789af9322864e42d1ab), for witch 3D model (CC BY 4.0).
- [ID Software](https://www.idsoftware.com) and [David St-Louis](https://github.com/Daivuk/PureDOOM), for PureDOOM (Doom License).
- [Miloslav Číž](https://codeberg.org/drummyfish/Anarch), for Anarch (CC0).
- [Nanofactory](https://sketchfab.com/3d-models/kgirls01-d2f946f58a8040ae993cda70c97b302c), for kgirls01 3D model (CC BY-NC-ND 4.0).
- [Quaternius](https://www.patreon.com/quaternius), for the lovely 3D robots (CC0).
- [RottingPixels](https://opengameart.org/content/2d-castle-platformer-tileset-16x16), for castle-tileset (CC0).
- [Rxi](https://github.com/rxi/autobatch), for lovely sprites & cats demo (MIT).
- [Tom Lewandowski](https://QuestStudios.com), for his MIDI recordings (c).
- [wwwtyro](https://github.com/wwwtyro/glsl-atmosphere), for nicest rayleigh/mie scattering shader around (CC0).

## Credits (Tools)
- [Aaron Barany](https://github.com/akb825/Cuttlefish), for cuttlefish (APACHE2).
- [Arseny Kapoulkine](https://github.com/zeux/pugixml/), for pugixml (MIT).
- [Assimp authors](https://github.com/assimp/assimp), for assimp (BSD3).
- [Bernhard Schelling](https://github.com/schellingb/TinySoundFont), for tml.h (Zlib) and tsf.h (MIT).
- [Christian Collins](http://www.schristiancollins.com), for GeneralUser GS soundfont (PD).
- [ffmpeg authors](https://www.ffmpeg.org/), for ffmpeg (LGPL21).
- [Imagination](https://developer.imaginationtech.com/pvrtextool/), for pvrtextoolcli (ITL).
- [Krzysztof Gabis](https://github.com/kgabis/ape), for split.py/join.py (MIT).
- [Lee Salzman](https://github.com/lsalzman/iqm/tree/5882b8c32fa622eba3861a621bb715d693573420/demo), for iqm.cpp (PD).
- [Martín Lucas Golini](https://github.com/SpartanJ/eepp/commit/8552941da19380d7a629c4da80a976aec5d39e5c), for emscripten-fs.html (CC0).
- [Mattias Gustavsson](https://github.com/mattiasgustavsson/libs), for mid.h (PD).
- [Michael Schmoock](http://github.com/willsteel/lcpp), for lcpp (MIT).
- [Morgan McGuire](https://casual-effects.com/markdeep/), for markdeep (BSD2).
- [Olivier Lapicque, Konstanty Bialkowski](https://github.com/Konstanty/libmodplug), for libmodplug (PD).
- [Polyglot Team](https://docs.google.com/spreadsheets/d/17f0dQawb-s_Fd7DHgmVvJoEGDMH_yoSd8EYigrb0zmM/edit), for polyglot gamedev (CC0).
- [Tildearrow](https://github.com/tildearrow/furnace/), for Furnace (GPL2).
- [Tomas Pettersson](http://www.drpetter.se/), for sfxr (PD).
- [Tor Andersson](https://github.com/ccxvii/asstools), for assiqe.c (BSD).

## Credits (Runtime)
- [Barerose](https://github.com/barerose), for swrap (CC0).
- [Camilla Löwy](https://github.com/elmindreda), for glfw3 (Zlib).
- [Dave Rand](https://tools.ietf.org/html/rfc1978) for ppp (PD).
- [David Herberth](https://github.com/dav1dde/), for glad generated code (PD).
- [David Reid](https://github.com/mackron), for miniaudio (PD).
- [Dominic Szablewski](https://github.com/phoboslab/pl_mpeg), for pl_mpeg (MIT).
- [Dominik Madarász](https://github.com/zaklaus), for json5 parser (PD).
- [Eduard Suica](https://github.com/eduardsui/tlse), for tlse (PD).
- [Gargaj+cce/Peisik](https://github.com/gargaj/foxotron), for Foxotron/PBR shaders (UNLICENSE).
- [Guillaume Vareille](http://tinyfiledialogs.sourceforge.net), for tinyfiledialogs (ZLIB).
- [Haruhiko Okumura](https://oku.edu.mie-u.ac.jp/~okumura/compression/) for lzss (PD).
- [Igor Pavlov](https://www.7-zip.org/) for LZMA (PD).
- [Ilya Muravyov](https://github.com/encode84) for bcm, balz, crush, ulz, lz4x (PD).
- [Jon Olick](https://www.jonolick.com/), for jo_mp1 and jo_mpeg (PD).
- [Joonas Pihlajamaa](https://github.com/jokkebk/JUnzip), for JUnzip library (PD).
- [Juliette Focault](https://github.com/juliettef/IconFontCppHeaders/blob/main/IconsMaterialDesign.h), for the generated MD header (ZLIB).
- [Lee Salzman](https://github.com/lsalzman/iqm/tree/5882b8c32fa622eba3861a621bb715d693573420/demo), for IQM spec & player (PD).
- [Lee Salzman, V.Hrytsenko, D.Madarász](https://github.com/zpl-c/enet/), for enet (MIT).
- [Libtomcrypt](https://github.com/libtom/libtomcrypt), for libtomcrypt (Unlicense).
- [Lua authors](https://www.lua.org/), for Lua language (MIT).
- [Mārtiņš Možeiko](https://gist.github.com/mmozeiko/68f0a8459ef2f98bcd879158011cc275), for A* pathfinding (PD).
- [Mattias Gustavsson](https://github.com/mattiasgustavsson/libs), for thread.h and https.h (PD).
- [Micha Mettke, Chris Willcocks, Dmitry Hrabrov](https://github.com/vurtun/nuklear), for nuklear (PD).
- [Michael Galetzka](https://github.com/Cultrarius/Swarmz), for swarmz (UNLICENSE).
- [Omar Cornut, vaiorabbit](https://github.com/ocornut/imgui/pull/3627), for tables of unicode ranges (MIT-0).
- [Rabia Alhaffar](https://github.com/Rabios/ice_libs), for ice_batt.h (PD).
- [Rich Geldreich](https://github.com/richgel999/miniz), for miniz (PD).
- [Ross Williams](http://ross.net/compression/lzrw3a.html) for lzrw3a (PD).
- [Samuli Raivio](https://github.com/bqqbarbhg/bq_websocket), for bq_websocket (PD).
- [Sean Barrett](https://github.com/nothings), for stb_image, stb_image_write, stb_sprintf, stb_truetype and stb_vorbis (PD).
- [Sebastian Steinhauer](https://github.com/kieselsteini), for sts_mixer (PD).
- [Stan Melax, Cloud Wu](https://web.archive.org/web/20031204035320/http://www.melax.com/polychop/gdmag.pdf), for polychop C algorithm (PD).
- [Stefan Gustavson](https://github.com/stegu/perlin-noise), for simplex noise (PD).
- [Tor Andersson](https://github.com/ccxvii/minilibs), for xml.c (PD).
- [Vassvik](https://github.com/vassvik/mv_easy_font), for mv_easy_font (Unlicense).
- Special thanks to [@ands](https://github.com/ands), [@barerose](https://github.com/barerose), [@datenwolf](https://github.com/datenwolf), [@evanw](https://github.com/evanw), [@glampert](https://github.com/glampert), [@krig](https://github.com/krig), [@sgorsten](https://github.com/sgorsten) and [@vurtun](https://github.com/vurtun) for their math libraries (PD,CC0,WTFPL2,CC0,PD,CC0,Unlicense,PD).

## Unlicense
This software is released into the [public domain](https://unlicense.org/). Also dual-licensed as [0-BSD](https://opensource.org/licenses/0BSD) or [MIT (No Attribution)](https://github.com/aws/mit-0) for those countries where public domain is a concern (sigh). Any contribution to this repository is implicitly subjected to the same release conditions aforementioned.

## Links
<p>
<a href="https://github.com/r-lyeh/FWK.2022/issues"><img alt="Issues" src="https://img.shields.io/github/issues-raw/r-lyeh/FWK.svg"/></a>
<a href="https://discord.gg/vu6Vt9d"><img alt="Discord" src="https://img.shields.io/discord/270565488365535232?color=5865F2&label=chat&logo=discord&logoColor=white"/></a><br/>

Still looking for alternatives?
[amulet](https://github.com/ianmaclarty/amulet), [aroma](https://github.com/leafo/aroma/), [astera](https://github.com/tek256/astera), [blendelf](https://github.com/jesterKing/BlendELF), [bullordengine](https://github.com/MarilynDafa/Bulllord-Engine), [candle](https://github.com/EvilPudding/candle), [cave](https://github.com/kieselsteini/cave), [chickpea](https://github.com/ivansafrin/chickpea), [corange](https://github.com/orangeduck/Corange), [cute](https://github.com/RandyGaul/cute_framework), [dos-like](https://github.com/mattiasgustavsson/dos-like), [ejoy2d](https://github.com/ejoy/ejoy2d), [exengine](https://github.com/exezin/exengine), [gunslinger](https://github.com/MrFrenik/gunslinger), [hate](https://github.com/excessive/hate), [island](https://github.com/island-org/island), [juno](https://github.com/rxi/juno), [l](https://github.com/Lyatus/L), [lgf](https://github.com/Planimeter/lgf), [limbus](https://github.com/redien/limbus), [love](https://github.com/love2d/love/), [lovr](https://github.com/bjornbytes/lovr), [mini3d](https://github.com/mini3d/mini3d), [mintaro](https://github.com/mackron/mintaro), [mio](https://github.com/ccxvii/mio), [olive.c](https://github.com/tsoding/olive.c), [opensource](https://github.com/w23/OpenSource), [ouzel](https://github.com/elnormous/ouzel/), [pez](https://github.com/prideout/pez), [pixie](https://github.com/mattiasgustavsson/pixie), [punity](https://github.com/martincohen/Punity), [r96](https://github.com/badlogic/r96), [ricotech](https://github.com/dbechrd/RicoTech), [rizz](https://github.com/septag/rizz), [tigr](https://github.com/erkkah/tigr), [yourgamelib](https://github.com/duddel/yourgamelib)
</p>

<p align="center">
<img src="https://raw.githubusercontent.com/r-lyeh/FWK.2022/master/tools/editor/logos/grid_4.png.removebg.png" width="512" height="512"/><br/>
</p>
