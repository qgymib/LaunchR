cmake_minimum_required(VERSION 4.0)
project(LaunchR)

set(CMAKE_CXX_STANDARD 20)

###############################################################################
# Functions
###############################################################################

# Enable all reasonable warnings and make all warnings into errors.
function(setup_target_wall name)
    if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${name} PRIVATE /W4 /WX /utf-8)
    else ()
        target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
    endif ()
endfunction()

add_executable(${PROJECT_NAME} WIN32
        src/widgets/MainFrame.cpp
        src/main.cpp
        src/resource.rc
)
target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
setup_target_wall(${PROJECT_NAME})

###############################################################################
# Dependency
###############################################################################

set(wxBUILD_SHARED OFF)
add_subdirectory(third_party/wxWidgets)
target_link_libraries(${PROJECT_NAME} PUBLIC wx::net wx::core wx::base)

###############################################################################
# Setup static link
###############################################################################

if (MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE
            -static-libgcc
            -static-libstdc++
            -static
    )
endif ()
