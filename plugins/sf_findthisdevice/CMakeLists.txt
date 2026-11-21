kdeconnect_add_plugin(
    sailfishconnect_findthisdevice
    JSON sailfishconnect_findthisdevice.json
    SOURCES findthisdeviceplugin.cpp)

find_package(PkgConfig REQUIRED)
pkg_search_module(NemoNotifications nemonotifications-qt5 REQUIRED IMPORTED_TARGET)

target_link_libraries(sailfishconnect_findthisdevice
PRIVATE
    kdeconnectcore
    KF5::I18n
    Qt5::DBus
    PkgConfig::NemoNotifications
)
