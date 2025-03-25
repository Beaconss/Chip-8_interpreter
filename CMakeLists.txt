cmake_minimum_required(VERSION 3.14)
project(Chip-8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out)

set(SFML_STATIC_LIBRARIES TRUE)
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    CMAKE_ARGS 
        -DSFML_STATIC_LIBRARIES=TRUE
        -DSFML_BUILD_AUDIO=TRUE
        -DSFML_USE_STATIC_STD_LIBS=TRUE
        -DSFML_SYSTEM_DEPENDS=STATIC
        -DSFML_SKIP_INSTALL=ON
)

FetchContent_MakeAvailable(sfml)

file(GLOB SOURCES "Chip-8_interpreter/src/*.cpp" "Chip-8_interpreter/src/*.h")

add_executable(Chip-8 ${SOURCES})

target_link_libraries(Chip-8 PRIVATE 
    sfml-graphics 
    sfml-audio
    sfml-system 
    sfml-window
)

IF(WIN32)
add_custom_command(TARGET Chip-8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:Chip-8>/OpenAL32.dll"
    COMMENT "Removing OpenAL32.dll")
ENDIF()
