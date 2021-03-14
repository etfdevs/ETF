cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
project(etf VERSION 2.0 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)

# Set a default build type if none was specified
set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo")
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    # Ensure debug symbols in backtrace
    link_libraries("-rdynamic")
endif()

if (NOT CMAKE_SYSTEM_PROCESSOR)
	message(FATAL_ERROR "No target CPU architecture set")
endif()

if (NOT CMAKE_SYSTEM_NAME)
	message(FATAL_ERROR "No target OS set")
endif()

Include(CheckTypeSize)
check_type_size("void*" CMAKE_SIZEOF_VOID_P)

set(X86 OFF)
set(X86_64 OFF)
set(ARM32 OFF)
set(ARM64 OFF)

if(WIN32)
	set(LIB_SUFFIX "_mp_")
	set(X86 ON)
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(Architecture "x86_64")
		set(WIN64 TRUE)
		set(X86_64 ON)
	else()
		set(Architecture "x86")
		set(WIN64 FALSE)
	endif()
	set(DLL_EXT ".dll")
else()
	if(APPLE)
		set(LIB_SUFFIX "_mac")
	else()
		set(LIB_SUFFIX ".mp.")
	endif()

    if(APPLE)
        set(DLL_EXT ".dylib")
    elseif(NOT WIN32)
        set(DLL_EXT ".so")
    else()
        set(DLL_EXT ".dll")
    endif()

    set(X86 OFF)
    set(X86_64 OFF)
    set(ARM32 OFF)
    set(ARM64 OFF)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
        set(ARM32 ON)
        set(Architecture "arm")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
        set(ARM64 ON)
        set(Architecture "aarch64")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
		set(X86 ON)
#		if(APPLE)
#			set(Architecture "x86")
#		else()
			# e.g. Linux
			set(Architecture "i386")
#		endif()
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^x86.64$")
        #set(X86 ON)
        set(X86_64 ON)
		set(Architecture "x86_64")
	elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc")
		set(Architecture "ppc")
	elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc64")
		set(Architecture "ppc64")
	else()
		set(Architecture "${CMAKE_SYSTEM_PROCESSOR}")
	endif()
endif()

message("Configured for ${CMAKE_SYSTEM_PROCESSOR} / ${Architecture}")

# target os
if (APPLE)
	set(os "macosx")
else()
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" os)
endif()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckFunctionExists)

option(CGAME			"Build the cgame dll code" ON)
option(GAME				"Build the qagame dll code" ON)
option(UI				"Build the ui dll code" ON)
option(BOTS				"Enable bot support" OFF)

# target cpu
#set(cpu ${CMAKE_SYSTEM_PROCESSOR})
#if (cpu STREQUAL "powerpc")
#	set(cpu "ppc")
#elseif (cpu MATCHES "i.86")
#	set(cpu "x86")
#endif()

#if (cpu STREQUAL "x86_64" AND X86)
#	set(cpu "x86")
#endif()

# game library suffix
#set(gamesuffix ${cpu})

# compiler specific flags

#set(nix_compiler_options -pipe -Wall -fno-strict-aliasing -Wno-sign-compare -Wno-switch -Wno-format-security)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
	CHECK_C_COMPILER_FLAG("-fvisibility=hidden" c_has_fvisibility)
	if (NOT c_has_fvisibility)
		message(FATAL_ERROR "Compiler does not support -fvisibility")
	endif()
	if(APPLE AND X86_64)
		set(compiler_flags_global -pipe -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-missing-field-initializers -fPIC -fno-strict-aliasing -fvisibility=hidden)
	elseif(X86_64)
		set(compiler_flags_global -pipe -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-old-style-declaration -Wno-missing-field-initializers -fPIC -fno-strict-aliasing -fvisibility=hidden)
	else()
		set(compiler_flags_global -pipe -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-old-style-declaration -Wno-missing-field-initializers -fPIC -fno-strict-aliasing -fvisibility=hidden -march=pentium -mtune=pentiumpro)
	endif()
	# debug: explicit -Og -D_DEBUG, implicit -g
	set(compiler_flags_debug ${compiler_flags_global} -Og -D_DEBUG)
	# relwithdebinfo: implicit -O2 -g -DNDEBUG
	set(compiler_flags_relwithdebinfo ${compiler_flags_global})
	# release: implicit -O3 -DNDEBUG
	if(APPLE AND X86_64)
		set(compiler_flags_release ${compiler_flags_global} -Winline -ffast-math -fomit-frame-pointer -finline-functions -fno-unsafe-math-optimizations)
	elseif(X86_64)
	set(compiler_flags_release ${compiler_flags_global} -Winline -ffast-math -fomit-frame-pointer -finline-functions -fschedule-insns2 -fno-unsafe-math-optimizations -fstrength-reduce)
	else()
		set(compiler_flags_release ${compiler_flags_global} -Winline -ffast-math -fomit-frame-pointer -finline-functions -fschedule-insns2 -fno-unsafe-math-optimizations -fstrength-reduce)
	endif()
