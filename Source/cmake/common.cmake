function(configure_webview2_target target_name)
    if (IU_ENABLE_WEBVIEW2)
        set_target_properties(${target_name} PROPERTIES 
            VS_GLOBAL_WebView2LoaderPreference "Static")
    endif()
endfunction()

function(configure_nuget_packages target_name)
	set_target_properties(${target_name} PROPERTIES 
            VS_USER_PROPS "${CMAKE_SOURCE_DIR}/App.Core.props")
    set_target_properties(${target_name} PROPERTIES 
            VS_PACKAGE_REFERENCES "${NUGET_PACKAGES_REFERENCES}")
endfunction()
			