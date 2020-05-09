qt5_add_dbus_interface(mce_request_SRC com.nokia.mce.request.xml mce_request)
set_source_files_properties(com.nokia.mce.request.xml PROPERTIES
    CLASSNAME MceRequest NO_NAMESPACE ON)
set_property(SOURCE mce_request.cpp PROPERTY SKIP_AUTOMOC ON)

qt5_add_dbus_interface(mce_signal_SRC com.nokia.mce.signal.xml mce_signal)
set_source_files_properties(com.nokia.mce.signal.xml PROPERTIES
    CLASSNAME MceSignal NO_NAMESPACE ON)
foreach(item ${mce_signal_SRC})
    set_property(SOURCE ${item} PROPERTY SKIP_AUTOMOC ON)
endforeach()

kdeconnect_add_plugin(
    sailfishconnect_batteryreport
    JSON sailfishconnect_batteryreport.json 
    SOURCES batteryreportplugin.cpp ${mce_request_SRC} ${mce_signal_SRC})

target_link_libraries(sailfishconnect_batteryreport
PRIVATE
    kdeconnectcore
    Qt5::DBus
    KF5::I18n
)
