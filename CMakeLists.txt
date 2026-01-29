# version minimum de cmake requise pour le build
cmake_minimum_required(VERSION 3.31.0 FATAL_ERROR)

# déclaration du projet
project(BATTLESHIPS VERSION 0.1.0 LANGUAGES C)

set(LIB_INET_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/inet_lib")
set(LIB_LOGGING_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/logging_lib")
set(LIB_APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/app_lib")


add_library(
	LIB_LOGGING
	"${LIB_LOGGING_PATH}/include/logging.h"

	"${LIB_LOGGING_PATH}/logging.c"
)
target_include_directories(LIB_LOGGING PUBLIC "${LIB_LOGGING_PATH}/include")


# ajout lib avec les couches session et data
add_library(
	LIB_INET 
	"${LIB_INET_PATH}/include/session.h"
	"${LIB_INET_PATH}/include/data.h"

	"${LIB_INET_PATH}/session.c"
	"${LIB_INET_PATH}/data.c"
)
target_include_directories(LIB_INET PUBLIC "${LIB_INET_PATH}/include")
target_link_libraries(LIB_INET PUBLIC LIB_LOGGING)


add_library(
	LIB_APP
	"${LIB_APP_PATH}/include/repReq.h"
	"${LIB_APP_PATH}/include/dial.h"
	"${LIB_APP_PATH}/include/protocol.h"

	"${LIB_APP_PATH}/repReq.c"
	"${LIB_APP_PATH}/dial.c"
	"${LIB_APP_PATH}/protocol.c"
)
target_include_directories(LIB_APP PUBLIC "${LIB_APP_PATH}/include")
target_link_libraries(LIB_APP PUBLIC LIB_INET)


add_executable(SERVER "demo_app.c")
target_compile_definitions(SERVER PRIVATE SERVER)
target_link_libraries(SERVER LIB_APP)

add_executable(CLIENT "demo_app.c")
target_compile_definitions(CLIENT PRIVATE CLIENT)
target_link_libraries(CLIENT LIB_APP)