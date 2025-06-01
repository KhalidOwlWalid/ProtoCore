#include <iostream>
#include <winsock2.h>
using namespace std;

#define BUFLEN 512 // размер буфера
#define PORT 8888 // порт сервера

class UDPServer {
public:
    UDPServer() {
        // инициализация winsock
        cout << "Initializing the UDP server" << endl;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            cout << "UDP server init failed" << WSAGetLastError() << endl;
            exit(0);
        }

        // создание сокета
        if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
            cout << "Failed to create the UDP socket" << WSAGetLastError() << endl;
            exit(EXIT_FAILURE);
        }

        // настройка структуры sockaddr_in
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(PORT);

        // привязка сокета
        if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
            cout << "Failed to bind the server socket" << WSAGetLastError() << endl;
            exit(EXIT_FAILURE);
        }
    }

    ~UDPServer() {
        closesocket(server_socket);
        WSACleanup();
    }

    void start() {
        while (true) {
            char buffer[1024];
            sockaddr_in client;
            int client_len = sizeof(client);
            cout << "Sending message from server" << endl;
            // 1. Wait for a client message
            int bytes_received = recvfrom(
                server_socket,
                buffer,
                BUFLEN,
                0,
                (sockaddr*)&client,
                &client_len
            );

            if (bytes_received == SOCKET_ERROR) {
                cout << "recvfrom() failed: " << WSAGetLastError() << endl;
                continue;
            }

            // 2. Print the received message
            buffer[bytes_received] = '\0';
            cout << "Received from client: " << buffer << endl;

            char message[200] = "Hello World";

            // отправка ответа клиенту
            if (sendto(server_socket, message, strlen(message), 0, (sockaddr*)&client, sizeof(sockaddr_in)) == SOCKET_ERROR) {
                cout << "Failed to send data from server " << WSAGetLastError() << endl;
                exit(EXIT_FAILURE);
            }
        }
    }

private:
    WSADATA wsa{};
    SOCKET server_socket = 0;
    sockaddr_in server{}, client{};
    bool exitRequested = false;
};

int main() {
    UDPServer udpServer;
    udpServer.start();
}