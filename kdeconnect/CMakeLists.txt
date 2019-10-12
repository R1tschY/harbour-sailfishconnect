# TODO: ship it with conan package
#
# kcoreaddons_add_plugin(plugin_name SOURCES... [JSON "pluginname.json"] [INSTALL_NAMESPACE "servicename"])
#
# This macro helps simplifying the creation of plugins for KPluginFactory
# based systems.
# It will create a plugin given the SOURCES list, the name of the JSON file
# that will define the plugin's metadata and the INSTALL_NAMESPACE so that
# the plugin is installed with the rest of the plugins from the same sub-system,
# within ${KDE_INSTALL_PLUGINDIR}.
#
# Example:
#   kcoreaddons_add_plugin(kdeconnect_share JSON kdeconnect_share.json SOURCES ${kdeconnect_share_SRCS})
#
# Since 5.10.0

function(kcoreaddons_add_plugin plugin)
    set(options)
    set(oneValueArgs JSON INSTALL_NAMESPACE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(KCA_ADD_PLUGIN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT KCA_ADD_PLUGIN_SOURCES)
      message(FATAL_ERROR "kcoreaddons_add_plugin called without SOURCES parameter")
    endif()
    get_filename_component(json "${KCA_ADD_PLUGIN_JSON}" REALPATH)

    add_library(${plugin} MODULE ${KCA_ADD_PLUGIN_SOURCES})
    set_property(TARGET ${plugin} APPEND PROPERTY AUTOGEN_TARGET_DEPENDS ${json})
    # If find_package(ECM 5.38) or higher is called, output the plugin in a INSTALL_NAMESPACE subfolder.
    # See https://community.kde.org/Guidelines_and_HOWTOs/Making_apps_run_uninstalled
    if(NOT ("${ECM_GLOBAL_FIND_VERSION}" VERSION_LESS "5.38.0"))
        set_target_properties(${plugin} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${KCA_ADD_PLUGIN_INSTALL_NAMESPACE}")
    endif()

    if (NOT KCA_ADD_PLUGIN_INSTALL_NAMESPACE)
        message(FATAL_ERROR "Must specify INSTALL_NAMESPACE for ${plugin}")
    endif()
    install(TARGETS ${plugin} DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KCA_ADD_PLUGIN_INSTALL_NAMESPACE})
endfunction()


set(SAILFISHOS ON)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(kdeconnect)