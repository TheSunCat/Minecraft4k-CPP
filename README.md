# Minecraft4k-CPP
Minecraft4k-CPP is a port of the original Minecraft4k-Reversed to C++, so that we are able to leverage both the speed of C++ and mainly the GPU itself for raymarching.<br>
<br>
Thus far, it already performs much better than the original game, as long as your GPU is powerful enough (make sure it supports OpenGL 4.3 Core). However, it hasn't been tested very thoroughly in different machines.<br>
If you encounter an issue, please let me know by creating an Issue including your PC specs and the beginning of the console output. Thanks!<br>
<br>

High resolution raymarched shadows:
![image](https://user-images.githubusercontent.com/44881120/114410780-ea723c00-9bab-11eb-85b2-a344b981988d.png)

Sunshet at original resolution:
![image](https://user-images.githubusercontent.com/44881120/114410800-f100b380-9bab-11eb-8eb5-f48e4ee0ed91.png)

"Classic" world and graphics (compile-time flag):
![image](https://user-images.githubusercontent.com/44881120/114411466-7c7a4480-9bac-11eb-8836-bfa5039fa636.png)

<br>
<br>
## Controls:<br>
- Move: WASD<br>
- Jump: Space<br>
- Break block: Left Click<br>
- Place block: Right Click<br>
- Cycle through inventory: Scroll<br>
- Higher resolution: Dot<br>
- Lower resolution: Comma<br>
- Render distance: + and -<br>
<br>
<br>
This project is a reverse-engineered version of Notch's submission for the 2010 Java 4k Contest, where one would submit Java programs of 4kb or under in size.<br>
<br>
Notch submitted Minecraft4k, a heavily optimized and limited Minecraft port. However, the game contained a multitude of bugs and is difficult to play today.
The original build can be found at https://archive.org/details/Minecraft4K. <br>
There is a Minecraft wiki page for Minecraft4k: https://minecraft.gamepedia.com/Minecraft_4k.<br>
<br>
Minecraft4k-Reversed was created by decompiling this jarfile, and documenting what most of the code does. After fixing some bugs related to movement and collision detection, we moved to expand the game. This resulted in proper terrain generation, raycasted shadows, better graphics, and huge performance improvements thanks to multithreading raymarching on the CPU. However, it became difficult to manage and improve performance further, so we moved to developing this C++ port which uses the GPU for raymarching.<br>
<br>
Some interesting things:<br>
- Textures take too much space to store. They are generated on game start by algorithms!<br>
- Notch wrote a custom voxel raymarching algorithm to avoid the filesize penalty of using a rasterizer library<br>
- Block shading is "baked in", where the bottom of every block has extra grayscale applied to it<br>

# Building
Please excuse the messy build environment; this was my first time using CMake. Hopefully everythink should just work.
When running the executable, make sure the `res` folder is in the working directory!

## Linux
You'll need the `linux-devel` and `glfw` packages installed, so you can link against `dl` and `glfw`. You'll also need `cmake`.
Run the following commands:
- `mkdir build; cd build`
- `cmake ..`
- `make`
- `cp Minecraft4K ..; cd ..`
- `./Minecraft4k`

## Windows
Run CMake to generate a .sln file, and open it in Visual Studio. It *should* build fine from there.
