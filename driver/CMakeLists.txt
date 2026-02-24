cmake_minimum_required (VERSION 2.6)

#SET(GCC_COVERAGE_COMPILE_FLAGS "-Wall -g")
SET (CMAKE_CXX_FLAGS "-g -Wall")
SET(GCC_LIBRARY "-lpthread -lboost_thread -lmysqlclient -lboost_system")

project(driver)


add_executable(driver src/main.cpp src/mesh_xbee.cpp src/api_uart.cpp src/xbee_listen.cpp src/control_listen.cpp src/log_class src/xbee_dev.cpp src/xmlParser.cpp src/comm_data.cpp )
TARGET_LINK_LIBRARIES( driver ${GCC_LIBRARY}  )

install(TARGETS driver DESTINATION bin)
