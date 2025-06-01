#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mavlink/all/mavlink.h>
// #include <mavlink.h>
// #include <all.h>
#include <time.h>

#include <windows.h>

#define print(msg) std::cout << msg;
#define print_line(msg) std::cout << msg << std::endl;

#define SERVER "127.0.0.1" // ip-адрес сервера (локальный хост)
#define BUFLEN 2048 // максимальная длина ответа
#define PORT 14550 // порт для приема данных

static bool run_application = true;

static time_t last_time = 0;

struct sockaddr_in source_addr = {};
socklen_t source_addr_len = sizeof(source_addr);

BOOL WINAPI control_handler(DWORD forward_control_type) {
    switch (forward_control_type) {
        case CTRL_C_EVENT:
            print_line("CTRL+C event");
            run_application = false;
            print("run_application is ");
            print(run_application);
            print_line("");
            return true;
        default:
            return false;
    }
}

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
            print_line("Connect error");
            exit(EXIT_FAILURE);
        }

        print_line("Successfully initialized the UDP client");
    }

    ~UDPClient() {
        closesocket(client_socket_fd);
        print_line("Closing socket");
        WSACleanup();
    }

    void request_attitude(uint8_t target_system_id, uint8_t target_component_id) {
        mavlink_message_t msg;

        // Pack the request message
        mavlink_msg_command_long_pack(
            42,
            MAV_COMP_ID_PERIPHERAL,
            &msg,
            target_system_id,
            target_component_id,
            MAV_CMD_REQUEST_MESSAGE,
            0,
            MAVLINK_MSG_ID_ATTITUDE,
            0, 0, 0, 0, 0, 0
        );

        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        const int len = mavlink_msg_to_send_buffer(buffer, &msg);
        print_line("Requesting for data");
        sendto(client_socket_fd, (const char *)(buffer), len, 0, (sockaddr *)&source_addr, source_addr_len);
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

        if (sendto(client_socket_fd, (const char *)buffer, len, 0, (sockaddr*)&source_addr, source_addr_len) == SOCKET_ERROR) {
            print_line("Failed to send from client");
            exit(EXIT_FAILURE);
        }
    }

    void loop() {
        while (run_application) {
            print("run_application inside loop is ");
            print(run_application);
            print_line("");
            // Will return n bytes if successful
            char recv_buffer[2048];
            const int result = recvfrom(client_socket_fd, recv_buffer, 2048, 0, (sockaddr*)&source_addr, &source_addr_len); 
            if (result == SOCKET_ERROR) {
                print("Failed to receive data from server with error: ");
                print(WSAGetLastError());
                print_line("");
                exit(EXIT_FAILURE);
            }
            print_line(result);

            time_t current_time = time(NULL);
            if (current_time - last_time >= 1) {
                send_heartbeat();
                request_attitude(1, 1);
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
    UDPClient udpClient;

    if (SetConsoleCtrlHandler(control_handler, true)) {
        while (run_application) {
            udpClient.loop();
        }
    } else {
        print_line("Fails to set control handler");
        return 1;
    }
    print_line("Exiting application");
    return 0;
}