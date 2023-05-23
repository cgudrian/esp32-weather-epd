cmake_minimum_required(VERSION 3.16)

project(WeatherStation VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set(PIO_ROOT ../platformio)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Network)

add_library(owa-icons STATIC ${PIO_ROOT}/lib/owa-icons/owa-icons.cpp)
target_include_directories(owa-icons INTERFACE ${PIO_ROOT}/lib/owa-icons)

add_library(aqi STATIC ${PIO_ROOT}/lib/pollutant-concentration-to-aqi/aqi.c)
target_include_directories(aqi INTERFACE ${PIO_ROOT}/lib/pollutant-concentration-to-aqi)

qt_standard_project_setup()

qt_add_executable(appWeatherStation
    main.cpp
    display.h display.cpp
    displayimageprovider.h displayimageprovider.cpp
    adafruitfont.cpp
    ${PIO_ROOT}/src/locales/locale_de_DE.cpp
    ${PIO_ROOT}/src/conversions.cpp
    ${PIO_ROOT}/src/_strftime.cpp
    ${PIO_ROOT}/src/display_utils.cpp
    ${PIO_ROOT}/src/renderer.cpp
    ${PIO_ROOT}/src/config.cpp
)

qt_add_qml_module(appWeatherStation
    URI WeatherStation
    VERSION 1.0
    QML_FILES Main.qml
)

set_target_properties(appWeatherStation PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(appWeatherStation
    PRIVATE
    ${PIO_ROOT}/include/fonts
    ${PIO_ROOT}/include/icons
    ${PIO_ROOT}/include
)

target_link_libraries(appWeatherStation
    PRIVATE
    Qt6::Quick
    Qt6::Network
    owa-icons
    aqi
)

install(TARGETS appWeatherStation
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
