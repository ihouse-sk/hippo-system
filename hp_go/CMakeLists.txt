cmake_minimum_required (VERSION 2.6)
project(hp_go)

#SET(GCC_COVERAGE_COMPILE_FLAGS "-Wall -g")
SET(CMAKE_CXX_FLAGS "-g -Wall -std=c++14")
SET(GCC_LIBRARY "-lmysqlclient -ljsoncpp")
SET(GCC_LIB_SNMP "-lnetsnmp")

find_package(OpenSSL REQUIRED)

add_subdirectory(../hp_lib hp_lib) # dolezite - pridanie kniznic do aktualneho projektu

add_executable(hp_go src/hp2heating.cpp src/ih_server_socket.cpp src/hp_gui_listener.cpp src/hp_hbx.cpp src/hp_pin_status_light.cpp src/hp_virtual_pin.cpp src/hp_listen_thread.cpp src/hp_server_socket.cpp src/hp_client_socket.cpp src/hp_sender.cpp src/hp_mess_data.cpp src/hp_db_handler.cpp src/hp_lightning.cpp src/hp_lightning_rule.cpp src/hp_push_button.cpp src/hp_output_pin.cpp src/hp_input_pin.cpp src/hp_pin_pwm.cpp src/hp_repeat_mess.cpp src/hp_shutters.cpp src/hp_shutter src/hp_pin_shutter.cpp src/hp_onOffButton.cpp src/hp_security.cpp src/hp_security_zone.cpp src/hp_sensor_pin.cpp src/hp_pin_temp.cpp  src/hp_heating_section.cpp src/hp2heating_zone.cpp  src/hp_pin_heater.cpp src/hp_pin_cons.cpp src/hp_cards.cpp src/hp_card.cpp src/hp_advanced_scenario.cpp src/hp_button_scenario.cpp src/hp_gates.cpp src/hp_gate.cpp src/hp_shared_memory.cpp src/hp_watering.cpp src/hp_watering_zone.cpp src/hp_rekuperacia.cpp src/hp_rekuperacia_zone.cpp src/hp_turnikets.cpp src/hp_turniket.cpp src/hp_threaded_socket.cpp src/hp_timing.cpp src/hp_sec_calc.cpp src/hp_notification.cpp src/hp_pin_notifier.cpp src/hp_guard.cpp src/hp_dali_item.cpp src/hp_dali_pwm_light.cpp src/hp_dali.cpp src/hp_dali_group.cpp src/hp_dalinet_device.cpp src/hp_modbus.cpp src/hp_modbus_data.cpp src/hp_heating_section_mdb.cpp src/hp_jablotron.cpp src/hp_jablotron_zone.cpp src/hp_conditions.cpp src/hp_impulz_counter.cpp src/hp2modbus_data.cpp src/hp2modbus.cpp src/hp2modbus_devices.cpp src/hp2modbus_slave.cpp src/hp_ws_server.cpp src/hp_wss_server.cpp src/hp_data_provider.cpp src/hp_wss_client.cpp  src/main.cpp src/hp2go.cpp)

add_executable(test src/bug_tester.cpp)
TARGET_LINK_LIBRARIES(test ${GCC_LIBRARY} ${OPENSSL_LIBRARIES} hp_lib)

TARGET_LINK_LIBRARIES(hp_go ${GCC_LIBRARY} ${OPENSSL_LIBRARIES} hp_lib)

add_executable(asio src/asio_test.cpp src/hp_threaded_socket.cpp)
TARGET_LINK_LIBRARIES(asio ${GCC_LIBRARY} ${OPENSSL_LIBRARIES} hp_lib)

#find_library(NetSnmp REQUIRED)

#add_executable(hp_meteo src/hp_giom.cpp src/hp_meteo_giom.cpp)
#TARGET_LINK_LIBRARIES(hp_meteo ${GCC_LIB_SNMP} hp_lib)

