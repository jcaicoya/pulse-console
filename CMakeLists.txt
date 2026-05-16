cmake_minimum_required(VERSION 3.28)
project(under_attack_pulse_console)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.7.3/msvc2022_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

# yaml-cpp: build directly from source, bypassing its own CMakeLists.txt.
# yaml-cpp 0.8.0 uses cmake_minimum_required(VERSION 2.8.12) which CMake 4.x
# rejects. We clone the sources once and compile them manually.
set(_yaml_src "${CMAKE_BINARY_DIR}/_deps/yaml_cpp-src")

if (NOT EXISTS "${_yaml_src}/include/yaml-cpp/yaml.h")
    message(STATUS "Fetching yaml-cpp 0.8.0...")
    execute_process(
        COMMAND git clone --depth 1 --branch 0.8.0
                https://github.com/jbeder/yaml-cpp.git "${_yaml_src}"
        RESULT_VARIABLE _yaml_clone_result
        OUTPUT_QUIET ERROR_QUIET
    )
    if (NOT _yaml_clone_result EQUAL 0)
        message(FATAL_ERROR "Failed to clone yaml-cpp. Make sure git is on PATH.")
    endif()
endif()

file(GLOB_RECURSE _yaml_sources "${_yaml_src}/src/*.cpp")
add_library(yaml-cpp STATIC ${_yaml_sources})
target_include_directories(yaml-cpp PUBLIC "${_yaml_src}/include")
target_compile_definitions(yaml-cpp PUBLIC YAML_CPP_STATIC_DEFINE)
add_library(yaml-cpp::yaml-cpp ALIAS yaml-cpp)

add_executable(under_attack_pulse_console
        src/main.cpp
        src/CyberAppMode.cpp
        src/ui/ConsoleWidget.cpp
        src/engine/ScriptLoader.cpp
        src/engine/Engine.cpp
        resources/pulseconsole.qrc
)

target_include_directories(under_attack_pulse_console
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(under_attack_pulse_console
        Qt::Core
        Qt::Gui
        Qt::Widgets
        yaml-cpp::yaml-cpp
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
