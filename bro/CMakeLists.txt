
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if ( BRO_DIST )
    include(DefaultCompilerSettings)

    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${BRO_DIST}/cmake)
    set(BRO_PLUGIN_BASE "${CMAKE_CURRENT_SOURCE_DIR}")

    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
    include_directories(${LLVM_INCLUDE_DIR})

    # Additional C{,XX}FLAGS for compiling with Bro's headers.
    set(addl_c_flags "-Wno-variadic-macros -Wno-extra-semi -Wno-inconsistent-missing-override")

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${addl_c_flags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${addl_c_flags}")

    if ( BRO_PLUGIN_ENABLE_DEBUG )
	    set(CMAKE_BUILD_TYPE "Debug")
        set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DDEBUG")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
	endif()

    if ( "${CMAKE_BUILD_TYPE}" MATCHES "Debug" )
	    # Bro's CMakeConfig checks this and adjusts CMAKE_BUILD_TYPE
		# if this is not set.
        set(ENABLE_DEBUG true)
	endif ()

    set(autogen "${CMAKE_CURRENT_BINARY_DIR}/autogen")
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${autogen})

    set(spicyc "${CMAKE_BINARY_DIR}/tools/spicyc")
    set(hiltic "${CMAKE_BINARY_DIR}/tools/hiltic")
    set(hiltip "${CMAKE_BINARY_DIR}/tools/hiltip")

    include(BroPlugin)

    bro_plugin_begin(Bro Hilti)
    bro_plugin_cc(src/Converter.cc)
    bro_plugin_cc(src/LocalReporter.cc)
    bro_plugin_cc(src/Manager.cc)
    bro_plugin_cc(src/SpicyAST.cc)
    bro_plugin_cc(src/SpicyAnalyzer.cc)
    bro_plugin_cc(src/SpicyFileAnalyzer.cc)
    bro_plugin_cc(src/Plugin.cc)
    bro_plugin_cc(src/Runtime.cc)
    bro_plugin_cc(src/RuntimeInterface.cc)
    bro_plugin_cc(src/compiler/BuilderBase.cc)
    bro_plugin_cc(src/compiler/Compiler.cc)
    bro_plugin_cc(src/compiler/ModuleBuilder.cc)
    bro_plugin_cc(src/compiler/StatementBuilder.cc)
    bro_plugin_cc(src/compiler/ExpressionBuilder.cc)
    bro_plugin_cc(src/compiler/TypeBuilder.cc)
    bro_plugin_cc(src/compiler/ValueBuilder.cc)
    bro_plugin_cc(src/compiler/ConversionBuilder.cc)
    bro_plugin_cc(src/compiler/ASTDumper.cc)
    bro_plugin_cc(src/compiler/Bif.cc)
    bro_plugin_bif(src/consts.bif)
    bro_plugin_bif(src/events.bif)
    bro_plugin_bif(src/functions.bif)
    bro_plugin_obj(${autogen}/bro.spicy.h)
    bro_plugin_obj(${autogen}/libbro.hlt.h)
    bro_plugin_obj(${autogen}/libbro.hlt.o)
    bro_plugin_end()

    # Autogenerate bro.spicy C prototypes.
    add_custom_command(
        OUTPUT   ${autogen}/bro.spicy.h
        COMMAND  ${spicyc} -P ${CMAKE_CURRENT_SOURCE_DIR}/spicy/bro.spicy -o ${autogen}/bro.spicy.h
        DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/spicy/bro.spicy spicy.hlt ${spicyc}
        )

    # Autogenerate libbro.hlt C prototypes.
    add_custom_command(
        OUTPUT   ${autogen}/libbro.hlt.h
        COMMAND  ${hiltip} -P ${CMAKE_CURRENT_SOURCE_DIR}/spicy/libbro.hlt -o ${autogen}/libbro.hlt.h
        DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/spicy/libbro.hlt ${hiltip}
        )

    # Autogenerate the type information for libbro.hlt.
    add_custom_command(
        OUTPUT   ${autogen}/libbro.hlt.o
        COMMAND  ${hiltic} -b ${CMAKE_CURRENT_SOURCE_DIR}/spicy/libbro.hlt -C -o ${autogen}/libbro.hlt.bc && llc -relocation-model=pic -filetype=obj -o ${autogen}/libbro.hlt.o ${autogen}/libbro.hlt.bc
        DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/spicy/libbro.hlt ${hiltic}
        )

    add_custom_target(generate_libbro DEPENDS ${autogen}/bro.spicy.h ${autogen}/libbro.hlt.h ${autogen}/libbro.hlt.o) 

    set_source_files_properties(${autogen}/bro.spicy.h   PROPERTIES GENERATED 1)
    set_source_files_properties(${autogen}/libbro.hlt.h PROPERTIES GENERATED 1)
    set_source_files_properties(${autogen}/libbro.hlt.o PROPERTIES GENERATED 1)

    if ( "${CMAKE_BUILD_TYPE}" MATCHES "Debug" )
        set(hilti_runtime  hilti-rt-dbg)
        set(spicy_runtime spicy-rt-dbg)
    else ()
        set(hilti_runtime  hilti-rt)
        set(spicy_runtime spicy-rt)
    endif ()

    add_dependencies(${_plugin_lib} generate_libbro)
    target_link_libraries(${_plugin_lib} hilti ${hilti_runtime})
    target_link_libraries(${_plugin_lib} spicy ${spicy_runtime})

    add_custom_command(TARGET ${_plugin_lib} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/spicy ${CMAKE_CURRENT_BINARY_DIR}/spicy)

                       # If we copy, changes to {scripts,spicy} won't reflected until the plugin gets relinked.
					   # COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/scripts && mkdir ${CMAKE_CURRENT_BINARY_DIR}/scripts
					   # COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/spicy && mkdir ${CMAKE_CURRENT_BINARY_DIR}/spicy
                       # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/scripts ${CMAKE_CURRENT_BINARY_DIR}/scripts
                       # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/spicy    ${CMAKE_CURRENT_BINARY_DIR}/spicy


else ()
    message(STATUS "Not building Bro plugin")
endif ()
