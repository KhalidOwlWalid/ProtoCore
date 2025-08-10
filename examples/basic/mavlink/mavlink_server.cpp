#include "mavlink_common.hpp"

int main() {

    spdlog::info("Mavlink Server example");

    // Setup file descriptor
    // Use of UDP protocol
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock_fd < 0) {
        spdlog::error("Socket fails to be created with error, {}", strerror(errno));
    } 

    memset(&server_address, 0, sizeof(client_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(server_address.sin_addr));
    server_address.sin_port = htons(14550);

    if (bind(sock_fd, (struct sockaddr*)(&server_address), sizeof(server_address)) < 0) {
        spdlog::error("Bind failed with error, {}", strerror(errno));
        close(sock_fd);
        return 0;
    }

    // spdlog::info("Mavlink server listening on port {}:{}", server_address.sin

    while (run_application) {

        ssize_t recv_len = recvfrom(sock_fd, buffer, 2048, 0, (struct sockaddr*)(&server_address), &address_len); 

        if (recv_len < 0) {
            spdlog::error("recvfrom failed with error, {}", strerror(errno));
            break;
        }

        mavlink_message_t mav_message;
        mavlink_status_t mav_status;
        uint8_t some_val = 1;
        
        for (size_t i = 0; i < recv_len; i++) {
            if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &mav_message, &mav_status)) {
                switch (mav_message.msgid) {
                    case MAVLINK_MSG_ID_HEARTBEAT:
                        spdlog::info("Receiving data from Heartbeat {}:{}", mav_message.sysid, mav_message.compid);
                        std::cout << some_val << std::endl;
                    case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
                        spdlog::info("Received data for target position");
                        mavlink_position_target_local_ned_t target_local_pos_ned;
                        mavlink_msg_position_target_local_ned_decode(&mav_message, &target_local_pos_ned);
                        spdlog::info("Target Local POS NED: {}, {}", target_local_pos_ned.x, target_local_pos_ned.y);
                    case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
                        spdlog::info("Receiving data from Local Position NED with {} bytes.", recv_len);
                        mavlink_local_position_ned_t local_pos_ned;
                        mavlink_msg_local_position_ned_decode(&mav_message, &local_pos_ned);
                        spdlog::info("Data received from NED: {}, {}, {}", local_pos_ned.x, local_pos_ned.y, local_pos_ned.z);
                    case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
                        mavlink_param_request_read_t param_request;
                        mavlink_msg_param_request_read_decode(&mav_message, &param_request);
                        spdlog::info("Data: {}", param_request.param_index);
                    case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
                        spdlog::info("Receiving data from Nav Controller Output with {} bytes.", recv_len);
                        mavlink_nav_controller_output_t nav_controller_output;
                        mavlink_msg_nav_controller_output_decode(&mav_message, &nav_controller_output);
                        mavlink_message_interval_t mav_interval_t;
                        mavlink_msg_message_interval_decode(&mav_message, &mav_interval_t);
                        spdlog::info("Data received from Nav Controller Output: {}, {}, {} with interval of {} us",  nav_controller_output.nav_roll, nav_controller_output.nav_pitch, nav_controller_output.nav_bearing, mav_interval_t.interval_us);
                    case MAVLINK_MSG_ID_PID_TUNING:
                        spdlog::info("Receiving data from PID Tuning Output with {} bytes.", recv_len);
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

    close(sock_fd);
    return 0;
}
