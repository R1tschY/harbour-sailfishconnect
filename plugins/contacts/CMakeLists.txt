kdeconnect_add_plugin(
    sailfishconnect_contacts
    JSON sailfishconnect_contacts.json 
    SOURCES contactsplugin.cpp)

target_link_libraries(sailfishconnect_contacts
    kdeconnectcore
    sailfishconnectpluginscommon
    KF5::I18n
)
target_include_directories(sailfishconnect_contacts
    SYSTEM PRIVATE ${ContextkitStatefs_INCLUDE_DIRS}
)
