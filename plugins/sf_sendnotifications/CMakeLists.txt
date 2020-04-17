kdeconnect_add_plugin(
    sailfishconnect_sendnotifications
    JSON sailfishconnect_sendnotifications.json 
    SOURCES sendnotificationsplugin.cpp notifyingapplication.cpp notificationslistener.cpp)

find_package(PkgConfig QUIET REQUIRED)
pkg_check_modules(DBus REQUIRED dbus-1)

target_link_libraries(sailfishconnect_sendnotifications
PRIVATE
    kdeconnectcore
    KF5::I18n
    ${DBus_LIBRARIES}
    Qt5::DBus
    Qt5::Gui
    Qt5::Quick
)
target_include_directories(sailfishconnect_sendnotifications
    SYSTEM PRIVATE ${DBus_INCLUDE_DIRS}
)
