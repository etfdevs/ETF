project(etf C CXX)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckFunctionExists)

option(CGAME			"Build the cgame dll code" ON)
option(GAME			"Build the qagame dll code" ON)
option(UI					"Build the ui dll code" ON)
option(BOTS				"Enable bot support" ON)
#option(ETXREAL		"Compatible with ETXreaL" OFF)

if (NOT CMAKE_SYSTEM_PROCESSOR)
	message(FATAL_ERROR "No target CPU architecture set")
endif()

if (NOT CMAKE_SYSTEM_NAME)
	message(FATAL_ERROR "No target OS set")
endif()

Include(CheckTypeSize)
check_type_size("void*" CMAKE_SIZEOF_VOID_P)

# ${Architecture} must match ARCH_STRING in q_platform.h,
# and is used in DLL names (jagamex86.dll, jagamex86.dylib, jagamei386.so).
if(WIN32)
	set(X86 ON)
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(Architecture "x86_64")
		set(WIN64 TRUE)
	else()
		set(Architecture "x86")
		set(WIN64 FALSE)
	endif()
else()
	set(X86 OFF)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
		set(Architecture "arm")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
		set(X86 ON)
		if(APPLE)
			set(Architecture "x86")
		else()
			# e.g. Linux
			set(Architecture "i386")
		endif()
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^x86.64$")
		set(X86 ON)
		set(Architecture "x86_64")
	elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc")
		set(Architecture "ppc")
	elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc64")
		set(Architecture "ppc64")
	else()
		set(Architecture "${CMAKE_SYSTEM_PROCESSOR}")
	endif()
endif()

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

# target os
if (APPLE)
	set(os "macosx")
else()
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" os)
endif()

# build type
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# compiler specific flags
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	add_definitions(-pipe)
	add_definitions(-Wall)

	if (NOT CMAKE_CROSSCOMPILING AND ONATIVE)
		add_definitions(-march=native)
	elseif (NOT APPLE AND Architecture STREQUAL "x86")
		add_definitions(-march=pentium3)
	endif()

	set(CMAKE_C_FLAGS_DEBUG "-g -D_DEBUG -O1")
	set(CMAKE_C_FLAGS_DEBUGALL "-g -ggdb -D_DEBUG")
	set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_MINSIZEREL "-Os -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")

	add_definitions(-fno-strict-aliasing)

	CHECK_C_COMPILER_FLAG("-fvisibility=hidden" c_has_fvisibility)
	if (NOT c_has_fvisibility)
		message(FATAL_ERROR "Compiler does not support -fvisibility")
	endif()
	add_definitions(-fvisibility=hidden)

	# TODO fix these warnings
	add_definitions(-Wno-sign-compare)
	add_definitions(-Wno-switch)
	add_definitions(-Wno-format-security)

	#CHECK_CXX_COMPILER_FLAG("-Woverloaded-virtual" cxx_has_Woverload_virtual)
	#if (cxx_has_Woverload_virtual)
	#	add_definitions(-Woverloaded-virtual)
	#endif()

	if (APPLE)
		# no suffix for .dylib
		#set(gamesuffix "")

		add_definitions(-DMACOS_X=1)

		if (Architecture STREQUAL "x86_64")
			add_definitions(-arch x86_64 -mmacosx-version-min=10.6)
			set(ldflags "${ldflags} -arch x86_64 -mmacosx-version-min=10.6")
		elseif (Architecture STREQUAL "x86")
			CHECK_CXX_COMPILER_FLAG("-arch i386" cxx_has_arch_i386)
			if (cxx_has_arch_i386)
				add_definitions(-arch i386)
				set(ldflags "${ldflags} -arch i386")
			endif()

			add_definitions(-mmacosx-version-min=10.4)
			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
		elseif (Architecture STREQUAL "ppc")
			CHECK_CXX_COMPILER_FLAG("-arch ppc" cxx_has_arch_ppc)
			if (cxx_has_arch_ppc)
				add_definitions(-arch ppc)
				set(ldflags "${ldflags} -arch ppc")
			endif()

			add_definitions(-mmacosx-version-min=10.4)
			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
		else()
			message(FATAL_ERROR "Unsupported CPU architecture for OSX")
		endif()

		set(sys_libs ${sys_libs} "-framework Carbon -framework Cocoa -framework OpenGL -framework IOKit")
	else()
		if (Architecture STREQUAL "x86" AND X86)
			add_definitions(-m32)
			set(ldflags "${ldflags} -m32")
		endif()

		if (os STREQUAL "linux")
			set(sys_libs ${sys_libs} dl)
		endif()
	endif()
