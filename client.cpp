#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1111);
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        close(clientSocket);
        return 1;
    }

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        close(clientSocket);
        return 1;
    }

    std::cout << "Connected to server. Type messages (type 'exit' to quit):" << std::endl;

    std::string message;
    while (true) {
        std::cout << ">>> ";
        std::getline(std::cin, message);


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
            std::cout << "Server closed connection or error occurred." << std::endl;
            break;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "[Server]: " << buffer << std::endl;
    }

    close(clientSocket);
    return 0;
}
