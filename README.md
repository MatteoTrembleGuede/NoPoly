# NoPoly
A small raymarching program that allows you to build little 3d scenes

![Marching Hammers](http://i.imgur.com/bRWJ2hZ.gif)

Inspired by Pink Floyd's Marching Hammers

## Requirements
### include folder
- BalazsJako's text editor : https://github.com/BalazsJako/ImGuiColorTextEdit
- imgui : https://github.com/ocornut/imgui
- ImGuizmo : https://github.com/CedricGuillemet/ImGuizmo
- stb_image : https://github.com/nothings/stb/blob/master/stb_image.h
- dirent : https://github.com/tronkko/dirent
- glad : https://glad.dav1d.de/#profile=compatibility&language=c&specification=gl&loader=on&api=gl%3D4.6
- GLFW : https://www.glfw.org
- glm : https://github.com/g-truc/glm
- Input : https://github.com/MatteoTrembleGuede/Input-system-for-glfw3-delegate

### lib folder
all libs coming from GLFW3

### binary folder (x64/Release or Debug)
openh264-1.8.0-win64.dll and glfw3.dll

### opencv
opencv 4.6.0 must be installed
you need to add environment variables :
- in your system variables : OPENCV_DIR <path to opencv directory containing bin and lib folders> (for example in my case : C:\Program Files\Git\usr\local\x64\vc16)
- add %OPENCV_DIR%\bin to Path

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
video recording only works in Release

Please ensure you have saved all your "SpaceRemap" and "Primitive" custom functions before saving the scene as you might not be able to reload it later (in the .scene file you could try to find a line where there is a "function" key but no path written after it)

You might encounter other issues

## Examples
![IQ Thanks](https://i.imgur.com/r22yhqq.gif)

Thanks to Inigo Quilez (https://iquilezles.org) for his amazing website which helped me a lot

![CreaLogo](https://i.imgur.com/co4mV0w.gif)

Creajeux's animated Logo i made for the intro of the game i worked on during my last year in school (paradox : https://www.creajeux.fr/project/paradox/)

![Tie](https://i.imgur.com/mH3RgDF.gif)

![Portals](https://i.imgur.com/y2BKBaD.gif)

![Hat](https://i.imgur.com/v9T23lu.png)

![Debug Distance And Steps](https://i.imgur.com/hd6nPso.png)

![Debug Trespassing](https://i.imgur.com/fYHoWAJ.png)
