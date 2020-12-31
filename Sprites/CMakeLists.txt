function(tim)
    set(options "")
    set(multiValueArgs "")
    set(oneValueArgs NAME DEST EXT)
    cmake_parse_arguments(TIM "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN})
    file(READ ${TIM_NAME}.flags flags)

    # Avoid CMake from introducing escape characters on
    # each whitespace inside flags.
    separate_arguments(flags NATIVE_COMMAND ${flags})

    if(NOT EXISTS ${TIM_DEST})
        file(MAKE_DIRECTORY ${TIM_DEST})
    endif()

    add_custom_target(${TIM_NAME}_tim ALL
        bmp2tim ${TIM_NAME}.bmp ${TIM_DEST}/${TIM_NAME}.${TIM_EXT} ${flags}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${TIM_NAME}.bmp
        BYPRODUCTS ${TIM_DEST}/${TIM_NAME}.${TIM_EXT})
    add_dependencies(iso ${TIM_NAME}_tim)
endfunction()

tim(NAME PSXDISK DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME TILESET1 DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME TILESET2 DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME MAINMENU DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME LOADING DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME PLANE DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME BUTTONS DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME GAMEPLN DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME PLNBLUE DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME BUBBLE DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME GPL DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME OPENSRC DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME MOUSE DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME DEPARR DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME PAGEUPDN DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME BLDNGS1 DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME LFRARROW DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME UDNARROW DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME BCNGW DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME MENUSTAR DEST ${cdroot}/DATA/SPRITES EXT TIM)
tim(NAME INTROFNT DEST ${cdroot}/DATA/FONTS EXT TIM)

tim(NAME FONT_1 DEST ${cdroot}/DATA/FONTS EXT FNT)
tim(NAME FONT_2 DEST ${cdroot}/DATA/FONTS EXT FNT)
