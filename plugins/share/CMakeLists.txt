kdeconnect_add_plugin(
    sailfishconnect_share
    JSON sailfishconnect_share.json 
    SOURCES shareplugin.cpp copyjob.cpp downloadjob.cpp filehelper.cpp)

find_package(Qt5 REQUIRED COMPONENTS Gui)

target_link_libraries(sailfishconnect_share
    kdeconnectcore
    KF5::I18n
    Qt5::Gui
)  