set(OUTPUT_NAME harbour-sailfishconnect)

# Qt
find_package(Qt5 COMPONENTS Core Network Qml Sql Gui Quick DBus REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# libsailfishapp
include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)

# plugins
include(Plugins.cmake)
add_static_plugins_lib(
  NAME kdeconnectplugins
  PREFIX kdeconnect
  PLUGINS
    Ping
    RemoteControl
    RemoteCommands
    RunCommand
#    Battery
#    RemoteSystemVolume
#    Presenter
)
add_static_plugins_lib(
  NAME sailfishconnectplugins
  PREFIX sailfishconnect
  PLUGINS
    BatteryReport
    Clipboard
    Contacts
    Share
    Telepathy
    Telephony
)

# plugin clients
#generate_plugin_client(
#  sailfishconnect_PLUGIN_CLIENTS ${PROJECT_SOURCE_DIR}/plugins/clipboard/clipboardplugin.h clipboardpluginclient)

# exe
set(CONFIG_SOURCE src/sailfishconnect-config.cpp)
configure_file(
    ${CONFIG_SOURCE}.in ${CONFIG_SOURCE}
    @ONLY
)
file(GLOB_RECURSE sailfishconnectapp_SOURCES "*.cpp")
add_executable(sailfishconnectapp 
  ${sailfishconnectapp_SOURCES}
  ${sailfishconnect_PLUGIN_CLIENTS}
  ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_SOURCE})

set_target_properties(sailfishconnectapp
  PROPERTIES
  OUTPUT_NAME ${OUTPUT_NAME}
)

target_include_directories(sailfishconnectapp 
  PRIVATE ${SAILFISH_INCLUDE_DIRS}
)

target_link_libraries(sailfishconnectapp
  Qt5::Core
  Qt5::Quick
  Qt5::DBus
  Qt5::Sql
  ${SAILFISH_LDFLAGS}

  kdeconnectcore
  kdeconnectinterfaces
  kdeconnectplugins
  sailfishconnectplugins
)

# Install
install(TARGETS sailfishconnectapp
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml
    DESTINATION share/${OUTPUT_NAME}
)
install(DIRECTORY translations
    DESTINATION share/${OUTPUT_NAME}
    FILES_MATCHING PATTERN "*.qm"
)
install(FILES ${OUTPUT_NAME}.desktop
    DESTINATION share/applications
)
install(FILES icons/86x86/${OUTPUT_NAME}.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/${OUTPUT_NAME}.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/${OUTPUT_NAME}.png
    DESTINATION share/icons/hicolor/128x128/apps
)