else()
message(FATAL_ERROR "Compiler flags not setup for this project")
endif()

#if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
#	#add_definitions(-pipe)
#	add_definitions(-Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-old-style-declaration -Wno-missing-field-initializers)
#
#	if (NOT APPLE AND Architecture STREQUAL "x86")
#		add_definitions(-march=pentium -mtune=pentiumpro)
#	elseif (NOT CMAKE_CROSSCOMPILING AND ONATIVE)
#		add_definitions(-march=native)
#	endif()
#
#	add_definitions(-fPIC)
#
#	set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -D_DEBUG")
#	set(CMAKE_C_FLAGS_DEBUGALL "-g -ggdb -D_DEBUG")
#	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer -fstrength-reduce")
#	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer -fstrength-reduce")
#	set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer -fstrength-reduce")
#
#	add_definitions(-fno-strict-aliasing)
#
#	CHECK_C_COMPILER_FLAG("-fvisibility=hidden" c_has_fvisibility)
#	if (NOT c_has_fvisibility)
#		message(FATAL_ERROR "Compiler does not support -fvisibility")
#	endif()
#	add_definitions(-fvisibility=hidden)
#
#	# TODO fix these warnings
#	#add_definitions(-Wno-sign-compare)
#	#add_definitions(-Wno-switch)
#	#add_definitions(-Wno-format-security)
#
#	#CHECK_CXX_COMPILER_FLAG("-Woverloaded-virtual" cxx_has_Woverload_virtual)
#	#if (cxx_has_Woverload_virtual)
#	#	add_definitions(-Woverloaded-virtual)
#	#endif()
#
#	if (APPLE)
#		# no suffix for .dylib
#		#set(gamesuffix "")
#
#		add_definitions(-DMACOS_X=1)
#
#		#if (Architecture STREQUAL "x86_64")
#		#	add_definitions(-arch x86_64 -mmacosx-version-min=10.6)
#		#	set(ldflags "${ldflags} -arch x86_64 -mmacosx-version-min=10.6")
#		#elseif (Architecture STREQUAL "x86")
#		if (Architecture STREQUAL "x86")
#			CHECK_CXX_COMPILER_FLAG("-arch i386" cxx_has_arch_i386)
#			if (cxx_has_arch_i386)
#				add_definitions(-arch i386)
#				set(ldflags "${ldflags} -arch i386")
#			endif()
#
#			add_definitions(-mmacosx-version-min=10.4)
#			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
#		elseif (Architecture STREQUAL "ppc")
#			CHECK_CXX_COMPILER_FLAG("-arch ppc" cxx_has_arch_ppc)
#			if (cxx_has_arch_ppc)
#				add_definitions(-arch ppc)
#				set(ldflags "${ldflags} -arch ppc")
#			endif()
#
#			add_definitions(-mmacosx-version-min=10.4)
#			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
#		else()
#			message(FATAL_ERROR "Unsupported CPU architecture for OSX")
#		endif()
#
#		set(sys_libs ${sys_libs} "-framework Carbon -framework Cocoa -framework OpenGL -framework IOKit")
#	else()
#		#if ((Architecture STREQUAL "x86" OR Architecture STREQUAL "i386") AND X86)
#		#	add_definitions(-m32)
#		#	set(ldflags "${ldflags} -m32")
#		#endif()
#
#		if (os STREQUAL "linux")
#			set(sys_libs ${sys_libs} dl)
#		endif()
#	endif()
#elseif (MSVC)
#	add_definitions(/W4)
#	add_definitions(/wd4100) # unreferenced formal parameter
#	add_definitions(/wd4127) # conditional expression is constant
#	add_definitions(/wd4244) # possible loss of data
#	add_definitions(/wd4245) # signed/unsigned mismatch
#	add_definitions(/wd4714) # 'function' marked as __forceinline not inlined
#	add_definitions(/wd4996) # 'function': was declared deprecated
#	set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG /Od /Zi /MTd")
#	set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG /Ox /Oy /MT")
#	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-DNDEBUG /Ox /Oy /Zi /MT")
#	set(CMAKE_C_FLAGS_MINSIZEREL "-DNDEBUG /Ox /Oy /Os /MT")
#else()
#	message(FATAL_ERROR "Unsupported compiler")
#endif()

