![Praxis3D-logo](https://i.imgur.com/5vsuh0X.png)

A work-in-progress 3D game engine. Its purpose is to serve as a testbed for new ideas and implementations. My aim is to improve my skills and learn everything I can by exposing myself to every part of a game engine. I have been working on it during my free time for quite a while, motivated only by my passion for programming and problem-solving.

Written in C++/20, using OpenGL graphics API.
Shaders are coded in GLSL and gameplay scripts in LUA.
Scene files are saved in a modified JSON format.

More information and media can be found at the [project page](https://pauldev.org/project-praxis3d.html).

### In-game demo video (YouTube link):
[![Praxis3D-demo](https://img.youtube.com/vi/VWo4p8f9otY/0.jpg)](https://www.youtube.com/watch?v=VWo4p8f9otY)


## Main features:
- Atmospheric light scattering
- Automatic light exposure
- Cascaded shadow mapping
- Communication via observer-listener
- Data-driven and modular engine design
- Deferred renderer
- Entity Component System
- FXAA screen-space anti-aliasing
- GUI system using ImGui
- HBAO and SSAO ambient occlusion
- HDR and tone-mapping
- In-game scene editor
- LUA scripting
- Parallax occlusion mapping
- Physically based bloom
- Physically based rendering
- Physics system using Bullet3
- Sound system using FMOD
- Stochastic texture sampling
- Task-based multi-threading
- Text editor
- Texture inspector


## Building:

Project is self-contained - all dependencies are included (with the exception of [FMOD](https://www.fmod.com/download), as its license prohibits redistribution) and all settings within the Visual Studio project files are configured for an out of the box compilation in **Visual Studio 2022**. Windows OS and Visual Studio IDE only.

## Releases:

If you want to try out running the engine, but don't want to build it yourself, the compiled binaries of some of the versions are going to be uploaded as releases.
They will contain all the required DLLs and minimal assets to run a demo map.

## Screenshots:

![Praxis3D-1](https://i.imgur.com/N2ODD7U.jpg)
![Praxis3D-2](https://i.imgur.com/ALJ0bMm.jpg)
![Praxis3D-3](https://i.imgur.com/TsIDcI1.jpg)
![Praxis3D-4](https://i.imgur.com/1MMKrDY.jpg)
![Praxis3D-5](https://i.imgur.com/USlG6HK.jpg)
![Praxis3D-6](https://i.imgur.com/whdZFYq.jpg)
![Praxis3D-7](https://i.imgur.com/ssr3XVr.jpg)
![Praxis3D-8](https://i.imgur.com/LChDbaX.jpg)
![Praxis3D-9](https://i.imgur.com/pZ9dHiE.jpg)
![Praxis3D-10](https://i.imgur.com/CMmvtuN.jpg)

## Dependencies:
Libraries used in this project:

- [assimp (Open Asset Import Library)](https://github.com/assimp/assimp)
- [bullet3 (Bullet Physics)](https://github.com/bulletphysics/bullet3)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [EnTT](https://github.com/skypjack/entt)
- [FMOD](https://www.fmod.com/)
- [FreeImage](https://freeimage.sourceforge.io/)
- [GLEW (OpenGL Extension Wrangler Library)](https://github.com/nigels-com/glew)
- [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm)
- [ImGuiColorTextEdit](https://github.com/santaclose/ImGuiColorTextEdit)
- [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)
- [ImGuiTexInspect](https://github.com/andyborrell/imgui_tex_inspect)
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
- [imspinner](https://github.com/dalerank/imspinner)
- [LuaJIT](https://luajit.org/)
- [SDL (Simple DirectMedia Layer)](https://www.libsdl.org/)
- [Sol3](https://github.com/ThePhD/sol2)
- [TBB (Intel® Threading Building Blocks)](https://github.com/oneapi-src/oneTBB)

Fonts:
- [Open Sans (font)](https://github.com/googlefonts/opensans)
  
Their individual licenses can be found in **LICENSE.md** file.

## License:

**The software is licensed under the [MIT](https://choosealicense.com/licenses/mit/) License:**

Copyright © 2023 Paulius Akulavicius

Permission is hereby granted, free of charge, 
to any person obtaining a copy of this software 
and associated documentation files (the “Software”), 
to deal in the Software without restriction, 
including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit 
persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice 
shall be included in all copies or substantial portions 
of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.

## Contacts:

Ways to reach me can be found at my [website](http://pauldev.org/contact.html).
### Contacts ###

www.pauldev.org

paulcodedev@gmail.com
