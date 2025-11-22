#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    sockaddr_in server_ad, client_ad;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_ad.sin_family = AF_INET;
    server_ad.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_ad.sin_port = htons(1111);
    if (bind(server_socket, (sockaddr*)&server_ad, sizeof(server_ad)) == -1) {
        std::cerr << "ERROR: Bind failed" << std::endl;
        close(server_socket);
        return 1;
    }
    std::cout << "Wait client ..." << std::endl;
    if (listen(server_socket, 1) == -1) {
        std::cerr << "ERROR: Listen failed" << std::endl;
        close(server_socket);
        return 1;
    }
    socklen_t client_addr_size = sizeof(client_ad);
    int client_socket = accept(server_socket, (sockaddr*)&client_ad, &client_addr_size);
    std::cout << "Client connect!" << std::endl;
    char buffer[1024];
    int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    }
    const char* response = "Message received. Response from server - pong";
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
    close(server_socket);
    return 0;
}
