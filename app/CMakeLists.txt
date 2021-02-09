add_definitions(-DTRANSLATION_DOMAIN=\"sailfishconnect-app\")

# Qt
find_package(Qt5 COMPONENTS Core Network Qml Sql Gui Quick DBus REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# libsailfishapp
include(FindPkgConfig)
pkg_search_module(SailfishApp sailfishapp REQUIRED IMPORTED_TARGET)
pkg_search_module(KeepAlive keepalive REQUIRED IMPORTED_TARGET)
pkg_search_module(NemoNotifications nemonotifications-qt5 REQUIRED IMPORTED_TARGET)

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
    RemoteKeyboard
    RemoteSystemVolume
#    Presenter
)
add_static_plugins_lib(
  NAME sailfishconnectplugins
  PREFIX sailfishconnect
  PLUGINS
    BatteryReport
    Contacts
    Clipboard
    MprisRemote
    SendNotifications
    Share
    Telepathy
    Telephony
)

# plugin clients
#generate_plugin_client(
#  sailfishconnect_PLUGIN_CLIENTS ${PROJECT_SOURCE_DIR}/plugins/sf_mprisremote/mprisremoteplugin.h mprisremotepluginclient)

# exe
set(CONFIG_SOURCE src/sailfishconnect-config.cpp)
configure_file(
    ${CONFIG_SOURCE}.in ${CONFIG_SOURCE}
    @ONLY
)
file(GLOB_RECURSE sailfishconnectapp_SOURCES "*.cpp")
FILE(GLOB_RECURSE sailfishconnectapp_QML_SOURCES "qml/*.qml")
add_executable(sailfishconnectapp 
  ${sailfishconnectapp_SOURCES}
  ${sailfishconnectapp_QML_SOURCES}
  ${sailfishconnect_PLUGIN_CLIENTS}
  ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_SOURCE})

set_target_properties(sailfishconnectapp
  PROPERTIES
  OUTPUT_NAME ${PACKAGE_NAME}
)

target_link_libraries(sailfishconnectapp
  Qt5::Core
  Qt5::Quick
  Qt5::DBus
  Qt5::Sql
  Qt5::Feedback
  KF5::CoreAddons
  KF5::I18n
  PkgConfig::SailfishApp
  PkgConfig::KeepAlive
  PkgConfig::NemoNotifications

  kdeconnectcore
  kdeconnectinterfaces
  kdeconnectplugins
  sailfishconnectplugins
  sailfishconnectpluginscommon

  # Needed by booster
  -pie
)

# Tell Qt Creator where the application executable(s) would be located on the
# device.
file(WRITE "${CMAKE_BINARY_DIR}/QtCreatorDeployment.txt"
    "${CMAKE_INSTALL_PREFIX}\n${CMAKE_BINARY_DIR}/${PACKAGE_NAME}:bin\n")

# Install
install(TARGETS sailfishconnectapp
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml
    DESTINATION share/${PACKAGE_NAME}
)
install(DIRECTORY translations
    DESTINATION share/${PACKAGE_NAME}
    FILES_MATCHING PATTERN "*.qm"
)
install(FILES ${PACKAGE_NAME}.desktop
    DESTINATION share/applications
)
install(FILES ${PACKAGE_NAME}.service
    DESTINATION share/${PACKAGE_NAME}
)
install(FILES icons/86x86/${PACKAGE_NAME}.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/${PACKAGE_NAME}.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/${PACKAGE_NAME}.png
    DESTINATION share/icons/hicolor/128x128/apps
)
