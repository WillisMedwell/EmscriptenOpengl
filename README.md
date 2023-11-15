# Native install
You will need to add the following dependencies for vcpkg.json file in addition.
```.json
    "dependencies": [
        "opengl",
        "glfw3",
        "glew",
        // others ...
    ]
```

## Resources
https://stackoverflow.com/questions/63062200/cmake-with-emscripten-and-vcpkg-cant-bind-two-toolchain-files
https://stackoverflow.com/questions/23997312/how-do-i-read-a-user-specified-file-in-an-emscripten-compiled-library
