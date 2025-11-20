#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
int main()
{
    sockaddr_in server_ad, client_ad;
    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_ad.sin_family = AF_INET;
    server_ad.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_ad.sin_port = htons(1111);
    int client_addr_size = sizeof(client_ad);
    if (bind(server_socket, (sockaddr*)&server_ad, sizeof(server_ad)) == SOCKET_ERROR) {
        std::cerr << "ERROR: Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    std::cout << "Wait client ..." << std::endl;
    if (listen(server_socket, 1) == SOCKET_ERROR)
    {
        std::cout << "Error" << std::endl;
    }

    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_ad, &client_addr_size);
    std::cout << "Client connect!" << std::endl;
    char buffer[1024];
    int bytesReceived = recv(client_socket, buffer, sizeof(buffer), 0);
    while (bytesReceived == -1)
    {
        bytesReceived = recv(client_socket, buffer, sizeof(buffer), 0);
    }
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    }
    const char* response = "Message received. Response from server.";
    send(client_socket, response, strlen(response), 0);

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}
