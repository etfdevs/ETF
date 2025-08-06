string(TIMESTAMP TODAY "%Y-%m-%d_%H-%M")
set(PK3_NAME "z${CMAKE_PROJECT_NAME}-${TODAY}")
set(PK3_PATH "${BASE_DIR_PATH}/${PK3_NAME}.pk3")

add_custom_target(mod_pk3
		COMMAND ${CMAKE_COMMAND}
			-DINPUT="cgame*.so;cgame*.dll;cgame_mac;ui*.so;ui*.dll;ui_mac"
			-DOUTPUT="${PK3_PATH}"
			-P ${CMAKE_SOURCE_DIR}/cmake/AssetsPackager.cmake
		WORKING_DIRECTORY ${BASE_DIR_PATH}
		DEPENDS cgame ui remove_timestamp remove_old_pk3
		COMMENT "Packing ${PK3_NAME}.pk3"
)

set(RELEASE_NAME "z${CMAKE_PROJECT_NAME}-${TODAY}-${ETF_GAME_VERSION}")
set(RELEASE_PATH "${CMAKE_BINARY_DIR}/${RELEASE_NAME}.zip")

add_custom_target(mod_release
		COMMAND ${CMAKE_COMMAND}
			-DINPUT="${BASE_DIR}/qagame*.so;${BASE_DIR}/qagame*.dll;${BASE_DIR}/qagame_mac;${BASE_DIR}/${PK3_NAME}.pk3"
			-DOUTPUT="${RELEASE_PATH}"
			-P ${CMAKE_SOURCE_DIR}/cmake/AssetsPackager.cmake
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		DEPENDS mod_pk3
		COMMENT "Packing ${RELEASE_NAME}.zip"
)

add_custom_target(remove_old_pk3
		COMMAND ${CMAKE_COMMAND}
			-DPK3_PATH="${BASE_DIR_PATH}"
			-P ${CMAKE_SOURCE_DIR}/cmake/RemoveOldPK3.cmake
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		COMMENT "Cleaning old mod pk3 files"
)

add_custom_target(remove_timestamp
		COMMAND ${CMAKE_COMMAND}
			-U TODAY ${CMAKE_BINARY_DIR}
		COMMENT "Cleaning timestamp"
)