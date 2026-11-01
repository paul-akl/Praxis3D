set(FMOD_ROOT "${CMAKE_SOURCE_DIR}/external/fmod" CACHE PATH "FMOD SDK root")

find_path(FMOD_INCLUDE_DIR
    NAMES fmod/fmod.hpp
    HINTS "${FMOD_ROOT}/include")

find_library(FMOD_CORE_DEBUG_LIBRARY NAMES fmod_vc HINTS "${FMOD_ROOT}/lib/x64/Debug")
find_library(FMOD_CORE_RELEASE_LIBRARY NAMES fmod_vc HINTS "${FMOD_ROOT}/lib/x64/Release")
find_library(FMOD_STUDIO_DEBUG_LIBRARY NAMES fmodstudio_vc HINTS "${FMOD_ROOT}/lib/x64/Debug")
find_library(FMOD_STUDIO_RELEASE_LIBRARY NAMES fmodstudio_vc HINTS "${FMOD_ROOT}/lib/x64/Release")
find_file(FMOD_CORE_DEBUG_DLL NAMES fmod.dll HINTS
    "${FMOD_ROOT}/bin/x64/Debug")
find_file(FMOD_CORE_RELEASE_DLL NAMES fmod.dll HINTS
    "${FMOD_ROOT}/bin/x64/Release")
find_file(FMOD_STUDIO_DEBUG_DLL NAMES fmodstudio.dll HINTS
    "${FMOD_ROOT}/bin/x64/Debug")
find_file(FMOD_STUDIO_RELEASE_DLL NAMES fmodstudio.dll HINTS
    "${FMOD_ROOT}/bin/x64/Release")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FMOD REQUIRED_VARS
    FMOD_INCLUDE_DIR
    FMOD_CORE_DEBUG_LIBRARY FMOD_CORE_RELEASE_LIBRARY
    FMOD_STUDIO_DEBUG_LIBRARY FMOD_STUDIO_RELEASE_LIBRARY
    FMOD_CORE_DEBUG_DLL FMOD_CORE_RELEASE_DLL
    FMOD_STUDIO_DEBUG_DLL FMOD_STUDIO_RELEASE_DLL
    FAIL_MESSAGE "FMOD is required but is not bundled. Install it under external/fmod with include/fmod, lib/x64/<Debug|Release>, and bin/x64/<Debug|Release>, or set FMOD_ROOT accordingly.")

if(FMOD_FOUND)
    add_library(FMOD::Core UNKNOWN IMPORTED)
    set_target_properties(FMOD::Core PROPERTIES
        IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
        IMPORTED_LOCATION_DEBUG "${FMOD_CORE_DEBUG_LIBRARY}"
        IMPORTED_LOCATION_RELEASE "${FMOD_CORE_RELEASE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}")

    add_library(FMOD::Studio UNKNOWN IMPORTED)
    set_target_properties(FMOD::Studio PROPERTIES
        IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
        IMPORTED_LOCATION_DEBUG "${FMOD_STUDIO_DEBUG_LIBRARY}"
        IMPORTED_LOCATION_RELEASE "${FMOD_STUDIO_RELEASE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}")

    set(FMOD_RUNTIME_FILES
        $<$<CONFIG:Debug>:${FMOD_CORE_DEBUG_DLL}>
        $<$<CONFIG:Debug>:${FMOD_STUDIO_DEBUG_DLL}>
        $<$<CONFIG:Release>:${FMOD_CORE_RELEASE_DLL}>
        $<$<CONFIG:Release>:${FMOD_STUDIO_RELEASE_DLL}>)
endif()