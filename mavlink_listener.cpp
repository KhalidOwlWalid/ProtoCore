#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mavlink/common/mavlink.h>
#include <time.h>

#define print(msg) std::cout << msg;
#define print_line(msg) std::cout << msg << std::endl;

#define SERVER "127.0.0.1" // ip-адрес сервера (локальный хост)
#define BUFLEN 2048 // максимальная длина ответа
#define PORT 14550 // порт для приема данных

static time_t last_time = 0;

struct sockaddr_in source_addr = {};
socklen_t source_addr_len = sizeof(source_addr);

class UDPClient {
public:
    UDPClient() {
        std::cout << "Initializing the UDP client \n";
        if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
            print_line("WSAStartup for client failed");
            exit(EXIT_FAILURE);
        }

        if ((client_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
            print_line("Failed to create UDP socket descriptor");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER);

        if (bind(client_socket_fd, (const sockaddr *)(&server_addr), sizeof(server_addr)) != 0) {
            print_line("Bind error");
            exit(EXIT_FAILURE);
        }
    }

    ~UDPClient() {
        closesocket(client_socket_fd);
        WSACleanup();
    }

    void send_heartbeat() {

        mavlink_message_t message;
        const uint8_t system_id = 42;
        const uint8_t base_mode = 0;
        const uint8_t custom_mode = 0;

        mavlink_msg_heartbeat_pack_chan(
            system_id,
            MAV_COMP_ID_PERIPHERAL,
            MAVLINK_COMM_0,
            &message,
            MAV_TYPE_GENERIC,
            MAV_AUTOPILOT_GENERIC,
            base_mode,
            custom_mode,
            MAV_STATE_STANDBY
        );

        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        const int len = mavlink_msg_to_send_buffer(buffer, &message);

        sendto(client_socket_fd, (const char *)buffer, strlen((const char *)buffer), 0, (sockaddr*)&source_addr, source_addr_len); 
        // if (sendto(client_socket_fd, (const char *)buffer, strlen((const char *)buffer), 0, (sockaddr*)&source_addr, source_addr_len) == SOCKET_ERROR) {
        //     print_line("Failed to send from client");
        //     exit(EXIT_FAILURE);
        // }
    }

    // void send_heartbeat()
    // {
        // mavlink_message_t message;

        // const uint8_t system_id = 42;
        // const uint8_t base_mode = 0;
        // const uint8_t custom_mode = 0;
        // mavlink_msg_heartbeat_pack_chan(
        //     system_id,
        //     MAV_COMP_ID_PERIPHERAL,
        //     MAVLINK_COMM_0,
        //     &message,
        //     MAV_TYPE_GENERIC,
        //     MAV_AUTOPILOT_GENERIC,
        //     base_mode,
        //     custom_mode,
        //     MAV_STATE_STANDBY);

    //     uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    //     const int len = mavlink_msg_to_send_buffer(buffer, &message);

    //     int ret = sendto(client_socket_fd, (const char *)buffer, len, 0, (sockaddr*)&source_addr, source_addr_len);
    //     if (ret != len) {
    //         printf("sendto error: %s\n", strerror(errno));
    //     } else {
    //         printf("Sent heartbeat\n");
    //     }
    // }

    void start() {
        while (true) {

            // Will return n bytes if successful
            char recv_buffer[2048];
            const int result = recvfrom(client_socket_fd, recv_buffer, BUFLEN, 0, (sockaddr*)&source_addr, &source_addr_len); 
            if (result == SOCKET_ERROR) {
                print("Failed to receive data from server with error: ");
                print(WSAGetLastError());
                print_line("");
                exit(EXIT_FAILURE);
            }

            time_t current_time = time(NULL);
            if (current_time - last_time >= 1) {
                send_heartbeat();
                last_time = current_time;
            }
        }
    }

private:
    WSADATA ws; // данные winsock
    SOCKET client_socket_fd; // сокет клиента
    sockaddr_in server_address; // адрес сервера
};

int main() {
    // system("title UDP CLIENT SIDE");
    // setlocale(0, "");
    UDPClient udpClient;
    udpClient.start();
}