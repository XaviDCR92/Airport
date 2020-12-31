function(vag)
    set(options "")
    set(multiValueArgs "")
    set(oneValueArgs NAME DEST)
    cmake_parse_arguments(VAG "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN})

    if(NOT EXISTS ${VAG_DEST})
        file(MAKE_DIRECTORY ${VAG_DEST})
    endif()

    add_custom_target(${VAG_NAME}_vag ALL
        wav2vag ${VAG_NAME}.wav ${VAG_DEST}/${VAG_NAME}.VAG
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${VAG_NAME}.wav
        BYPRODUCTS ${VAG_DEST}/${VAG_NAME}.VAG)
    add_dependencies(iso ${VAG_NAME}_vag)
endfunction()

vag(NAME BELL DEST ${cdroot}/DATA/SOUNDS)
vag(NAME ACCEPT DEST ${cdroot}/DATA/SOUNDS)
vag(NAME TRAYCL DEST ${cdroot}/DATA/SOUNDS)
vag(NAME SPINDISK DEST ${cdroot}/DATA/SOUNDS)
vag(NAME BEEP DEST ${cdroot}/DATA/SOUNDS)
vag(NAME TAKEOFF1 DEST ${cdroot}/DATA/SOUNDS)
vag(NAME RCTM1F1 DEST ${cdroot}/DATA/SOUNDS)
vag(NAME RCPW1A1 DEST ${cdroot}/DATA/SOUNDS)
vag(NAME RCPM1A1 DEST ${cdroot}/DATA/SOUNDS)
