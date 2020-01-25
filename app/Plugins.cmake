function(add_static_plugins_lib)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs PLUGINS)
    cmake_parse_arguments(add_static_plugins_lib "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    list(LENGTH add_static_plugins_lib_PLUGINS plugins_len)
    math(EXPR plugins_stop "${plugins_len} - 1")
    
    # collect targets
    foreach(plugin ${add_static_plugins_lib_PLUGINS})
        string(TOLOWER "${plugin}" plugin_lower)
        list(APPEND targets "kdeconnect_${plugin_lower}")
    endforeach()
    
    # collect plugins
    set(source_code_lines "#include <QtPlugin>\n")
    foreach(plugin ${add_static_plugins_lib_PLUGINS})
        list(APPEND source_code_lines "Q_IMPORT_PLUGIN(${plugin}PluginFactory)\n")
    endforeach()
    string(CONCAT source_code ${source_code_lines})

    set(source_path "${CMAKE_CURRENT_BINARY_DIR}/${add_static_plugins_lib_NAME}-import-plugins.cpp")
    file(WRITE "${source_path}" "${source_code}")

    set(dummy_path "${CMAKE_CURRENT_BINARY_DIR}/${add_static_plugins_lib_NAME}-dummy.cpp")
    file(WRITE "${dummy_path}" "")
    
    add_library("${add_static_plugins_lib_NAME}" ${dummy_path})
    target_link_libraries("${add_static_plugins_lib_NAME}" PRIVATE ${targets})
    target_sources("${add_static_plugins_lib_NAME}" INTERFACE "${source_path}")
endfunction()
