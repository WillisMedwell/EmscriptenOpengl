cmake_minimum_required(VERSION 3.11)

project(Graphics)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c" "src/*.cxx")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h")

find_package(glm CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED) 

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

if(NOT DEFINED EMSCRIPTEN)
    add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})

    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)

    target_include_directories(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )
    
    message(STATUS "${CMAKE_SOURCE_DIR}")

    target_link_libraries(
        ${CMAKE_PROJECT_NAME} 
        PRIVATE 
            OpenGL::GL
            GLEW::GLEW
            glfw
            glm::glm
            glaze::glaze
            lodepng
    )
elseif(DEFINED EMSCRIPTEN)
    add_definitions(-DEMSCRIPTEN)
    add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fexperimental-library)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_RUNTIME_METHODS='[\"callMain\"]' -s DEMANGLE_SUPPORT=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -s ASSERTIONS=2 -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=1 -s NO_EXIT_RUNTIME=0")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    target_include_directories(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(
        ${CMAKE_PROJECT_NAME} 
        PRIVATE 
            glm::glm
            glaze::glaze
            lodepng
    )
endif()
