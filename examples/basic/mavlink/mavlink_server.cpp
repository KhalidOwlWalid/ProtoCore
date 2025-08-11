/*
This example provides a basic guide on how you can create your own custom GCS by listening to the open by the autopilot.

To test this script, on your terminal, run:
sim_vehicle.py --vehicle Copter --out=127.0.0.1:14550

Once the SITL is running, run this script:
./mavlink_server

Please note that since this binds with the localhost:14550 socket, having any Mission Planner or QGroundControl which already
connects to this network will cause bind error. Please ensure you disconnect them before running this.
*/

#include "mavlink_common.hpp"

int socket_fd;
struct sockaddr_in server_address = {};
struct sockaddr_in client_address = {};
socklen_t client_address_len = sizeof(client_address);

int main() {

    spdlog::info("Mavlink Server example");

    // This is compulsary if you are developing GCS as you need to bind to a socket
    init_server(socket_fd, server_address, AF_INET, SOCK_DGRAM, "127.0.0.1", MAVLINK_SERVER_PORT);

    char ip_str[INET_ADDRSTRLEN];
    spdlog::info("Mavlink server running on port {}:{}", \ 
        inet_ntop(AF_INET, &(server_address.sin_addr), ip_str, INET_ADDRSTRLEN), ntohs(server_address.sin_port));

    while (run_application) {

        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        ssize_t recv_len = recvfrom(socket_fd, buffer, 2048, 0, (struct sockaddr*)(&client_address), &client_address_len); 

        if (recv_len < 0) {
            spdlog::error("recvfrom failed with error, {}", strerror(errno));
            break;
        }

        mavlink_message_t mav_message;
        mavlink_status_t mav_status;

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), ip_str, INET_ADDRSTRLEN);

        for (size_t i = 0; i < recv_len; i++) {
            if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &mav_message, &mav_status)) {
                switch (mav_message.msgid) {
                    case MAVLINK_MSG_ID_HEARTBEAT:
                        spdlog::info("Receiving Heartbeat from [target:component][{}:{}] at address {}:{}", mav_message.sysid, mav_message.compid, ip_str, ntohs(client_address.sin_port));
                    case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
                        spdlog::info("Received data for target position");
                        mavlink_position_target_local_ned_t target_local_pos_ned;
                        mavlink_msg_position_target_local_ned_decode(&mav_message, &target_local_pos_ned);
                        spdlog::info("Target Local POS NED: {}, {}", target_local_pos_ned.x, target_local_pos_ned.y);
                    default:
                        #if PRINT_ALL_MSG_ID
                        spdlog::info("Receiveing data from: -");
                        spdlog::info("Message ID: {}", (uint32_t)mav_message.msgid);
                        spdlog::info("System ID: {}", (uint32_t)mav_message.sysid);
                        spdlog::info("Component ID: {}", (uint32_t)mav_message.compid);
                        #endif
                        continue;
                }
            }
        }
    }
    close(socket_fd);
    return 0;
}
