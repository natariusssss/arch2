#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

int main() {

    WSAData wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

  
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1111); 
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); 

    std::cout << "Connecting to server..." << std::endl;
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;


    const char* message = "ping";
    int sendResult = send(clientSocket, message, strlen(message), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Sent: " << message << std::endl;

 
    char buffer[1024];
    int recvResult = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (recvResult == SOCKET_ERROR) {
        std::cerr << "Receive failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    else if (recvResult == 0) {
        std::cout << "Server closed the connection." << std::endl;
    }
    else {
        buffer[recvResult] = '\0'; 
        std::cout << "Received: " << buffer << std::endl;
    }


    closesocket(clientSocket);
    WSACleanup();

    std::cout << "Client finished." << std::endl;
    return 0;
}
