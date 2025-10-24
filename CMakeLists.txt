cmake_minimum_required(VERSION 4.0)
project(pwscp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(PkgConfig REQUIRED)

find_package(Qt6 COMPONENTS
             Core
             Gui
             Widgets
             REQUIRED)

pkg_check_modules(ALSA REQUIRED alsa)

add_executable(${PROJECT_NAME} src/main.cpp
               src/ui/mainWindow.cpp
               src/ui/mainWindow.h
               src/appStrings/AppStrings.cpp
               src/appStrings/AppStrings.h
               src/ui/typo.h
               src/audioAPI/alsa.cpp
               src/audioAPI/alsa.h
               src/audioAPI/soundcard.cpp
)

if(alsa_FOUND)
endif()

if (ALSA_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${ALSA_INCLUDE_DIRS})

    #debug prints
    message(STATUS "ALSA include dirs: ${ALSA_INCLUDE_DIRS}")
    message(STATUS "ALSA libs: ${ALSA_LIBRARIES}")
endif()

target_link_libraries(${PROJECT_NAME}
                      Qt::Core
                      Qt::Gui
                      Qt::Widgets
                      ${ALSA_LIBRARIES}
)

# warnings for debug
target_compile_options(${PROJECT_NAME} PRIVATE
                       $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic>
)