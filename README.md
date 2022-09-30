# NoPoly
A small raymarching program that allows you to build little 3d scenes

![Marching Hammers](http://i.imgur.com/bRWJ2hZ.gif)

## Requirements
### include folder
- BalazsJako's text editor : https://github.com/BalazsJako/ImGuiColorTextEdit
- FFmpeg : https://ffmpeg.org/download.html
- x264 : https://www.videolan.org/developers/x264.html
- imgui : https://github.com/ocornut/imgui
- ImGuizmo : https://github.com/CedricGuillemet/ImGuizmo
- stb_image : https://github.com/nothings/stb/blob/master/stb_image.h
- dirent : https://github.com/tronkko/dirent
- glad : https://glad.dav1d.de/#profile=compatibility&language=c&specification=gl&loader=on&api=gl%3D4.6
- GLFW : https://www.glfw.org
- glm : https://github.com/g-truc/glm

### lib folder
all libs coming from ffmpeg, x264, GLFW3

### binary folder (x64/Release or Debug)
all dll coming from ffmpeg, x264, GLFW3

## Features
non-exhaustive list
### Scene building
- preset primitives (box, sphere, plane...)
- lighting (global such as sun color and direction, and additionnal such as directionnal, spot, point lights)
- basic triplanar texturing (normal maps are a bit wrong)

### Coding
- custom primitives
- space distortions

### Other
- some debug options
- loading/saving scenes and functions
- time management

## Warning
There is a feature that allows you to save a video of your scene but it is very glitchy (easy crashs and lots of encoding issues), this was just a way to try learning about video encoding but as a matter of fact i don't understand very much yet how ffmpeg works.

Please ensure you have saved all your "SpaceRemap" and "Primitive" custom functions before saving the scene as you might not be able to reload it later (in the .scene file you could try to find a line where there is a "function" key but no path written after it)

You might encounter other issues

## Examples
![IQ Thanks](https://i.imgur.com/r22yhqq.gif)
Thanks to Inigo Quilez (https://iquilezles.org) for his amazing website which helped me a lot

![Tie](https://i.imgur.com/mH3RgDF.gif)

![Portals](https://i.imgur.com/y2BKBaD.gif)

![Debug Distance And Steps](https://i.imgur.com/hd6nPso.png)

![Debug Trespassing](https://i.imgur.com/fYHoWAJ.png)
