#ifndef PROTOCOL_H
#define PROTOCOL_H

#define protocol_version "Q20211210"

#define websocket_port 7003
#define udp_port 7000
#define keep_alive_time_out 10000
#define keep_alive_time_cycle 5000

#define cmd_src_client_name_server "msg_server"
#define cmd_src_client_name_monitor "msg_monitor"
#define cmd_src_client_name_C101 "C101"
#define cmd_src_client_name_C102 "C102"
#define cmd_src_client_name_C103 "C103"
#define cmd_src_client_name_C104 "C104"
#define cmd_src_client_name_C105 "C105"
#define cmd_src_client_name_C106 "C106"
#define cmd_src_client_name_C107 "C107"
#define cmd_src_client_name_C108 "C108"
#define cmd_src_client_name_C109 "C109"
#define cmd_src_client_name_C110 "C110"
#define cmd_src_client_name_C111 "C111"
#define cmd_src_client_name_C112 "C112"


#define cmd_status_send 10
#define cmd_status_receive 20
#define cmd_status_processing 30
#define cmd_status_end_success 40
#define cmd_status_end_error 41

#define cmd_code_discovery 1010
#define cmd_name_discovery "discovery server"
#define cmd_code_test 1011
#define cmd_name_test "test server"

#define cmd_code_client_list 1021
#define cmd_name_client_list "get client List"

#define cmd_code_client_connect 1022
#define cmd_name_client_connect "new client connect"

#define cmd_code_client_remove 1023
#define cmd_name_client_remove "client removed"

#define cmd_code_client_keep_alive 1031
#define cmd_name_client_keep_alive "alive check"


#define cmd_code_client_ezcap_save_avi 2010
#define cmd_name_client_ezcap_save_avi "ezcap save avi"
#define cmd_code_client_ezcap_max_window 2020
#define cmd_name_client_ezcap_max_window "ezcap max window"


//cmd_src_client_name
//cmd_des_client_name
//cmd_value_client_name
//cmd_value_client_ezcap_set_window
//cmd_value_client_ezcap_set_avi


#endif // PROTOCOL_H
