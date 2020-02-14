kdeconnect_add_plugin(
    sailfishconnect_telephony
    JSON sailfishconnect_telephony.json 
    SOURCES telephonyplugin.cpp ofono.cpp)

target_link_libraries(sailfishconnect_telephony
    kdeconnectcore
    sailfishconnectpluginscommon
    KF5::I18n
    Qt5::DBus
)
