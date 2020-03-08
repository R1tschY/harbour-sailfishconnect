include(../kdeconnect-kde/KDEConnectMacros.cmake)

add_definitions(-DTRANSLATION_DOMAIN=\"sailfishconnect-plugins\")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(batteryreport)
add_subdirectory(contacts)
add_subdirectory(telepathy)
add_subdirectory(telephony)
#add_subdirectory(sendnotifications)

add_subdirectory(sf_share)
add_subdirectory(sf_clipboard)
#add_subdirectory(sf_mprisremote)