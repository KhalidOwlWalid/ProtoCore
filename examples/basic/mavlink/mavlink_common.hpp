#include "../common.hpp"
#include "mavlink/all/mavlink.h"

#include <time.h>
#include <chrono>

#define MAVLINK_SERVER_PORT 14550
#define MAVLINK_SERVER_ADDRESS "127.0.0.1"
#define ARDUPILOT_SITL_ADDRESS MAVLINK_SERVER_ADDRESS

// Quick helper function for the use of the examples
using timer = std::chrono::system_clock;

timer::duration get_current_time_since_epoch() {
    return timer::now().time_since_epoch();
}

auto get_current_time_ms() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return milliseconds;
}

int setup_sock_fd(int &socket_fd, const int domain, const int socket_type) {
    socket_fd = socket(domain, socket_type, 0);
    if (socket_fd < 0) {
        spdlog::error("Socket fails to be created with error, {}", strerror(errno));
        return 0;
    }
    return 1;
}

void setup_address(struct sockaddr_in &sock_address, const int domain, const char *char_address, const int port) {
    memset(&sock_address, 0, sizeof(sock_address));
    sock_address.sin_family = domain;
    inet_pton(domain, char_address, &(sock_address.sin_addr));
    sock_address.sin_port = htons(port);
}

int bind_socket(int &socket_fd, struct sockaddr_in &socket_address) {
    // Bind the socket to listen to incoming data
    if (bind(socket_fd, (struct sockaddr*)(&socket_address), sizeof(socket_address)) < 0) {
        spdlog::error("Bind failed with error, {}", strerror(errno));
        close(socket_fd);
        return 0;
    }
    return 1;
}

int init_server(int &socket_fd, struct sockaddr_in &server_address, const int domain, const int socket_type, const char *address, const int port) {
    // Setup file descriptor
    // Use of UDP protocol
    // socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    // if (socket_fd < 0) {
    //     spdlog::error("Socket fails to be created with error, {}", strerror(errno));
    // }
    setup_sock_fd(socket_fd, domain, socket_type);
    setup_address(server_address, domain, address, port);
    bind_socket(socket_fd, server_address);

    // Bind the socket to listen to incoming data
    // if (bind(socket_fd, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0) {
    //     spdlog::error("Bind failed with error, {}", strerror(errno));
    //     close(socket_fd);
    //     return 0;
    // }

    return 1;
}

int send_mavlink_packet(int &socket_fd, mavlink_message_t &msg, const struct sockaddr* target_address) {
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    const int len = mavlink_msg_to_send_buffer(buffer, (const mavlink_message_t *)&msg);
    int ret = sendto(socket_fd, buffer, len, 0, target_address, sizeof(*target_address));
    if (ret != len) {
        spdlog::warn("sendto error: {}", strerror(errno));
        return 0;
    } 
}

ssize_t receive_mavlink_packet(int &socket_fd, struct sockaddr *recv_address, uint8_t *buffer) {
    socklen_t address_len = sizeof(*recv_address);
    ssize_t recv_n_byte = recvfrom(socket_fd, buffer, 2048, 0, recv_address, &address_len);
    return recv_n_byte;
}
