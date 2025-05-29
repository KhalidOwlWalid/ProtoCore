#pragma once
#include <winsock2.h>
#include <iostream>

// #pragma comment(lib, "ws_232.lib")

#define print(msg) std::cout << msg;
#define print_line(msg) std::cout << msg << std::endl;

#define LOOPBACK_ADDRESS "127.0.0.1"

void test_print() {

    WSAData wsa_data;
    switch(WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
        case 0:
            print_line("Successfully initialize the UDP connection.");
            break;
        case WSASYSNOTREADY:
            break;
        default:
            print_line("Nothing happened");
    };

    // AF_INET - Address family that specifies IPv4 address
    SOCKET server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);

    // #define s_addr	S_un.S_addr
    // s_addr is a macro to shorten the use of the union used in sin_addr
    // So instead of having to type
    // server_address.sin_addr.S_un.S_addr, its shorten
    server_address.sin_addr.s_addr = inet_addr(LOOPBACK_ADDRESS);

    switch(bind(server_socket, (sockaddr *)&server_address, sizeof(server_address))) {
        case 0:
            print("Socket successfully binded to an address of ");
            print(inet_ntoa(server_address.sin_addr));
            break;
        default:
            print_line("Ignore");
    };
}
