include(../kdeconnect/kdeconnect/KDEConnectMacros.cmake)

add_definitions(-DTRANSLATION_DOMAIN=\"sailfishconnect-plugins\")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(batteryreport)