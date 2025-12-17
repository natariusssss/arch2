#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include "utils.h"
int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        die("Error creating socket");
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1111);
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        close(clientSocket);
        die("Invalid address");
    }
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(clientSocket);
        die("Connection failed");
    }
    std::cout << "Connected to server. Type messages (type 'exit' to quit):" << std::endl;
    std::string message;
    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, message)) break;
        if (send(clientSocket, message.c_str(), message.length(), 0) < 0) {
            std::cerr << "Send failed" << std::endl;
            break;
        }
        if (message == "exit") {
            std::cout << "Exiting..." << std::endl;
            break;
        }
        char buffer[1024] = {0};
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cout << "Server closed connection or timeout occurred." << std::endl;
            break;
        }
        buffer[bytesReceived] = '\0';
        std::cout << "[Server]: " << buffer << std::endl;
    }

    close(clientSocket);
    return 0;
}