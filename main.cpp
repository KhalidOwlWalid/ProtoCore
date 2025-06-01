#include <iostream>
#include <winsock2.h>
#include <mavlink/common/mavlink.h>
using namespace std;

#define print(msg) std::cout << msg;
#define print_line(msg) std::cout << msg << std::endl;

#define SERVER "127.0.0.1" // ip-адрес сервера (локальный хост)
#define BUFLEN 512 // максимальная длина ответа
#define PORT 14550 // порт для приема данных

class UDPClient {
public:
    UDPClient() {
        cout << "Initializing the UDP client \n";
        if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
            cout << "UDP client failed to initialize" << WSAGetLastError() << "\n";
            exit(EXIT_FAILURE);
        }
        cout << "Creating the UDP socket \n";

        if ((client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
            cout << "Failed to create UDP socket descriptor" << WSAGetLastError() << "\n";
            exit(EXIT_FAILURE);
        }

        memset((char*)&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        server.sin_addr.S_un.S_addr = inet_addr(SERVER);
    }

    ~UDPClient() {
        closesocket(client_socket);
        WSACleanup();
    }

    void start() {
        while (true) {
            char message[200] = "Hello world from the client";

            if (sendto(client_socket, message, strlen(message), 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR) {
                cout << "Failed to send from client" << WSAGetLastError() << "\n";
                exit(EXIT_FAILURE);
            }

            char buffer[BUFLEN] = {};
            int slen = sizeof(sockaddr_in);
            int recv_bytes;

            if ((recv_bytes = recvfrom(client_socket, buffer, BUFLEN, 0, (sockaddr*)&server, &slen)) == SOCKET_ERROR) {
                cout << "Socket error" << WSAGetLastError() << "\n";
                exit(EXIT_FAILURE);
            }

            cout << "Received bytes: " << recv_bytes << "\n";
        }
    }

private:
    WSADATA ws; // данные winsock
    SOCKET client_socket; // сокет клиента
    sockaddr_in server; // адрес сервера
};

int main() {
    // system("title UDP CLIENT SIDE");
    // setlocale(0, "");
    UDPClient udpClient;
    udpClient.start();
}