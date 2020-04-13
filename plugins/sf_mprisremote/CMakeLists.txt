kdeconnect_add_plugin(
    sailfishconnect_mprisremote
    JSON sailfishconnect_mprisremote.json 
    SOURCES mprisremoteplugin.cpp albumartcache.cpp)

target_link_libraries(sailfishconnect_mprisremote
PRIVATE
    sailfishconnectpluginscommon
    kdeconnectcore
    Qt5::Core
    Qt5::Quick
    KF5::I18n
)
