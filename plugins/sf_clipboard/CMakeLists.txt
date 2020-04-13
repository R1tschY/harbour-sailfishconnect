kdeconnect_add_plugin(
    sailfishconnect_clipboard
    JSON sailfishconnect_clipboard.json 
    SOURCES clipboardplugin.cpp)

find_package(Qt5 REQUIRED COMPONENTS Gui)

target_link_libraries(sailfishconnect_clipboard
PRIVATE
    kdeconnectcore
    Qt5::Gui
    KF5::I18n
)
