#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>   
#include <cstring>    
#include <cerrno>     
#include <string>

int main() {

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return 1;
    }


    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1111); // Порт 1111
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        close(clientSocket);
        return 1;
    }

    std::cout << "Connecting to server..." << std::endl;
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connect failed: " << strerror(errno) << std::endl;
        close(clientSocket);
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;


    const char* message = "ping";
    ssize_t sendResult = send(clientSocket, message, strlen(message), 0);
    if (sendResult < 0) {
        std::cerr << "Send failed: " << strerror(errno) << std::endl;
        close(clientSocket);
        return 1;
    }

    std::cout << "Sent: " << message << std::endl;


    char buffer[1024] = {0};
    ssize_t recvResult = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (recvResult < 0) {
        std::cerr << "Receive failed: " << strerror(errno) << std::endl;
    } else if (recvResult == 0) {
        std::cout << "Server closed the connection." << std::endl;
    } else {
        buffer[recvResult] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    }


    close(clientSocket);

    std::cout << "Client finished." << std::endl;
    return 0;
}
