/*
This example provides a quick guideline on how to write your own heartbeat for either your ground control station or even
autopilot.
*/
#include "mavlink_common.hpp"

int socket_fd;
struct sockaddr_in target_address = {};

int main() {

    spdlog::info("Mavlink Heartbeat example");

    setup_sock_fd(socket_fd, AF_INET, SOCK_DGRAM);
    setup_address(target_address, AF_INET, MAVLINK_SERVER_ADDRESS, MAVLINK_SERVER_PORT);

    char ip_str[INET_ADDRSTRLEN];
    spdlog::info("Mavlink heartbeat sending to port {}:{}", \ 
        inet_ntop(AF_INET, &(target_address.sin_addr), ip_str, INET_ADDRSTRLEN), ntohs(target_address.sin_port));

    auto last_time = get_current_time_ms();

    while (run_application) {

        auto curr_time = get_current_time_ms();

        if (curr_time - last_time > 1000) {

            mavlink_message_t msg;
            const uint8_t system_id = 16;
            const uint8_t base_mode = 0;
            const uint8_t custom_mode = 0;
            mavlink_msg_heartbeat_pack_chan(
                system_id,
                MAV_COMP_ID_PERIPHERAL,
                MAVLINK_COMM_0,
                &msg,
                MAV_TYPE_GENERIC,
                MAV_AUTOPILOT_GENERIC,
                base_mode,
                custom_mode,
                MAV_STATE_STANDBY);
            
            int success = send_mavlink_packet(socket_fd, msg, (const struct sockaddr *)(&target_address));

            if (success) {
                spdlog::info("Heartbeat sent!");
            }

            last_time = get_current_time_ms();
        }
    }

    close(socket_fd);
    return 0;
}
