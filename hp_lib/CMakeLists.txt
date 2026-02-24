cmake_minimum_required (VERSION 3.0)

option(USE_STANDALONE_ASIO "set ON to use standalone Asio instead of Boost.Asio" OFF)
option(BUILD_TESTING "set ON to build library tests" OFF)

#set(SOURCE_FILES include/server_https.hpp include/server_http.hpp include/client_https.hpp include/client_http.hpp include/crypto.hpp include/status_code.hpp include/utility.hpp src/hp_xml_parser.cpp src/xmlParser.cpp src/hp_tcp_acceptor.cpp src/hp_tcp_stream.cpp) #src/hp_xml_parser.cpp src/xmlParser.cpp)
set(SOURCE_FILES src/hp_tcp_connector.cpp src/hp_xml_parser.cpp src/xmlParser.cpp src/hp_tcp_acceptor.cpp src/hp_tcp_stream.cpp src/pugixml.cpp) 

if(NOT MSVC)
	add_compile_options(-std=c++17 -Wall -Wextra -Wsign-conversion)
	set(GCC_LIBRARY "-lpthread -lboost_thread -lboost_filesystem -lboost_system -lmysqlclient -lrt -ljsoncpp")
else()
	add_compile_options(/W4)
endif()

add_library(hp_lib ${SOURCE_FILES})

target_link_libraries(hp_lib ${GCC_LIBRARY})

# HTTP/S server/client based on boost asio

add_library(simple-web-server INTERFACE)
add_library(simple-websocket-server INTERFACE)

target_include_directories(simple-web-server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(simple-websocket-server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(simple-web-server INTERFACE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(simple-websocket-server INTERFACE ${CMAKE_THREAD_LIBS_INIT})

# TODO 2020 when Debian Jessie LTS ends:
# Remove Boost system, thread, regex components; use Boost::<component> aliases; remove Boost target_include_directories
if(USE_STANDALONE_ASIO)
#webserver
    target_compile_definitions(simple-web-server INTERFACE USE_STANDALONE_ASIO)
    include(CheckIncludeFileCXX)
    CHECK_INCLUDE_FILE_CXX(asio.hpp HAVE_ASIO)
    if(NOT HAVE_ASIO)
        message(FATAL_ERROR "Standalone Asio not found")
    endif()
#websocket
    target_compile_definitions(simple-websocket-server INTERFACE USE_STANDALONE_ASIO)
    find_path(ASIO_PATH asio.hpp)
    if(NOT ASIO_PATH)
        message(FATAL_ERROR "Standalone Asio not found")
    else()
        target_include_directories(simple-websocket-server INTERFACE ${ASIO_PATH})
    endif()

else()
#webserver
    find_package(Boost 1.53.0 COMPONENTS system thread REQUIRED)
    target_link_libraries(simple-web-server INTERFACE ${Boost_LIBRARIES})
    target_include_directories(simple-web-server INTERFACE ${Boost_INCLUDE_DIR})
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        target_compile_definitions(simple-web-server INTERFACE USE_BOOST_REGEX)
        find_package(Boost 1.53.0 COMPONENTS regex REQUIRED)
        target_link_libraries(simple-web-server INTERFACE ${Boost_LIBRARIES})
        target_include_directories(simple-web-server INTERFACE ${Boost_INCLUDE_DIR})
    endif()
#websocket
    find_package(Boost 1.54.0 COMPONENTS system thread coroutine context REQUIRED)
    target_link_libraries(simple-websocket-server INTERFACE ${Boost_LIBRARIES})
    target_include_directories(simple-websocket-server INTERFACE ${Boost_INCLUDE_DIR})
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        target_compile_definitions(simple-websocket-server INTERFACE USE_BOOST_REGEX)
        find_package(Boost 1.54.0 COMPONENTS regex REQUIRED)
        target_link_libraries(simple-websocket-server INTERFACE ${Boost_LIBRARIES})
        target_include_directories(simple-websocket-server INTERFACE ${Boost_INCLUDE_DIR})
    endif()

endif()
if(WIN32)
    target_link_libraries(simple-web-server INTERFACE ws2_32 wsock32)
endif()

if(APPLE)
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()
find_package(OpenSSL)
if(OPENSSL_FOUND)
    target_compile_definitions(simple-web-server INTERFACE HAVE_OPENSSL)
    target_link_libraries(simple-web-server INTERFACE ${OPENSSL_LIBRARIES})
    target_include_directories(simple-web-server INTERFACE ${OPENSSL_INCLUDE_DIR})
    
    target_compile_definitions(simple-websocket-server INTERFACE HAVE_OPENSSL)
    target_link_libraries(simple-websocket-server INTERFACE ${OPENSSL_LIBRARIES})
    target_include_directories(simple-websocket-server INTERFACE ${OPENSSL_INCLUDE_DIR})

endif()
