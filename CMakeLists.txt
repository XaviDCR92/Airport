cmake_minimum_required(VERSION 3.13)
if("$ENV{PSXSDK_PATH}" STREQUAL "")
    message(FATAL_ERROR "Please set PSXSDK_PATH env variable first "
        "where psxsdk root is located e.g.: /usr/local/psxsdk")
endif()
set(CMAKE_C_COMPILER psx-gcc)
set(CMAKE_CXX_COMPILER psx-g++)
project(airport C)
set(cdroot ${CMAKE_SOURCE_DIR}/cdimg)

if(NOT EXISTS ${cdroot})
    file(MAKE_DIRECTORY ${cdroot})
endif()

add_executable(${PROJECT_NAME}
    "Source/Aircraft.c"
    "Source/Camera.c"
    "Source/EndAnimation.c"
    "Source/Font.c"
    "Source/Game.c"
    "Source/GameGui.c"
    "Source/Gfx.c"
    "Source/LoadMenu.c"
    "Source/main.c"
    "Source/MainMenuBtnAni.c"
    "Source/MemCard.c"
    "Source/Menu.c"
    "Source/Message.c"
    "Source/Pad.c"
    "Source/PltParser.c"
    "Source/PSXSDKIntro.c"
    "Source/Serial.c"
    "Source/Sfx.c"
    "Source/System.c"
    "Source/Timer.c"
)
target_link_directories(${PROJECT_NAME} PUBLIC $ENV{PSXSDK_PATH}/lib)
target_compile_options(${PROJECT_NAME} PUBLIC -DFIXMATH_FAST_SIN -D_PAL_MODE_
    -DPSXSDK_DEBUG -DNO_CDDA -DNO_INTRO -Wall -g3 -Og)
target_link_libraries(${PROJECT_NAME} PUBLIC -lpsx -lfixmath)
target_include_directories(${PROJECT_NAME} PRIVATE . $ENV{PSXSDK_PATH}/include)
add_custom_target(exe ALL elf2exe ${PROJECT_NAME}
    ${cdroot}/${PROJECT_NAME}.exe -mark=\"A homebrew game created with PSXSDK\"
    DEPENDS ${PROJECT_NAME})
add_custom_target(iso ALL mkisofs -o ${PROJECT_NAME}.iso -V ${PROJECT_NAME}
    -sysid PLAYSTATION ${cdroot} DEPENDS exe)
set(license $ENV{PSXSDK_PATH}/share/licenses/infoeur.dat)
add_custom_target(bin_cue ALL mkpsxiso ${PROJECT_NAME}.iso ${PROJECT_NAME}.bin
    ${license} -s DEPENDS iso)
add_subdirectory(Levels)
add_subdirectory(Sprites)
add_subdirectory(Sounds)
file(COPY "Source/system.cnf" DESTINATION ${cdroot})