elseif (MSVC)
	add_definitions(/W4)
	add_definitions(/wd4100) # unreferenced formal parameter
	add_definitions(/wd4127) # conditional expression is constant
	add_definitions(/wd4244) # possible loss of data
	add_definitions(/wd4245) # signed/unsigned mismatch
	add_definitions(/wd4714) # 'function' marked as __forceinline not inlined
	add_definitions(/wd4996) # 'function': was declared deprecated
	set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG /Od /Zi /MTd")
	set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG /Ox /Oy /MT")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-DNDEBUG /Ox /Oy /Zi /MT")
	set(CMAKE_C_FLAGS_MINSIZEREL "-DNDEBUG /Ox /Oy /Os /MT")
else()
	message(FATAL_ERROR "Unsupported compiler")
endif()

# Operating settings
if(WIN64)
	add_definitions(-DWIN64)
endif()

if (NOT WIN32 AND NOT APPLE)
	add_definitions(-DARCH_STRING=\"${Architecture}\")
endif()

set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
set(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL})

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

configure_file(
    "${CMAKE_SOURCE_DIR}/config.h.in"
    "${CMAKE_BINARY_DIR}/config.h"
)

message(STATUS "Building ${CMAKE_BUILD_TYPE} for ${os}-${Architecture}")

set(src_shared
	${CMAKE_SOURCE_DIR}/api/shared/q_bit.c
	${CMAKE_SOURCE_DIR}/api/shared/q_bit.h
	${CMAKE_SOURCE_DIR}/api/shared/q_color.c
	${CMAKE_SOURCE_DIR}/api/shared/q_color.h
	${CMAKE_SOURCE_DIR}/api/shared/q_math.c
	${CMAKE_SOURCE_DIR}/api/shared/q_math.h
	${CMAKE_SOURCE_DIR}/api/shared/q_path.c
	${CMAKE_SOURCE_DIR}/api/shared/q_path.h
	${CMAKE_SOURCE_DIR}/api/shared/q_string.c
	${CMAKE_SOURCE_DIR}/api/shared/q_string.h
	${CMAKE_SOURCE_DIR}/api/et/q_shared.c
	${CMAKE_SOURCE_DIR}/api/shared/q_platform.h
)
	

set(src_cgame
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
	game/bg_q3f_tea.c
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
	cgame/cg_marks.c
	cgame/cg_newdraw.c
	#cgame/cg_omnibot.c
	cgame/cg_players.c
	cgame/cg_playerstate.c
	cgame/cg_polybus.c
	cgame/cg_predict.c
	cgame/cg_q3f_anticheat.c
	cgame/cg_q3f_classutil.c
	cgame/cg_q3f_controllable.c
	cgame/cg_q3f_crc.c
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
	cgame/cg_q3f_water.c
	cgame/cg_q3f_waypoint.c
	cgame/cg_scoreboard.c
	cgame/cg_servercmds.c
	cgame/cg_snapshot.c
	${CMAKE_SOURCE_DIR}/api/et/cg_syscalls.c
	cgame/cg_trails.c
	cgame/cg_unlagged.c
	cgame/cg_view.c
	cgame/cg_weapons.c
	#cgame/Omnibot_Interprocess.cpp
	cgame/cgame.def
	${src_shared}
	ui_new/ui_shared.c
)

set(src_game
	game/bg_misc.c
	game/bg_pmove.c
	game/bg_q3f_controllable.c
	game/bg_q3f_flyby.c
	game/bg_q3f_grenades.c
	game/bg_q3f_mapsentry.c
	game/bg_q3f_permon.c
	game/bg_q3f_playerclass.c
	game/bg_q3f_splines.c
	game/bg_q3f_tea.c
	game/bg_q3f_util.c
	game/bg_q3f_weapon.c
	game/bg_slidemove.c
	game/bg_tracemap.c
	game/g_active.c
	game/g_arenas.c
	#game/g_bot_interface.c
	game/g_client.c
	game/g_cmds.c
	game/g_combat.c
	game/g_items.c
	game/g_main.c
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
	game/g_svcmds.c
	${CMAKE_SOURCE_DIR}/api/et/g_syscalls.c
	game/g_target.c
	game/g_team.c
	game/g_trigger.c
	game/g_unlagged.c
	game/g_utils.c
	game/g_weapon.c
	game/game.def
	${src_shared}
)

