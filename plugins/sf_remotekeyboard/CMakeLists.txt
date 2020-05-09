kdeconnect_add_plugin(
    sailfishconnect_remotekeyboard
    JSON sailfishconnect_remotekeyboard.json 
    SOURCES remotekeyboardplugin.cpp)

target_link_libraries(sailfishconnect_remotekeyboard
PRIVATE
    kdeconnectcore
    KF5::I18n
)
