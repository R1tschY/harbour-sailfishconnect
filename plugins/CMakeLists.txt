include(../kdeconnect/kdeconnect/KDEConnectMacros.cmake)

add_definitions(-DTRANSLATION_DOMAIN=\"sailfishconnect-plugins\")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(batteryreport)
add_subdirectory(clipboard)
add_subdirectory(contacts)
add_subdirectory(share)
add_subdirectory(telepathy)
add_subdirectory(telephony)