set(src_ui
	game/bg_misc.c
	game/bg_q3f_info.c
	game/bg_q3f_playerclass.c
	${src_shared}
	ui_new/ui_atoms.c
	ui_new/ui_gameinfo.c
	ui_new/ui_main.c
	ui_new/ui_players.c
	ui_new/ui_q3f_f2r.c
	ui_new/ui_shared.c
	${CMAKE_SOURCE_DIR}/api/et/ui_syscalls.c
	ui_new/ui_util.c
	ui_new/ui.def
)

if(BOTS)
		set(CMAKE_BOT_INCLUDE "E:/omnibot_g/Omnibot")

		#include_directories(${CMAKE_SOURCE_DIR}/bot) #FIXME WRONG
		include_directories(${CMAKE_BOT_INCLUDE}/Common)
		include_directories(${CMAKE_BOT_INCLUDE}/TeamFortressLib)
		include_directories(${CMAKE_BOT_INCLUDE}/ETF)
	if(GAME)
		set(src_bot
			game/g_bot_interface.cpp
			${CMAKE_BOT_INCLUDE}/Common/BotLoadLibrary.cpp
		)
		#add_definitions(-DDREVIL_BOT_SUPPORT)
		add_definitions(-DBUILD_BOTS)
	endif()
endif()

#if(ETXREAL)
#	add_definitions(-D_ETXREAL)
#endif()

add_definitions(-DAPI_ET)
include_directories(${CMAKE_SOURCE_DIR}/api/et)
#if (NOT MSVC)
	include_directories(${CMAKE_BINARY_DIR})
#endif()
include_directories(${CMAKE_SOURCE_DIR})

if (CGAME)
	add_library(cgame.mp.${Architecture} SHARED ${src_cgame})
	set_target_properties(cgame.mp.${Architecture} PROPERTIES PREFIX "")
	set_target_properties(cgame.mp.${Architecture} PROPERTIES COMPILE_DEFINITIONS "CGAME;CGAMEDLL;WOLF_FLAME") #FIXME WRONG
	set_target_properties(cgame.mp.${Architecture} PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/cgame")
	set_target_properties(cgame.mp.${Architecture} PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(cgame.mp.${Architecture} PROPERTIES INSTALL_NAME_DIR "@executable_path")
	target_link_libraries(cgame.mp.${Architecture} ${sys_libs})
endif()

if (GAME)
	if (BOTS)
		add_library(qagame.mp.${Architecture} SHARED ${src_game} ${src_bot})
	else()
		add_library(qagame.mp.${Architecture} SHARED ${src_game})
	endif()
	set_target_properties(qagame.mp.${Architecture} PROPERTIES PREFIX "")
	set_target_properties(qagame.mp.${Architecture} PROPERTIES COMPILE_DEFINITIONS "QAGAME")
	set_target_properties(qagame.mp.${Architecture} PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/game")
	set_target_properties(qagame.mp.${Architecture} PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(qagame.mp.${Architecture} PROPERTIES INSTALL_NAME_DIR "@executable_path")
	target_link_libraries(qagame.mp.${Architecture} ${sys_libs})
endif()

if (UI)
	add_library(ui.mp.${Architecture} SHARED ${src_ui})
	set_target_properties(ui.mp.${Architecture} PROPERTIES PREFIX "")
	set_target_properties(ui.mp.${Architecture} PROPERTIES COMPILE_DEFINITIONS "UI_EXPORTS")
	set_target_properties(ui.mp.${Architecture} PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/ui_new")
	set_target_properties(ui.mp.${Architecture} PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(ui.mp.${Architecture} PROPERTIES INSTALL_NAME_DIR "@executable_path")
	target_link_libraries(ui.mp.${Architecture} ${sys_libs})
endif()