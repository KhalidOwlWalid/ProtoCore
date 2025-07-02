#include <iostream>
#include <mavlink/all/mavlink.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#elif __linux__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define print(msg) std::cout << msg;
#define print_line(msg) std::cout << msg << std::endl;

#define SERVER_ADDRESS "127.0.0.1"
#define BUFLEN 2048
#define PORT 14550

static bool run_application = true;

static time_t last_time = 0;

struct sockaddr_in source_addr = {};
socklen_t source_addr_len = sizeof(source_addr);
bool source_addr_set = false;


#ifdef WIN32

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
        #ifdef (WIN32)
        if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
            print_line("WSAStartup for client failed");
            exit(EXIT_FAILURE);
        }
        #endif

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
#elif __linux__
class UDPClient {
public:
    UDPClient() {

        client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

        if (client_socket_fd < 0) {
            printf("Socket error: %s\n", strerror(errno));
        }

        struct sockaddr_in server_address = {};
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        inet_pton(AF_INET, SERVER_ADDRESS, &(server_address.sin_addr)); // listen on all network interfaces
        server_address.sin_port = htons(14550); // default port on the ground

        if (bind(client_socket_fd, (struct sockaddr*)(&server_address), sizeof(server_address)) != 0) {
            printf("bind error: %s\n", strerror(errno));
        }

        // We set a timeout at 100ms to prevent being stuck in recvfrom for too
        // long and missing our chance to send some stuff.
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        if (setsockopt(client_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            printf("setsockopt error: %s\n", strerror(errno));
        }

        struct sockaddr_in src_addr = {};
        socklen_t src_addr_len = sizeof(src_addr);
        bool src_addr_set = false;
    }

    void receive_some(int socket_fd, struct sockaddr_in* src_addr, socklen_t* src_addr_len, bool* src_addr_set)
    {
        // We just receive one UDP datagram and then return again.
        char buffer[2048]; // enough for MTU 1500 bytes

        const int ret = recvfrom(
                socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)(src_addr), src_addr_len);

        // This error will mostly get triggered if we set the timeout of the socket to be too fast
        // It will complain about resource not available
        if (ret < 0) {
            printf("recvfrom error [Error ID: %i]: %s\n", errno, strerror(errno));
        } else if (ret == 0) {
            // peer has done an orderly shutdown
            return;
        }

        *src_addr_set = true;

        mavlink_message_t message;
        mavlink_status_t status;
        for (int i = 0; i < ret; ++i) {
            if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &message, &status) == 1) {

                // printf(
                //     "Received message %d from %d/%d\n",
                //     message.msgid, message.sysid, message.compid);

                switch (message.msgid) {
                case MAVLINK_MSG_ID_HEARTBEAT:
                    handle_heartbeat(&message);
                    break;
                }
            }
        }
    }

    void handle_heartbeat(const mavlink_message_t* message)
    {
        mavlink_heartbeat_t heartbeat;
        mavlink_msg_heartbeat_decode(message, &heartbeat);

        printf("Got heartbeat from ");
        switch (heartbeat.autopilot) {
            case MAV_AUTOPILOT_GENERIC:
                printf("generic");
                break;
            case MAV_AUTOPILOT_ARDUPILOTMEGA:
                printf("ArduPilot");
                break;
            case MAV_AUTOPILOT_PX4:
                printf("PX4");
                break;
            default:
                printf("other");
                break;
        }
        printf(" autopilot\n");
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

        if (sendto(client_socket_fd, (const char *)buffer, len, 0, (sockaddr*)&source_addr, source_addr_len) == -1) {
            print_line("Failed to send from client");
            exit(EXIT_FAILURE);
        }
    }

    void loop() {
        while (run_application) {
            // For illustration purposes we don't bother with threads or async here
            // and just interleave receiving and sending.
            // This only works  if receive_some returns every now and then.
            receive_some(client_socket_fd, &source_addr, &source_addr_len, &source_addr_set);

            // time_t current_time = time(NULL);
            // if (current_time - last_time >= 1) {
            //     send_heartbeat();
            //     request_attitude(1, 1);
            //     last_time = current_time;
            // }
        }

    }

private:
    int client_socket_fd; // сокет клиента
    sockaddr_in server_address; // адрес сервера
};
#endif

int main() {
    UDPClient udpClient;

    #ifdef WIN32
    if (SetConsoleCtrlHandler(control_handler, true)) {
        while (run_application) {
            udpClient.loop();
        }
    } else {
        print_line("Fails to set control handler");
        return 1;
    }
    print_line("Exiting application");
    #elif __linux__
    while (run_application) {
        udpClient.loop();
    }
    #endif
    return 0;
}
