cmake_minimum_required(VERSION 3.26)

project(Praxis3D VERSION 0.1.0 LANGUAGES CXX RC)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# compile_commands.json (for clangd etc.) is only emitted by single-config generators like Ninja
if(NOT CMAKE_GENERATOR MATCHES "Ninja")
    message(WARNING "Non-Ninja generator - compile_commands.json will not be generated.")
endif()

if(NOT WIN32)
    message(FATAL_ERROR "Praxis3D currently supports Windows only.")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "Praxis3D requires an MSVC toolchain because its bundled dependencies are MSVC libraries.")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build configuration" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release)$")
    message(FATAL_ERROR "Praxis3D only supports Debug and Release configurations (got '${CMAKE_BUILD_TYPE}').")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FMOD REQUIRED)

set(PRAXIS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Praxis3D")
set(PRAXIS_DEPENDENCIES "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies")

file(GLOB_RECURSE PRAXIS_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${PRAXIS_ROOT}/Source/*.cpp")

set(PRAXIS_IMGUI_SOURCES
    "${PRAXIS_DEPENDENCIES}/include/ImGuiColorTextEdit/LanguageDefinitions.cpp"
    "${PRAXIS_DEPENDENCIES}/include/ImGuiColorTextEdit/TextEditor.cpp"
    "${PRAXIS_DEPENDENCIES}/include/ImGuiFileDialog/ImGuiFileDialog.cpp"
    "${PRAXIS_DEPENDENCIES}/include/ImGuizmo/ImGuizmo.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui/backends/imgui_impl_opengl3.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui/backends/imgui_impl_sdl2.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui/imgui.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui/imgui_demo.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui/imgui_draw.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui/imgui_stdlib.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui/imgui_tables.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui/imgui_widgets.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui_tex_inspect/imgui_tex_inspect.cpp"
    "${PRAXIS_DEPENDENCIES}/include/imgui_tex_inspect/tex_inspect_opengl.cpp")

add_executable(Praxis3D
    "${PRAXIS_ROOT}/main.cpp"
    ${PRAXIS_ENGINE_SOURCES}
    ${PRAXIS_IMGUI_SOURCES})

set_target_properties(Praxis3D PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    VS_DEBUGGER_WORKING_DIRECTORY "${PRAXIS_ROOT}"
    VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/$<CONFIG>\;${FMOD_ROOT}/bin/x64/$<CONFIG>\;%PATH%")

target_compile_definitions(Praxis3D PRIVATE
    NOMINMAX
    _CRT_SECURE_NO_DEPRECATE
    GLM_FORCE_CTOR_INIT
    SOL_LUAJIT
    PRAXIS3D_CMAKE_BUILD
    CUSTOM_IMGUIFILEDIALOG_CONFIG=\"Engine/Include/EngineDefinitions.hpp\"
    "$<$<CONFIG:Debug>:_DEBUG>"
    "$<$<CONFIG:Release>:NDEBUG>")

target_compile_options(Praxis3D PRIVATE /W3 /bigobj /MP)
target_link_options(Praxis3D PRIVATE /SUBSYSTEM:CONSOLE)

target_include_directories(Praxis3D PRIVATE
    "${PRAXIS_ROOT}/Source"
    "${PRAXIS_DEPENDENCIES}/include"
    "${PRAXIS_DEPENDENCIES}/include/Lua"
    "${PRAXIS_DEPENDENCIES}/include/imgui"
    "${PRAXIS_DEPENDENCIES}/include/imgui/backends"
    "${PRAXIS_DEPENDENCIES}/include/sdl"
    "${PRAXIS_DEPENDENCIES}/include/bullet3"
    "${PRAXIS_DEPENDENCIES}/include/imgui_tex_inspect"
    "${PRAXIS_DEPENDENCIES}/include/ImGuizmo"
    "${PRAXIS_DEPENDENCIES}/include/ImGuiColorTextEdit"
    "${PRAXIS_DEPENDENCIES}/include/ImGuiFileDialog")

target_link_libraries(Praxis3D PRIVATE
    opengl32
    "${PRAXIS_DEPENDENCIES}/x64/$<CONFIG>/lib/SDL2.lib"
    "${PRAXIS_DEPENDENCIES}/x64/$<CONFIG>/lib/SDL2main.lib"
    "${PRAXIS_DEPENDENCIES}/x64/$<CONFIG>/lib/glew32.lib"
    "${PRAXIS_DEPENDENCIES}/x64/$<CONFIG>/lib/FreeImage.lib"
    "${PRAXIS_DEPENDENCIES}/x64/$<CONFIG>/lib/lua51.lib"
    FMOD::Core
    FMOD::Studio
    $<$<CONFIG:Debug>:${PRAXIS_DEPENDENCIES}/x64/Debug/lib/assimp-vc143-mtd.lib>
    $<$<CONFIG:Release>:${PRAXIS_DEPENDENCIES}/x64/Release/lib/assimp-vc143-mt.lib>
    $<$<CONFIG:Debug>:${PRAXIS_DEPENDENCIES}/x64/Debug/lib/tbb_debug.lib>
    $<$<CONFIG:Release>:${PRAXIS_DEPENDENCIES}/x64/Release/lib/tbb.lib>)

set(PRAXIS_BULLET_LIBRARIES
    Bullet3Collision
    Bullet3Common
    Bullet3Dynamics
    Bullet3Geometry
    Bullet3OpenCL_clew
    BulletCollision
    BulletDynamics
    BulletInverseDynamics
    BulletInverseDynamicsUtils
    BulletSoftBody
    LinearMath)

foreach(bullet_library IN LISTS PRAXIS_BULLET_LIBRARIES)
    target_link_libraries(Praxis3D PRIVATE
        $<$<CONFIG:Debug>:${PRAXIS_DEPENDENCIES}/x64/Debug/lib/bullet/${bullet_library}_x64_debug.lib>
        $<$<CONFIG:Release>:${PRAXIS_DEPENDENCIES}/x64/Release/lib/Bullet/${bullet_library}_x64_release.lib>)
endforeach()

# Copy compile_commands.json to the project root so clangd and other tooling find it.
# Only Ninja (single-config) generators emit it; on other generators the file does not exist.
if(CMAKE_GENERATOR MATCHES "Ninja")
    add_custom_command(TARGET Praxis3D POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json")
endif()

# Copy every runtime DLL (prebuilt dependencies + FMOD) next to the executable. The Windows
# loader searches the executable's own directory first, so it runs from any working directory.
# Data/ is intentionally NOT copied (it can be huge) - running from the source tree working
# directory resolves it in place (VS debugger property, .vscode launch configs, "run" target).
# Full standalone deployment, including Data/, is handled by the install target.
set(PRAXIS_RUNTIME_DLLS
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/$<CONFIG>/SDL2.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/$<CONFIG>/glew32.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/$<CONFIG>/FreeImage.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/$<CONFIG>/lua51.dll"
    "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Debug/assimp-vc143-mtd.dll>"
    "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Release/assimp-vc143-mt.dll>"
    "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Debug/tbb_debug.dll>"
    "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Release/tbb.dll>"
    ${FMOD_RUNTIME_FILES})

add_custom_command(TARGET Praxis3D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PRAXIS_RUNTIME_DLLS}
        "$<TARGET_FILE_DIR:Praxis3D>"
    COMMAND_EXPAND_LISTS)

# Runs the game with its working directory set to the source tree, so Data/ is loaded in
# place instead of being copied. Config-aware: builds and runs whichever configuration the
# build preset selected (cmake --build --preset debug|release --target run)
add_custom_target(run
    COMMAND $<TARGET_FILE:Praxis3D>
    WORKING_DIRECTORY "${PRAXIS_ROOT}"
    DEPENDS Praxis3D
    VERBATIM)

install(TARGETS Praxis3D
    RUNTIME DESTINATION "."
    CONFIGURATIONS Release)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md"
    DESTINATION "."
    CONFIGURATIONS Release)

install(DIRECTORY "${PRAXIS_ROOT}/Data/"
    DESTINATION "Data"
    CONFIGURATIONS Release)

install(FILES
    "${FMOD_CORE_RELEASE_DLL}"
    "${FMOD_STUDIO_RELEASE_DLL}"
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Release/SDL2.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Release/glew32.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Release/FreeImage.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Release/lua51.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Release/assimp-vc143-mt.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/Builds/x64/Release/tbb.dll"
    DESTINATION "."
    CONFIGURATIONS Release)