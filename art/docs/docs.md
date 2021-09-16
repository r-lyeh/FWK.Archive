**[F·W·K (2021.9)](https://github.com/r-lyeh/FWK)**
 <!--p align="center">
  <img src="https://i.imgur.com/sInbRoA.gif"/>
 </p-->
+++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Glossary

## About

<!-- where and what,when,how much,why,who,how -->
- This is the documentation website for FWK.
- https://github.com/r-lyeh/FWK is a 3D game framework in C, started in 2020, with few new releases every year. Totally free, no licensing fees.
- Less is more. Minimalism everywhere: naming, language of choice, exposed API, no external deps, small repo size, etc.
- Need extra support? <a href="https://discord.gg/vu6Vt9d">Join us</a>.

![Image from demo.c](https://i.imgur.com/sInbRoA.gif)

~~~~~~C linenumbers
// Compact code style
#include "fwk.h"
int main(int argc, char **argv) {
    // Create window, 75% size, no extra flags
    window_create(75.0, 0);
    // Game loop
    while( window_swap() && !input(KEY_ESC) ) {
        puts("hello");
    }
}
~~~~~~

!!! Note
    While browsing documentation you'll find notes like this one.

!!! Tip
    This is a informational note however.

!!! WARNING
    This is a warning note.

!!! ERROR: Watch out
    Pay attention to these **error notes** always.

| Last modified | 2021/09/16  |
|:--------------|:------------|
|Commit:        | |
|Branch:        | |

<!-- 🅂 🅃 🄴 🄵 🅄 🄼 Ⓕ 🅕 🅔 🅢 🅣 🅜 -->
## 🅃 typedefs
## 🄴 enums
## 🅂 structs
## 🅄 unions
## 🄵 functions
[video_open](#api/video/video_open), [video_seek](#api/video/video_seek), [video_close](#api/video/video_close),
## 🄳 defines

# API
## video

<a name="api/video/video_open"></a>
<details><summary><code lang=C>API video_t video_open(char *filename, unsigned flags);</code></summary>

### video_open
Function that opens a video for displaying into a texture. 

- `filename`: video to open. 
- `flags`: `0` for now.
- Returns: `0` on success
- See also: [video_close](#api/video/video_close)
- Example:
~~~~~~C linenumbers
int main() {
    video_t v = video_open("my_video.mp4", 0);
}
~~~~~~

!!! Tip
    Use `WITH_VIDEO_YCBCR=1` directive to optimize video decoding.

!!! WARNING
    Fix me: *Audio decoder might exhaust buffers on Linux!*

</details>

<a name="api/video/video_seek"></a>
<details><summary><code lang=C>API int video_seek(video_t *video, double seek_ss);</code></summary>

### video_seek
Reposition the `video_t` stream. See also: [video_open](#api/video/video_open)

Argument|Description
-----|------
video|Video stream.
seek_ss|Reposition the video stream in `seek_ss` seconds from beginning.
return|1

~~~~~~C linenumbers
int main() {
    video_t *video = video_open("my_video.mp4", 0);
    video_seek(video, 3); // seek to 3s
}
~~~~~~
</details>

<a name="api/video/video_close"></a>
<details><summary><code lang=C>API int video_close(video_t *video);</code></summary>

### video_close
Close the `video_t` stream. See also: [video_open](#api/video/video_open)

Argument|Description
-----|------
video|Video stream.
return|1

~~~~~~C linenumbers
int main() {
    video_t *video = video_open("my_video.mp4", 0);
    video_close(video);
}
~~~~~~
</details>
