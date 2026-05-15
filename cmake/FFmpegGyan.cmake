# Generate config.h from the gyan prebuilt FFmpeg tree.
set(FFMPEG_GYAN_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ffplay_generate_config)
    set(FFMPEG_VERSION_STR "unknown")
    set(_verh "${FFMPEG_PREBUILT_ROOT}/include/libavutil/ffversion.h")
    if(EXISTS "${_verh}")
        file(STRINGS "${_verh}" _ver REGEX "define FFMPEG_VERSION")
        if(_ver MATCHES "define FFMPEG_VERSION \"([^\"]+)\"")
            set(FFMPEG_VERSION_STR "${CMAKE_MATCH_1}")
        endif()
    endif()

    string(TIMESTAMP CONFIG_THIS_YEAR "%Y")

    configure_file(
        "${FFMPEG_GYAN_MODULE_DIR}/config.h.in"
        "${FFPLAY_GEN_DIR}/config.h"
        @ONLY)
    configure_file(
        "${FFMPEG_GYAN_MODULE_DIR}/config_components.h.in"
        "${FFPLAY_GEN_DIR}/config_components.h"
        COPYONLY)
endfunction()
