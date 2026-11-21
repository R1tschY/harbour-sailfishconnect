kdeconnect_add_plugin(
    sailfishconnect_sailfishsms
    JSON sailfishconnect_sms.json
    SOURCES sailfishsmsplugin.cpp)

find_package(Qt5 REQUIRED COMPONENTS Sql)

target_link_libraries(sailfishconnect_sailfishsms
PRIVATE
    kdeconnectcore
    KF5::I18n
    Qt5::Sql
    Qt5::DBus
)
