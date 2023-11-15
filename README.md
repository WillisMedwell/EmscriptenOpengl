# Emscripten Opengl
This is a repo i have made seperately for my project.

Chosen to do this so that I can use github actions to compile my project to WASM/Webgl2 automatically. Also the toolchain for building this on a windows machine is way too annoying especially if you have other compilers like clang/msvc/gcc(mysys2) installed. 

Porting this so it compiles to both native and web. I think its a major part of games development considering the popularity of [unity's web export](https://docs.unity3d.com/Manual/webgl-gettingstarted.html).
## Native Build
You will need to add the following dependencies for vcpkg.json file in addition to the other ones.
```.json
    "dependencies": [
        "opengl",
        "glfw3",
        "glew",
        ...
    ]
```
Then it should be as simple as:
```
mkdir build
cd build
cmake ..
```

## Web Build
On windows this was way too difficult especially using vcpkg manager as it has its unexpected 'quirks'. 

You can have a look at the workflow/action to understand what you need; but for me it was much easier to spool up a fresh docker container and compile it that way.

## Resources
- [vcpkg and cmake](https://stackoverflow.com/questions/63062200/cmake-with-emscripten-and-vcpkg-cant-bind-two-toolchain-files)
- [file reading](https://stackoverflow.com/questions/23997312/how-do-i-read-a-user-specified-file-in-an-emscripten-compiled-library)
- Major resource to see what I need was the [Emscripten test suite](https://github.com/emscripten-core/emscripten/tree/main/test) as it's pretty poorly documented.
