kdeconnect_add_plugin(
    sailfishconnect_batteryreport
    JSON sailfishconnect_batteryreport.json 
    SOURCES batteryreportplugin.cpp)

find_package(PkgConfig QUIET REQUIRED)
pkg_check_modules(ContextkitStatefs REQUIRED contextkit-statefs)

target_link_libraries(sailfishconnect_batteryreport
PRIVATE
    kdeconnectcore
    ${ContextkitStatefs_LIBRARIES}
    KF5::I18n
)
target_include_directories(sailfishconnect_batteryreport
    SYSTEM PRIVATE ${ContextkitStatefs_INCLUDE_DIRS}
)
