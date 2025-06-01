#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mavlink/common/mavlink.h>

#define print(msg) std::cout << msg;
#define print_line(msg) std::cout << msg << std::endl;

#define SERVER "127.0.0.1" // ip-адрес сервера (локальный хост)
#define BUFLEN 2048 // максимальная длина ответа
#define PORT 14550 // порт для приема данных

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

        memset((char*)&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT);
        server_address.sin_addr.S_un.S_addr = inet_addr(SERVER);
    }

    ~UDPClient() {
        closesocket(client_socket_fd);
        WSACleanup();
    }

    void start() {
        while (true) {
            char buffer[BUFLEN] = {};
            int server_addr_size = sizeof(server_address);
            char message[200] = "Hello world from the client";

            if (sendto(client_socket_fd, message, strlen(message), 0, (sockaddr*)&server_address, server_addr_size) == SOCKET_ERROR) {
                print_line("Failed to send from client");
                exit(EXIT_FAILURE);
            }

            // Will return n bytes if successful
            const int result = recvfrom(client_socket_fd, buffer, BUFLEN, 0, (sockaddr*)&server_address, &server_addr_size); 
            if (result == SOCKET_ERROR) {
                print("Failed to receive data from server with error: ");
                print(WSAGetLastError());
                print_line("");
                exit(EXIT_FAILURE);
            }

            print("Received bytes: ");
            print(result);
            print_line("");


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