cmake_minimum_required(VERSION 4.0)
project(pwscp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")

find_package(Qt6 COMPONENTS
             Core
             Gui
             Widgets
             REQUIRED)

find_package(ALSA REQUIRED)

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

target_include_directories(${PROJECT_NAME} PRIVATE ${ALSA_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
                      Qt::Core
                      Qt::Gui
                      Qt::Widgets
                      ALSA::ALSA
)

# warnings for debug
target_compile_options(${PROJECT_NAME} PRIVATE
                       -Wall
                       -Wextra
                       -Wpedantic
                       -Wuninitialized
                       -Wmisleading-indentation
)

# Add sanitizers for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE
                           -fsanitize=address
                           -fsanitize=undefined
    )
    target_link_options(${PROJECT_NAME} PRIVATE
                        -fsanitize=address
                        -fsanitize=undefined
    )
endif()