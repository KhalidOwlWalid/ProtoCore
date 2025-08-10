#include "../common.hpp"
#include "mavlink/all/mavlink.h"

#define MAVLINK_SERVER_PORT 14550
#define MAVLINK_SERVER_ADDRESS "127.0.0.1"
#define ARDUPILOT_SITL_ADDRESS MAVLINK_SERVER_ADDRESS

int sock_fd;
struct sockaddr_in server_address = {};
struct sockaddr_in client_address = {};
socklen_t address_len = sizeof(client_address);

uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
