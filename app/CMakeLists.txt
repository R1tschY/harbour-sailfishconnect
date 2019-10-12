set(OUTPUT_NAME harbour-sailfishconnect)

# Qt
find_package(Qt5 COMPONENTS Core Network Qml Gui Quick DBus REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# libsailfishapp
include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)

# exe
file(GLOB_RECURSE SailfishConnectApp_SOURCES "*.cpp")
add_executable(SailfishConnectApp ${SailfishConnectApp_SOURCES})

set_target_properties(SailfishConnectApp
  PROPERTIES
  OUTPUT_NAME ${OUTPUT_NAME}
)

target_include_directories(SailfishConnectApp 
  PRIVATE ${SAILFISH_INCLUDE_DIRS}
)

target_link_libraries(SailfishConnectApp
  Qt5::Core
  Qt5::Quick
  Qt5::DBus
  kdeconnectcore
  ${SAILFISH_LDFLAGS}
)

# Install
install(TARGETS SailfishConnectApp
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml
    DESTINATION share/${OUTPUT_NAME}
)
install(DIRECTORY translations
    DESTINATION share/${OUTPUT_NAME}
    FILES_MATCHING PATTERN "*.qm"
)
install(FILES ${OUTPUT_NAME}.desktop
    DESTINATION share/applications
)
install(FILES icons/86x86/${OUTPUT_NAME}.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/${OUTPUT_NAME}.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/${OUTPUT_NAME}.png
    DESTINATION share/icons/hicolor/128x128/apps
)
