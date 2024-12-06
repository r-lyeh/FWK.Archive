<h1 align="center"><a href="https://bit.ly/-fwk-">F·W·K</a></h1>
<p align="center">3D game framework in C, with Luajit bindings now.</p>

<p align="center">
<img src="https://i.imgur.com/sInbRoA.gif"/><br/>
</p>

## Goals
- [x] ~~C++~~. C.
- [x] ~~Fast~~. Naive.
- [x] ~~Modern~~. Simple.
- [x] ~~Full featured~~. Small.
- [x] ~~Rich build system~~. Single file.
- [x] ~~Royaltie fee~~. Free and unlicensed.

## Features ᕦ(ᐛ)ᕤ
- [x] Pipeline: configurable and integrated [asset pipeline](fwk.ini).
- [x] Embedded: single-file, all dependencies included.
- [x] Compiler: MSVC, MINGW64, TCC, GCC and clang.
- [x] Platform: Windows, Linux and OSX.
- [x] DS: hash, sort, array/vector, map, set.
- [x] Math: rand, noise, ease, vec2/3/4, mat33/34/44, quat.
- [x] Geometry: ray, line, plane, aabb, sphere, capsule, triangle, poly and frustum.
- [x] Window: windowed, soft/hard fullscreen, msaa, icon, cursor handling.
- [x] Input: keyboard, mouse and gamepads.
- [x] Script: Lua scripting, Luajit bindings.
- [x] Network: downloads (HTTPS) and sockets (TCP/UDP).
- [x] UI: color3/4, button, list, slider, toggle, checkbox, editbox, dialog, image, menus.
- [x] Font: TTF and TTC. Basic syntax highlighter. Unicode ranges, atlasing, I18N.
- [x] Image: JPG, PNG, BMP, PSD, PIC, PNM, ICO.
- [x] Texture: KTX/2, PVR, DDS, ASTC, BASIS, HDR, TGA.
- [x] Texel: Depth, R, RG, RGB, RGBA, BC1/2/3/4/5/6/7, PVRI/II, ETC1/2, ASTC.
- [x] Audio: WAV/FLAC, OGG/MP1/MP3, MOD/XM/S3M/IT, SFXR and MID.
- [x] Video: MP4, MPG, OGV, MKV, WMV and AVI.
- [x] Model: IQM/E, GLTF/2, GLB, FBX, OBJ, DAE, BLEND, MD3/5, MS3D, SMD, X, 3DS, BVH, DXF, LWO.
- [x] Render: PBR (metallic-roughness) workflow.
- [x] Render: Cubemaps, panoramas and spherical harmonics. Rayleigh/Mie scattering.
- [x] Render: Post-effects (SSAO,FXAA1/3,CRT,Contrast,Grain,Outline,Vignette...).
- [x] Render: 3D Anims, skeletal anims, hardware skinning and instanced rendering.
- [x] Render: 3D Debugdraw, batching and vectorial font.
- [x] Render: 2D Sprites, spritesheets, AA zooming and batching.
- [x] Compression: DEFLATE, LZMA, LZ4, ULZ, BALZ, BCM, CRUSH, LZW3, LZSS and PPP.
- [x] Virtual filesystem: ZIP, PAK, TAR and DIR.
- [x] Level data: JSON, JSON5, SJSON, XML.
- [x] Disk cache.
- [x] Scene handling.
- [x] Profiler, stats and leaks finder.
- [x] [Documentation (wip)](https://bit.ly/-fwk-).

## Roadmap ᕕ(ᐛ)ᕗ (in order of arrival; ✱: partial support)
- [ ] Network: NAT traversal. Socketless API, message API and pub/sub wrappers (enet/websocket).
- [ ] Render: Materials (colors✱, textures✱, matcaps✱, videos✱, shadertoys✱). Shadertoys as post-fx✱. <!--materials as postfx, as they have an update() method -->
- [ ] Render: Hard/soft shadow mapping and baked lightmaps.
- [ ] Scene: toggles on/off (billboards✱, materials, un/lit, cast shadows, wireframe, skybox/mie, collide, physics).
- [ ] Scene: node singleton display, node console, node labels, node outlines.
- [ ] Math: quat2, bezier, catmull.
- [ ] Editor: gizmos✱, scene tree, property editor, load/save, undo/redo, copy/paste. <!-- editor = tree of nodes. levels and objects are nodes, and widgets are also nodes --><!-- you can perform actions on nodes, with or without descendants, top-bottom or bottom-top --><!-- operations include load/save, reset, undo/redo, play/render vis on/off/alpha logic on/off/other ddraw on/off log on/off, etc -->
- [ ] Level objects: volumes✱, triggers, platforms, streaming.
- [ ] Script: DLL✱ (module->plugin/sys), Lua✱, Luajit✱, Teal✱ and TypeScript.
- [ ] Script: Docs/Refl/Meta binding tool (during cook stage).
- [ ] Pipeline: Extend: shaders, bindings. Per-platform✱, per-type✱, per-asset options. GIF, PKM.
- [ ] Pipeline: Extend atlas (sprite/lightmaps). Fit packing (sprites).
- [ ] Maybe: Font text layout and shaping, underlining, soft/hard shadows, outlines.
- [ ] Maybe: Tiled maps and 2D spines. Sprite parallaxs.
- [ ] Maybe: Animation pass.
- [ ] Maybe: AI/Logic pass.
- [ ] Maybe: Lighting pass.
- [ ] Maybe: VM/ECS/Replication pass.
- [ ] API: More examples. Also, less examples.
- [ ] API: Discuss API and freeze it.
- [ ] API: Document everything.

<!--
// almost done:
// [x] shadertoy textures
// [*] billboards (follow sprite API? state instead? ie, billboard(true); before rendering?)
// [ ] soft shadows (vsm, vsmcube)

// nice to have:
// [ ] fix leaks and todos
// [ ] fwk_app: cpu usage, orientation
// [ ] fwk_input: cursor, mouse clip, mouse wrap,
// [ ] zip0 seek-vfs optimization. zip_append_file is suboptimal, and requires tons of contiguous memory for giant files.
// [ ] expose uniforms as ui options, also model_uniform();
// [ ] font: M/SDF https://github.com/WilliamBundy/wiggle https://github.com/Chlumsky/msdf-atlas-gen
// [ ] lod: https://github.com/songrun/SeamAwareDecimater
// [ ] lightmaps: https://blackpawn.com/texts/lightmaps/default.html 
// [ ] https://github.com/jpcy/xatlas

// plan:
//[ ] Script: Refl/meta binding tool (during cook stage).
//    fwk_cook (*.c, *.h) as .proto/.pbc maybe, free reflection+automatic bindings
// 4) (json) level editor: load/save jsons, property editor for anything (remote osc server/client)
//    cam: friction, projections (dimetric, isometric, ...)
//    gizmo: proportional, arcball XY (+shift for Z/tilt)
//    scene: scenegraph, obj naming, ~~obj picking, obj bounds,~~ obj collisions, obj/scene streaming
//    placeholders google
//    vcs
//[ ] Level objects: ~~volumes, triggers, platforms, streaming~~.
//    level: emitters: particles, lights, lightmaps, sound sources, triggers, etc
//    level: box triggers, start/end, spawn, streaming, checkpoints,
//    level: jump, shoots, platforms, collisions
//    level: 60s, 70s, 80s, 90s
//[ ] Render: Materials (textures, matcaps, videos, shadertoys).
//    material: fixed color, texture or script that returns color
//    animated textures (shadertoys, videos)
//[ ] Pipeline: Extend asset pipeline (shaders, bindings, xml). Asset options.
//      cook: hlsl2glsl (XShaderCompiler), hlsl2spirv (dxc), spirv2many (spirv-cross), glsl2many (sokol-shdc)
//      cook: tiled2json, spine2json, tlfx22json
// 6) anims, I (playlist: forward/backwards/loop/rewind), II (blend/shapes), III (ik/bone), IV (graph/controller)
//    blend anims, animtracks+animevents, ~~draw skeleton~~, additive anims,
//    fwk_data: quantization: ~~half, quant, microfloat~~.
//    anim; keyframes[] { frame+delay,frame+delay,... }, anim duration, anim flip
//[ ] Maybe: AI/Logic pass.
//    app: app state manager (h/fsm modules)
//    scripts: states, signals, h/fsm, coroutines, load/save context
//    ai: h/fsm for level objects and small enemies, planning GOAL, BTrees,
//    nav: navpaths, waypoints, navigation, pathfinding (jps.hh), A*, A* GOAL planning, swarm/flocks,
//[ ] Maybe: VM/ECS/Replication pass.
// 8) vm/ecs core + engines: custom frontends & backends
//    vm: ram, workqueues, threading, priorities, load/save
//    service protocols: websocket bqqbarbhg/bq_websocket, https, handshake
//    databases, services, quotas, black/whitelists, etc
//[ ] Core: wecs+replication
//    modules: script or dll + ram load/save/diff/patch + play/stop/init/ + attach/detach
//    logic tree/ << [] |> || >>
//    - scene |>
//       - enemies
//    ecs: sys are modules, ecs: messaging, ecs: filesystem (e/dir,c/files,s/dll)
//    world: streaming, migration
// 7) network replication & messaging
//    network: replication, dead reckoning, interpolation, extrapolation, bandwidth
//    network: messaging: un/reliable, fragmentation, priority, etc
//    network: topologies: bus, star, p2p, pubsub, etc
//    network: filesystem
//    int send_game_state(void *ptr, int len, int flags); PROTOCOL_V1|QUANTIZE|COMPRESS|RLE
//    int recv_game_state(); compensate, extrapolate, intrapolate(); lerp();
//[ ] Maybe: Lighting/PBR pass.
// 9) render+
//    lighting: vsm,vsmcube (per @procedural's request)
//    2d: billboards
//    PBR/IBL/materials (from Foxotron+sgorsten) + shading models
//    lightmapping/xatlas (demos), reflection probes
//    ~~instancing~~, frustum culling, impostors, mesh lods,
//    renderbuckets
//    decals
//    tessellation
//    2d: spines, particles (tlfx2)
//    reverse-z {
//    fbo attach format D16_UNORM -> D32_SFLOAT
//    pipeline depth compare LEQUAL -> GEQUAL
//    pipeline depth clear 1.0 -> 0.0
//    proj matrix: float a = zfar / (zfar - znear); -> float a = -znear / (zfar - znear);
//    proj matrix: float b = (-znear * zfar) / (zfar - znear); -> float b = (znear * zfar) / (zfar - znear);
//    }


	char* os_exec(...) -> int rc = popen(...); [...] return va("%c%s", (unsigned char)rc, output);
	char *result = os_exec("dir *.f")
	int rc = result[0];
	char *log = result+1;

-->

## Gallery
<p align="center">
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_script.png"    width="204px" title="Script."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_font.png"      width="204px" title="Fonts."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_collide.png"   width="204px" title="Collision."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_model.png"     width="204px" title="Model."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_scene.png"     width="204px" title="Scene."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_sprite.png"    width="204px" title="Sprite."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_shadertoy.png" width="204px" title="Shadertoy."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_cubemap.png"   width="204px" title="Cubemaps and SH."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_video.png"     width="204px" title="Video."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_pbr.png"       width="204px" title="PBR."/>
<img src="https://raw.githubusercontent.com/r-lyeh/FWK/master/demos/demo_instanced.png" width="204px" title="Instanced rendering."/>
<br/>
</p>

## Build (as static library)
Type `MAKE.bat` (Win) or `sh MAKE.bat` (Linux/OSX) to build everything. Alternatively,

```lua
echo Windows (vc+tcc)      && cl  demo.c       fwk.c
echo Windows (mingw64)     && gcc demo.c       fwk.c -o demo -w -lws2_32 -lgdi32 -lwinmm -ldbghelp
echo Linux (gcc+clang+tcc) && cc  demo.c       fwk.c -o demo -w -lm -ldl -lpthread
echo OSX (gcc+clang)       && cc  demo.c -ObjC fwk.c -o demo -w -framework cocoa -framework iokit
```
<!-- - Note: Windows: Assimp.dll may need [this package installed](https://www.microsoft.com/en-us/download/confirmation.aspx?id=30679).-->
- Note: TCC is partially supported on Windows+Linux. Beware, no threading.

## Build (as dynamic library)
Type `MAKE.bat dll` or `sh MAKE.bat dll` (Linux/OSX) to build FWK as a dynamic library. Alternatively,

```lua
echo Windows && cl fwk.c /LD /DAPI=EXPORT
echo Linux   && cc -fPIC fwk.c -shared -o libfwk.so -w
echo OSX     && cc -ObjC -dynamiclib -o libfwk.dylib fwk.c -framework cocoa -framework iokit -w
```

- Quick test on Windows: `cl demo.c fwk.lib /DAPI=IMPORT`
- Or also, copy the dynamic library into `demos/lua` and then run `luajit demo_luajit_model.lua` from there.

## Amalgamation
- Split FWK into separate files by running `art/tools/split.bat` (or `sh art/tools/split.bat` in Linux/OSX).
- Merge those files back into FWK by running `art/tools/join.bat` (or `sh art/tools/join.bat` in Linux/OSX).
- Optionally, generate a single-header distribution by executing following script:

```lua
echo // This C file is a header that you can #include. Do #define FWK_C  > fwk-single-header.c
echo // early in **one** compilation unit to unroll the implementation. >> fwk-single-header.c
type fwk.h           >> fwk-single-header.c
echo #ifdef FWK_C    >> fwk-single-header.c
echo #pragma once    >> fwk-single-header.c
type fwk.c           >> fwk-single-header.c
echo #endif // FWK_C >> fwk-single-header.c
```

## Extra tips
- Dropped files into game window will be imported & saved into [`art/import`](art/import) folder.
- Linux/OSX users can optionally install wine to use the Windows pipeline as an alternate asset pipeline (use `--with-wine` flag).
- Although not recommended, you can remove the cooking stage by invoking `--with-jobs=0` or by removing the [`art/tools`](art/tools) folder.
- Depending on your IDE, you might need to [split all amalgamated](#Amalgamation) files when debugging FWK.
<!-- - On windows + vc, you can use `make bindings` or `make docs` to generate everything prior to a release --><!-- gamecontrollerdb.txt -->

## Bindings
- Luajit: Luajit bindings are provided in the [fwk.lua](demos/lua/fwk.lua) auto-generated file.
- Nelua: [Nelua bindings](https://github.com/Rabios/nelua-fun/tree/main/fwk) provided by Rabia Alhaffar.

## License
This software is released into the [public domain](https://unlicense.org/).<br/>
Also dual-licensed as [0-BSD](https://opensource.org/licenses/FPL-1.0.0) or [MIT (No Attribution)](https://github.com/aws/mit-0) for those countries where public domain is a concern (sigh).<br/>
Any contribution to this repository is implicitly subjected to the same release conditions aforementioned.

## Credits (Artwork + demos)
- [FMS_Cat](https://gist.github.com/FMS-Cat/a1ccea3ce866c34706084e3526204f4f), for nicest VHS/VCR shader around (MIT).
- [Goblin165cm](https://sketchfab.com/3d-models/halloween-little-witch-ccc023590bfb4789af9322864e42d1ab), for witch 3D model (CC BY 4.0).
- [Nanofactory](https://sketchfab.com/3d-models/kgirls01-d2f946f58a8040ae993cda70c97b302c), for kgirls01 3D model (CC BY-NC-ND 4.0).
- [Quaternius](https://www.patreon.com/quaternius), for the lovely 3D robots (CC0).
- [Rxi](https://github.com/rxi/autobatch), for lovely sprites & cats demo (MIT).
- [wwwtyro](https://github.com/wwwtyro/glsl-atmosphere), for nicest rayleigh/mie scattering shader around (CC0).

## Credits (Tools)
- [Aaron Barany](https://github.com/akb825/Cuttlefish), for cuttlefish (APACHE2).
- [Assimp authors](https://github.com/assimp/assimp), for assimp (BSD3).
- [Bernhard Schelling](https://github.com/schellingb/TinySoundFont), for tml.h (Zlib) and tsf.h (MIT).
- [ffmpeg authors](https://www.ffmpeg.org/), for ffmpeg (LGPL21).
- [Imagination](https://developer.imaginationtech.com/pvrtextool/), for pvrtextoolcli (ITL).
- [Krzysztof Gabis](https://github.com/kgabis/ape), for split.py/join.py (MIT).
- [Lee Salzman](https://github.com/lsalzman/iqm/tree/5882b8c32fa622eba3861a621bb715d693573420/demo), for iqm.cpp (PD).
- [Mattias Gustavsson](https://github.com/mattiasgustavsson/libs), for mid.h (PD).
- [Michael Schmoock](http://github.com/willsteel/lcpp), for lcpp (MIT).
- [Morgan McGuire](https://casual-effects.com/markdeep/), for markdeep (BSD2).
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
- [Lee Salzman](https://github.com/lsalzman/iqm/tree/5882b8c32fa622eba3861a621bb715d693573420/demo), for IQM spec & player (PD).
- [Lee Salzman, V.Hrytsenko and D.Madarász](https://github.com/zpl-c/enet/), for enet (MIT).
- [Libtomcrypt](https://github.com/libtom/libtomcrypt), for libtomcrypt (Unlicense).
- [Lua authors](https://www.lua.org/), for Lua language (MIT).
- [Mattias Gustavsson](https://github.com/mattiasgustavsson/libs), for thread.h and https.h (PD).
- [Micha Mettke](https://github.com/vurtun/nuklear), for nuklear (PD).
- [Omar Cornut, vaiorabbit](https://github.com/ocornut/imgui/pull/3627), for tables of unicode ranges (MIT-0).
- [Rich Geldreich](https://github.com/richgel999/miniz), for miniz (PD).
- [Ross Williams](http://ross.net/compression/lzrw3a.html) for lzrw3a (PD).
- [Samuli Raivio](https://github.com/bqqbarbhg/bq_websocket), for bq_websocket (PD).
- [Sean Barrett](https://github.com/nothings), for stb_image, stb_image_write, stb_truetype and stb_vorbis (PD).
- [Sebastian Steinhauer](https://github.com/kieselsteini), for sts_mixer (PD).
- [Stefan Gustavson](https://github.com/stegu/perlin-noise), for simplex noise (PD).
- [Vassvik](https://github.com/vassvik/mv_easy_font), for mv_easy_font (Unlicense).
- Special thanks to [@ands](https://github.com/ands) (PD), [@barerose](https://github.com/barerose) (CC0), [@datenwolf](https://github.com/datenwolf) (WTFPL2), [@evanw](https://github.com/evanw) (CC0), [@glampert](https://github.com/glampert) (PD), [@krig](https://github.com/krig) (CC0), [@sgorsten](https://github.com/sgorsten) (Unlicense) and [@vurtun](https://github.com/vurtun) (PD) for their math libraries.

## Links
<a href="https://github.com/r-lyeh/FWK/issues"><img alt="Issues" src="https://img.shields.io/github/issues-raw/r-lyeh/FWK.svg"/></a>
<a href="https://discord.gg/vu6Vt9d"><img alt="Discord" src="https://img.shields.io/discord/270565488365535232?color=5865F2&label=chat&logo=discord&logoColor=white"/></a><br/>

Still looking for alternatives?
[amulet](https://github.com/ianmaclarty/amulet),
[aroma](https://github.com/leafo/aroma/),
[astera](https://github.com/tek256/astera),
[blendelf](https://github.com/jesterKing/BlendELF),
[bullordengine](https://github.com/MarilynDafa/Bulllord-Engine),
[candle](https://github.com/EvilPudding/candle),
[cave](https://github.com/kieselsteini/cave),
[chickpea](https://github.com/ivansafrin/chickpea),
[corange](https://github.com/orangeduck/Corange),
[cute](https://github.com/RandyGaul/cute_framework),
[dos-like](https://github.com/mattiasgustavsson/dos-like),
[ejoy2d](https://github.com/ejoy/ejoy2d),
[exengine](https://github.com/exezin/exengine),
[gunslinger](https://github.com/MrFrenik/gunslinger),
[hate](https://github.com/excessive/hate),
[island](https://github.com/island-org/island),
[juno](https://github.com/rxi/juno),
[l](https://github.com/Lyatus/L),
[lgf](https://github.com/Planimeter/lgf),
[limbus](https://github.com/redien/limbus),
[love](https://github.com/love2d/love/),
[lovr](https://github.com/bjornbytes/lovr),
[mini3d](https://github.com/mini3d/mini3d),
[mintaro](https://github.com/mackron/mintaro),
[mio](https://github.com/ccxvii/mio),
[opensource](https://github.com/w23/OpenSource),
[ouzel](https://github.com/elnormous/ouzel/),
[pez](https://github.com/prideout/pez),
[pixie](https://github.com/mattiasgustavsson/pixie),
[punity](https://github.com/martincohen/Punity),
[ricotech](https://github.com/dbechrd/RicoTech),
[rizz](https://github.com/septag/rizz),
[tigr](https://github.com/erkkah/tigr),
