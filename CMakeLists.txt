cmake_minimum_required(VERSION 3.21)

project(Praxis3D VERSION 0.1.0 LANGUAGES CXX RC)

if(NOT WIN32)
    message(FATAL_ERROR "Praxis3D currently supports Windows only.")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "Praxis3D requires an MSVC toolchain because its bundled dependencies are MSVC libraries.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FMOD REQUIRED)

set(PRAXIS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Praxis3D")
set(PRAXIS_DEPENDENCIES "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies")

file(GLOB_RECURSE PRAXIS_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${PRAXIS_ROOT}/Source/*.cpp")
list(REMOVE_ITEM PRAXIS_ENGINE_SOURCES "${PRAXIS_ROOT}/Source/main.cpp")

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
    "${PRAXIS_ROOT}/Praxis3D1.rc"
    ${PRAXIS_ENGINE_SOURCES}
    ${PRAXIS_IMGUI_SOURCES})

set_target_properties(Praxis3D PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    VS_DEBUGGER_WORKING_DIRECTORY "${PRAXIS_ROOT}")

target_compile_definitions(Praxis3D PRIVATE
    NOMINMAX
    _CONSOLE
    _CRT_SECURE_NO_DEPRECATE
    GLM_FORCE_CTOR_INIT
    PRAXIS3D_CMAKE_BUILD
    CUSTOM_IMGUIFILEDIALOG_CONFIG=\"Engine/Include/EngineDefinitions.hpp\"
    $<$<CONFIG:Debug>:_DEBUG;SOL_LUAJIT>
    $<$<CONFIG:Release>:NDEBUG;_RELEASE>)

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
    "${PRAXIS_DEPENDENCIES}/x64/$<CONFIG>/lib/luajit.lib"
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

add_custom_command(TARGET Praxis3D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FMOD_RUNTIME_FILES}
        "$<TARGET_FILE_DIR:Praxis3D>"
    COMMAND_EXPAND_LISTS)

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
    DESTINATION "."
    CONFIGURATIONS Release)