# Operating settings
#if(WIN64)
#	add_definitions(-DWIN64)
#endif()

#if (NOT WIN32 AND NOT APPLE)
#	add_definitions(-DARCH_STRING=\"${Architecture}\")
#endif()

#set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
#set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
#set(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL})

# mingw and msvc
if (WIN32)
	add_definitions(-DWIN32)
	add_definitions(-D_WINDOWS)
	#add_definitions(-DWINVER=0x0501)
	#add_definitions(-D_WIN32_WINNT=0x0501)

	set(sys_libs
		winmm
	)
endif()

#configure_file(
#    "${CMAKE_SOURCE_DIR}/config.h.in"
#    "${CMAKE_BINARY_DIR}/config.h"
#)

#message(STATUS "CMAKE_C_FLAGS_RELEASE is ${CMAKE_C_FLAGS_RELEASE}")
#message(STATUS "CMAKE_C_FLAGS_DEBUG is ${CMAKE_C_FLAGS_DEBUG}")
#message(STATUS "Building ${CMAKE_BUILD_TYPE} for ${os}-${Architecture}")

#find_package(Lua)

set(SRCDIR "${CMAKE_SOURCE_DIR}")

set(cJSON_files
	cJSON/cJSON.c
	cJSON/cJSON.h
)

set(api_files
	api/shared/q_bit.c
	api/shared/q_bit.h
	api/shared/q_color.c
	api/shared/q_color.h
	api/shared/q_math.c
	api/shared/q_math.h
	api/shared/q_path.c
	api/shared/q_path.h
	api/shared/q_string.c
	api/shared/q_string.h
	api/et/q_shared.c
	api/shared/q_platform.h
	#${src_cJSON}
)

set(cgame_files
	game/bg_misc.c
	game/bg_pmove.c
	game/bg_q3f_controllable.c
	game/bg_q3f_flyby.c
	game/bg_q3f_grenades.c
	game/bg_q3f_info.c
	game/bg_q3f_mapsentry.c
	game/bg_q3f_permon.c
	game/bg_q3f_playerclass.c
	game/bg_q3f_splines.c
	game/bg_q3f_util.c
	game/bg_q3f_weapon.c
	game/bg_slidemove.c
	game/bg_tracemap.c
	cgame/cg_atmospheric.c
	cgame/cg_consolecmds.c
	cgame/cg_draw.c
	cgame/cg_drawtools.c
	cgame/cg_effects.c
	cgame/cg_ents.c
	cgame/cg_event.c
	cgame/cg_flamethrower.c
	cgame/cg_info.c
	cgame/cg_localents.c
	cgame/cg_main.c
	cgame/cg_local.h
	cgame/cg_marks.c
	cgame/cg_newdraw.c
	cgame/cg_players.c
	cgame/cg_playerstate.c
	cgame/cg_polybus.c
	cgame/cg_predict.c
	cgame/cg_q3f_classutil.c
	cgame/cg_q3f_customshader.c
	cgame/cg_q3f_f2r.c
	cgame/cg_q3f_flare.c
	cgame/cg_q3f_flyby.c
	cgame/cg_q3f_grenades.c
	cgame/cg_q3f_init.c
	cgame/cg_q3f_mapinfo.c
	cgame/cg_q3f_mapselect.c
	cgame/cg_q3f_mapsentry.c
	cgame/cg_q3f_menu.c
	cgame/cg_q3f_panel.c
	cgame/cg_q3f_rendereffects.c
	cgame/cg_q3f_scanner.c
	cgame/cg_q3f_scriptlib.c
	cgame/cg_q3f_sentry.c
	cgame/cg_q3f_sounddict.c
	cgame/cg_q3f_spawn.c
	cgame/cg_q3f_spirit.c
	cgame/cg_q3f_trapcalls.c
	cgame/cg_q3f_usermenu.c
	cgame/cg_q3f_water.c
	cgame/cg_q3f_waypoint.c
	cgame/cg_scoreboard.c
	cgame/cg_servercmds.c
	cgame/cg_snapshot.c
	api/et/cg_syscalls.c
	cgame/cg_trails.c
	cgame/cg_unlagged.c
	cgame/cg_view.c
	cgame/cg_weapons.c
	cgame/cgame.def
	ui_new/ui_shared.c
	ui_new/ui_shared.h
)

