cmake_minimum_required(VERSION 3.11)

project(Graphics)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c" "src/*.cxx")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h")

find_package(glm CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)

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
    )
elseif(DEFINED EMSCRIPTEN)
    add_definitions(-DEMSCRIPTEN)

    message(STATUS "EMSCRIPTEN BUILD")

    add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})

    message(STATUS "${CMAKE_SOURCE_DIR}")
    message(STATUS "${CMAKE_SOURCE_DIR}")

    message(STATUS "${HEADERS}")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_WEBGL2=1 -s USE_GLFW=3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_RUNTIME_METHODS='[\"callMain\"]' -s DEMANGLE_SUPPORT=1")
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
    )
endif()
