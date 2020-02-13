function(add_static_plugins_lib)
    set(options)
    set(oneValueArgs NAME PREFIX)
    set(multiValueArgs PLUGINS)
    cmake_parse_arguments(add_static_plugins_lib "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    list(LENGTH add_static_plugins_lib_PLUGINS plugins_len)
    math(EXPR plugins_stop "${plugins_len} - 1")
    set(prefix "${add_static_plugins_lib_PREFIX}")
    
    # collect targets
    foreach(plugin ${add_static_plugins_lib_PLUGINS})
        string(TOLOWER "${plugin}" plugin_lower)
        list(APPEND targets "${prefix}_${plugin_lower}")
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
    
    add_library("${add_static_plugins_lib_NAME}" STATIC ${dummy_path})
    target_link_libraries("${add_static_plugins_lib_NAME}" PRIVATE ${targets})
    target_sources("${add_static_plugins_lib_NAME}" INTERFACE "${source_path}")
endfunction()


function(generate_plugin_client src_var source_h output_h)
    set(xml_file ${CMAKE_CURRENT_BINARY_DIR}/${output_h}.xml)
    qt5_generate_dbus_interface(${source_h} ${xml_file})

    set_source_files_properties(${xml_file} PROPERTIES NO_NAMESPACE true)

    qt5_add_dbus_interface(_source ${xml_file} ${output_h})
    set_property(SOURCE ${_source} PROPERTY SKIP_AUTOMOC ON)
    set_property(SOURCE ${output_h} PROPERTY SKIP_AUTOMOC ON)
    set(${src_var} ${${src_var}} ${_source} PARENT_SCOPE)
endfunction()