kdeconnect_add_plugin(
    sailfishconnect_sftpserver
    JSON sailfishconnect_sftpserver.json 
    SOURCES sftpserverplugin.cpp sftpserver.cpp sftpservercommands.cpp)

target_link_libraries(sailfishconnect_sftpserver
PRIVATE
    kdeconnectcore
    KF5::I18n
    ssh::static
)
target_include_directories(sailfishconnect_sftpserver
    SYSTEM PRIVATE ${ContextkitStatefs_INCLUDE_DIRS}
)