if(BOTS)
	set( cgame_files ${cgame_files} cgame/cg_omnibot.c cgame/Omnibot_Interprocess.cpp )
endif(BOTS)


#set(src_crypto
#    crypto/sha-1/sha1.c
#    crypto/sha-1/sha1.h
#)

set(game_files
	game/bg_misc.c
	game/bg_pmove.c
	game/bg_q3f_controllable.c
	game/bg_q3f_flyby.c
	game/bg_q3f_grenades.c
	game/bg_q3f_mapsentry.c
	game/bg_q3f_permon.c
	game/bg_q3f_playerclass.c
	game/bg_q3f_splines.c
	game/bg_q3f_util.c
	game/bg_q3f_weapon.c
	game/bg_slidemove.c
	game/bg_tracemap.c
	game/g_active.c
	#game/g_bot_interface.c
	game/g_client.c
	game/g_cmds.c
	game/g_combat.c
	game/g_items.c
	game/g_main.c
	game/g_local.h
	game/g_mem.c
	game/g_misc.c
	game/g_missile.c
	game/g_mover.c
	game/g_q3f_admin.c
	game/g_q3f_charge.c
	game/g_q3f_controllable.c
	game/g_q3f_crc.c
	game/g_q3f_engineer.c
	game/g_q3f_flag.c
	game/g_q3f_grenades.c
	game/g_q3f_mapdata.c
	game/g_q3f_mapents.c
	game/g_q3f_mapinfo.c
	game/g_q3f_mapselect.c
	game/g_q3f_mapsentry.c
	game/g_q3f_panel.c
	game/g_q3f_playerclass.c
	game/g_q3f_ready.c
	game/g_q3f_scanner.c
	game/g_q3f_serverconfig.c
	game/g_q3f_soundscript.c
	game/g_q3f_specents.c
	game/g_q3f_string.c
	game/g_q3f_team.c
	game/g_q3f_waypoint.c
	game/g_q3f_weapon.c
	game/g_session.c
	game/g_spawn.c
	game/g_stats.c
	game/g_svcmds.c
	api/et/g_syscalls.c
	game/g_target.c
	game/g_team.c
	game/g_trigger.c
	game/g_unlagged.c
	game/g_utils.c
	game/g_weapon.c
	game/game.def
)

if(LUA_FOUND)
	message(STATUS, "Found LUA version ${LUA_VERSION_STRING}")
	#set( src_game ${src_game} game/g_lua.c game/g_lua.h game/lsqlite3.c game/sqlite3.c game/sqlite3.h )
    set( game_files ${game_files} game/g_lua.c game/g_lua.h )
else()
	message(STATUS, "LUA not found, skipping")
endif()

if(BOTS)
	set( game_files ${game_files} game/g_bot_interface.cpp)
endif(BOTS)

set(ui_files
	game/bg_misc.c
	game/bg_q3f_info.c
	game/bg_q3f_playerclass.c
	ui_new/ui_atoms.c
	ui_new/ui_gameinfo.c
	ui_new/ui_main.c
	ui_new/ui_local.h
	ui_new/ui_players.c
	ui_new/ui_q3f_f2r.c
	ui_new/ui_shared.c
	ui_new/ui_shared.h
	api/et/ui_syscalls.c
	ui_new/ui_util.c
	ui_new/ui.def
)

if(BOTS)
	set(CMAKE_BOT_INCLUDE "" CACHE PATH "External Path for Omnibot" FORCE)
	#if(CGAME)
	#	set( src_cgame ${src_cgame} cg_omnibot.c cgame/Omnibot_Interprocess.cpp)
	#endif()
	if(GAME)
		set( src_game ${src_game} game/g_bot_interface.cpp ${CMAKE_BOT_INCLUDE}/Common/BotLoadLibrary.cpp)
	endif(GAME)

	#include_directories(${CMAKE_SOURCE_DIR}/bot) #FIXME WRONG
	include_directories(${CMAKE_BOT_INCLUDE}/Common)
	include_directories(${CMAKE_BOT_INCLUDE}/TeamFortressLib)
	include_directories(${CMAKE_BOT_INCLUDE}/ETF)

	add_definitions(-DBUILD_BOTS)
endif()

#if(ETXREAL)
#	add_definitions(-D_ETXREAL)
#endif()

