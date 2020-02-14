kdeconnect_add_plugin(
    sailfishconnect_telepathy
    JSON sailfishconnect_telepathy.json 
    SOURCES telepathyplugin.cpp)

target_link_libraries(sailfishconnect_telepathy
    kdeconnectcore
    KF5::I18n
    Qt5::DBus
)
