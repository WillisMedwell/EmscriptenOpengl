# Native install
* You need cmake
* The packages `opengl, GLEW, glfw3` need to be installed so it can be found by cmake. *If you don't know how to do this I would use vcpkg then do the following:*
    ```
    vcpkg install opengl
    vcpkg install glew
    vcpkg install glfw3
    ```
    ```
    -DCMAKE_TOOLCHAIN_FILE=C:/apps/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake
    ```