#add_definitions(-DAPI_ET)
#include_directories(${CMAKE_SOURCE_DIR}/api/et)
#if (NOT MSVC)
#	include_directories(${CMAKE_BINARY_DIR})
#endif()
#include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/cJSON/)

#if(CMAKE_BUILD_TYPE MATCHES "DEBUG" OR CMAKE_BUILD_TYPE MATCHES "Debug")
#	# CMake already defines _DEBUG for MSVC.
#	if (NOT MSVC)
#		add_definitions(-D_DEBUG)
#	endif()
#endif()


add_library(cgame SHARED "${cgame_files}" "${api_files}")
target_compile_options(cgame
	PRIVATE $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:DEBUG>>:${compiler_flags_debug} -w>
			$<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:RELEASE>>:${compiler_flags_release}>
			$<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:RELWITHDEBINFO>>:${compiler_flags_relwithdebinfo} -w>
			$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:DEBUG>>:${compiler_flags_debug} -w>
			$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RELEASE>>:${compiler_flags_release}>
			$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RELWITHDEBINFO>>:${compiler_flags_relwithdebinfo} -w>
)
set_target_properties(cgame PROPERTIES PREFIX "")
set_target_properties(cgame PROPERTIES SUFFIX "${LIB_SUFFIX}${Architecture}${DLL_EXT}") #NOTE changeme when multi-platform and multi-arch support is added
target_compile_definitions(cgame PUBLIC "API_ET" "CGAME" "CGAMEDLL" "WOLF_FLAME" "ARCH_STRING=\"${Architecture}\"")
target_include_directories(cgame PUBLIC ${SRCDIR}/cgame ${SRCDIR}/game ${SRCDIR}/ui_new ${SRCDIR}/api/et ${SRCDIR}/api/shared )
target_link_libraries(cgame PRIVATE ${CMAKE_DL_LIBS} "m")

add_library(qagame SHARED "${game_files}" "${cJSON_files}" "${api_files}")
target_compile_options(qagame
	PRIVATE $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:DEBUG>>:${compiler_flags_debug} -w>
			$<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:RELEASE>>:${compiler_flags_release}>
			$<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:RELWITHDEBINFO>>:${compiler_flags_relwithdebinfo} -w>
			$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:DEBUG>>:${compiler_flags_debug} -w>
			$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RELEASE>>:${compiler_flags_release}>
			$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RELWITHDEBINFO>>:${compiler_flags_relwithdebinfo} -w>
)
set_target_properties(qagame PROPERTIES PREFIX "")
set_target_properties(qagame PROPERTIES SUFFIX "${LIB_SUFFIX}${Architecture}${DLL_EXT}") #NOTE changeme when multi-platform and multi-arch support is added
target_compile_definitions(qagame PUBLIC "API_ET" "QAGAME" "NO_BOT_SUPPORT" "ARCH_STRING=\"${Architecture}\"")
target_include_directories(qagame PUBLIC ${SRCDIR}/game ${SRCDIR}/cJSON ${SRCDIR}/api/et ${SRCDIR}/api/shared )
target_link_libraries(qagame PRIVATE ${CMAKE_DL_LIBS} "m")

add_library(ui SHARED "${ui_files}" "${api_files}")
target_compile_options(ui
	PRIVATE $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:DEBUG>>:${compiler_flags_debug} -w>
			$<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:RELEASE>>:${compiler_flags_release}>
			$<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:RELWITHDEBINFO>>:${compiler_flags_relwithdebinfo} -w>
			$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:DEBUG>>:${compiler_flags_debug} -w>
			$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RELEASE>>:${compiler_flags_release}>
			$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RELWITHDEBINFO>>:${compiler_flags_relwithdebinfo} -w>
)
set_target_properties(ui PROPERTIES PREFIX "")
set_target_properties(ui PROPERTIES SUFFIX "${LIB_SUFFIX}${Architecture}${DLL_EXT}") #NOTE changeme when multi-platform and multi-arch support is added
target_compile_definitions(ui PUBLIC "API_ET" "UI_EXPORTS" "ARCH_STRING=\"${Architecture}\"")
target_include_directories(ui PUBLIC ${SRCDIR}/cgame ${SRCDIR}/game ${SRCDIR}/ui_new ${SRCDIR}/api/et ${SRCDIR}/api/shared )
target_link_libraries(ui PRIVATE ${CMAKE_DL_LIBS} "m")

if(WIN32)
	install(TARGETS cgame qagame ui
		RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/etf"
	)
else()
	install(TARGETS cgame qagame ui
		LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/etf"
	)
endif()
