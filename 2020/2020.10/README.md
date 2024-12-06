# F·W·K
Minimalistic framework in plain C for quick 3D game development.

## Goals
- [x] ~C++~. C.
- [x] ~Fast~. Small.
- [x] ~Modern~. Fun.
- [x] ~Full featured~. Hackable.
- [x] ~Royaltie fee~. Free and unlicensed.

## Features
- [x] Portable: Windows, ~~Linux and OSX.~~
- [x] Windowing
- [x] UI: color3, color4, button, list, slider, checkbox, editbox...
- [x] Math: vec2, vec3, vec4, mat33, mat44, quat ~~and quat2~~.
- [x] Script: Lua ~~and FFI.~~
- [x] Network: web download (http/s and ftp) ~~and tcp/udp sockets.~~
- [x] Input: Keyboard, mouse and gamepads.
- [x] Rendering: naive but extendable GL3 renderer.
- [x] Audios: WAV, MOD, XM, FLAC, OGG, MP1, MP3 ~~and SFXR.~~
- [x] Models: GLTF, FBX, OBJ, DAE, MD3, MD5, MS3D, SMD, X, 3DS, BVH, DXF, LWO.
- [x] Textures: JPG, PNG, TGA, BMP, PSD, HDR, PIC, PNM ~~and GIF.~~
- [ ] Compressed Textures: DDS, KTX, ETC, BCn.
- [ ] 3D Anims.
- [ ] 2D Spines.
- [ ] Collisions.
- [ ] Level data/XML/JSON.

## Build (msvc, x64)
```
c:\fwk> cl test.c && test
```
Note: Assimp.dll may need [this package installed](https://www.microsoft.com/en-us/download/confirmation.aspx?id=30679).

## Showcase
![img](https://i.imgur.com/cIVYnul.png)

## License
The project is released into the public domain (as per [this notice](LICENSE)).<br/>
Any contribution to this repository is implicitly subjected to the same release conditions.

## Credits
- [Assimp authors](https://github.com/assimp/assimp), for assimp (BSD3).
- [Barerose](https://github.com/barerose), for swrap (CC0).
- [Camilla Löwy](https://github.com/elmindreda), for glfw3 (Zlib)
- [David Herberth](https://github.com/dav1dde/), for glad generated code (PD).
- [David Reid](https://github.com/mackron), for miniaudio (PD).
- [Goblin165](https://sketchfab.com/3d-models/halloween-little-witch-ccc023590bfb4789af9322864e42d1ab), for his 3D model (CC-BY-4.0).
- [Jon Olick](https://www.jonolick.com/), for jo_mp1 (PD).
- [Joshua Reisenauer](https://github.com/kd7tck), for jar_mod and jar_xm (PD).
- [Lua authors](https://www.lua.org/), for Lua language (MIT).
- [Micha Mettke](https://github.com/vurtun/nuklear), for nuklear (PD).
- [Sean Barrett](https://github.com/nothings), for stb_image and stb_vorbis (PD).
- [Sebastian Steinhauer](https://github.com/kieselsteini), for sts_mixer (PD).
- [Tomas Pettersson](http://www.drpetter.se/), for sfxr (PD).
- Special thanks to @ands (PD), @barerose (CC0), @datenwolf (WTFPL2), @evanw (CC0), @glampert (PD), @sgorsten (Unlicense), @vurtun (PD) for their math libraries.

## Alternatives
[amulet](https://google.com/search?q=github+amulet+game+engine),
[anvil](https://google.com/search?q=github+anvil+game+engine),
[aroma](https://google.com/search?q=github+aroma+game+engine),
[astera](https://google.com/search?q=github+astera+game+engine),
[atto](https://google.com/search?q=github+atto+game+engine),
[blendelf](https://google.com/search?q=github+blendelf+game+engine),
[blueshiftengine](https://google.com/search?q=github+blueshiftengine+game+engine),
[blunted2](https://google.com/search?q=github+blunted2+game+engine),
[bullordengine](https://google.com/search?q=github+bullordengine+game+engine),
[candle](https://google.com/search?q=github+candle+game+engine),
[cave](https://google.com/search?q=github+cave+game+engine),
[chickpea](https://google.com/search?q=github+chickpea+game+engine),
[corange](https://google.com/search?q=github+corange+game+engine),
[ejoy2d](https://google.com/search?q=github+ejoy2d+game+engine),
[exengine](https://google.com/search?q=github+exengine+game+engine),
[hate](https://google.com/search?q=github+hate+game+engine),
[island](https://google.com/search?q=github+island+game+engine),
[juno](https://google.com/search?q=github+juno+game+engine),
[l](https://google.com/search?q=github+l+game+engine),
[limbus](https://google.com/search?q=github+limbus+game+engine),
[love3d](https://google.com/search?q=github+love3d+game+engine),
[love](https://google.com/search?q=github+love+game+engine),
[mini3d](https://google.com/search?q=github+mini3d+game+engine),
[mintaro](https://google.com/search?q=github+mintaro+game+engine),
[mio](https://google.com/search?q=github+mio+game+engine),
[moon](https://google.com/search?q=github+moon+game+engine),
[newpixie](https://google.com/search?q=github+newpixie+game+engine),
[onut](https://google.com/search?q=github+onut+game+engine),
[openframeworks](https://google.com/search?q=github+openframeworks+game+engine),
[openkore](https://google.com/search?q=github+openkore+game+engine),
[pez](https://google.com/search?q=github+pez+game+engine),
[pmtech](https://google.com/search?q=github+pmtech+game+engine),
[punity](https://google.com/search?q=github+punity+game+engine),
[ricotech](https://google.com/search?q=github+ricotech+game+engine),
[rizz](https://google.com/search?q=github+rizz+game+engine),
[roam](https://google.com/search?q=github+roam+game+engine),
[spartanengine](https://google.com/search?q=github+spartanengine+game+engine),
[sushi](https://google.com/search?q=github+sushi+game+engine),
[tigr](https://google.com/search?q=github+tigr+game+engine),
[tinyengine](https://google.com/search?q=github+tinyengine+game+engine),
[wickedengine](https://google.com/search?q=github+wickedengine+game+engine),
