# version minimum de cmake requise pour le build
cmake_minimum_required(VERSION 3.22.0 FATAL_ERROR)

# déclaration du projet
project(BATTLESHIPS VERSION 0.1.0 LANGUAGES C)

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_INET_PATH "${SRC}/inet_lib")
set(LIB_LOGGING_PATH "${SRC}/logging_lib")
set(LIB_APP_PATH "${SRC}/app_lib")
set(LIB_GAME_PATH "${SRC}/game_logic")

add_library(
	LIB_LOGGING
	"${LIB_LOGGING_PATH}/include/logging.h"
	"${LIB_LOGGING_PATH}/logging.c"
)
target_include_directories(LIB_LOGGING PUBLIC "${LIB_LOGGING_PATH}/include")

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
	LIB_GAME
	"${LIB_GAME_PATH}/include/bataille_navale.h"
	"${LIB_GAME_PATH}/include/affichage.h"
	"${LIB_GAME_PATH}/include/logic.h"
	"${LIB_GAME_PATH}/include/structSerial.h"
	"${LIB_GAME_PATH}/logic.c"
	"${LIB_GAME_PATH}/affichage.c"
	"${LIB_GAME_PATH}/structSerial.c"
)
target_include_directories(LIB_GAME PUBLIC "${LIB_GAME_PATH}/include")



add_library(
	LIB_APP
	"${LIB_APP_PATH}/include/repReq.h"
	"${LIB_APP_PATH}/include/dial.h"
	"${LIB_APP_PATH}/include/protocol.h"
	"${LIB_APP_PATH}/include/datastructs.h"
	"${LIB_APP_PATH}/include/interface.h"
	"${LIB_APP_PATH}/repReq.c"
	"${LIB_APP_PATH}/dial.c"
	"${LIB_APP_PATH}/protocol.c"
	"${LIB_APP_PATH}/datastructs.c"
	"${LIB_APP_PATH}/interface.c"
)
target_include_directories(LIB_APP PUBLIC "${LIB_APP_PATH}/include")
target_link_libraries(LIB_APP PUBLIC LIB_INET)
target_link_libraries(LIB_APP PUBLIC LIB_GAME)


add_executable(server "${SRC}/serveurEnregistrement.c")
target_link_libraries(server LIB_APP LIB_APP)

add_executable(client "${SRC}/client_louis.c")
target_link_libraries(client LIB_GAME LIB_APP)

add_executable(client_mat "${SRC}/client.c")
target_link_libraries(client_mat LIB_APP LIB_APP)
