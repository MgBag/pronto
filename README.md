# Pronto
A work in progress game framework with a lot left to do, see [What's next](#Whats-next)

### Installation
Clone using the repo using `--recurse-submodules` or run `git submodule update --init` after cloning.

### Build 
Open `Prontal.sln` and compile, set the `game` project as `StartUp project`

### 'Play'
Fps controls on right mouse button hold with `WASD` and `Space bar`\
Camera movement also possible with the `Arrow keys`\
`Ctrl` and `Shift` are speed modifiers.\
\
Adjust camera sensitivity with `-` and `=`\
Adjust movement speed with `[` and `]`\
\
`1` Spawns a cube\
`2` Spawns a truck\
`3` Spawns a point light

## What's next
Quite a few things are in need of improvement:
- Fully dynamic pipeline state objects
- Instanced drawing
- GPU upload pipeline
- Entity deletion
- Physics integration
- Multiplatform support

And a few things on the todo:
- Spot lights
- Directional lights
- Shadow maps
- Normal maps
- PBR
- Ambient Ocolution 

## Credits
### Models
[Littlest Tokyo](https://sketchfab.com/3d-models/littlest-tokyo-94b24a60dc1b48248de50bf087c0f042) by glenatron under the [CC Attribution License](https://creativecommons.org/licenses/by/4.0/)\
[Cylander](https://sketchfab.com/3d-models/cylander-e3fda2adbd6a4bb19dfdb8f880fa3e15) by artwork guide under the [CC Attribution License](https://creativecommons.org/licenses/by/4.0/)\
[Sphere](https://sketchfab.com/3d-models/sphere-bb721dfff9594206aee62175e43c08b2) by oatmas64134 under the [CC Attribution License](https://creativecommons.org/licenses/by/4.0/)\
[Sponza](https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0/Sponza) taken from Khronos' glTF sample models\
Selection of glTF [sample models](https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0)

### Libraries
[TinyglTF](https://github.com/syoyo/tinygltf) by syoyo under the MIT license.\
[Bullet Physics](https://github.com/bulletphysics/bullet3) under the zlib/libpng license\
[glm](https://github.com/g-truc/glm) under the [MIT and Modified MIT License](https://github.com/g-truc/glm/blob/master/manual.md#section0)

### Tutorials
[3D Game Engine Programming](https://3dgep.com/)\
[Microsoft examples and documentation](https://github.com/microsoft/DirectX-Graphics-Samples)\
[Braynzarsoft](https://www.braynzarsoft.net)\
[Scratch pixel](https://www.scratchapixel.com)