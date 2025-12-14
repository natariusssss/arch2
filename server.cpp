#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
const int max_threads = 20;
std::queue<std::string> messages;
std::vector<std::thread> client_threads;
std::queue<int> client_queue;
std::mutex queue_mutex;
std::condition_variable condition;
void handle_client(int client_socket)
{
    while (true)
    {
        char buffer[1024];
        int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        buffer[bytesReceived] = '\0';
        if (bytesReceived > 0)
        {
            std::cout << "Received: " << buffer << std::endl;
        }
        if (std::strcmp(buffer, "exit") == 0)
        {
            break;
        }
        if (std::strcmp(buffer, "ping") == 0)
        {
            const char* response = "pong";
            send(client_socket, response, strlen(response), 0);
            continue;
        }
        const char* response = "Message received. Response from server - pong";
        send(client_socket, response, strlen(response), 0);
    }
    close(client_socket);
}
void worker_thread() 
{
    while (true) {
        int client_socket;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [] {return !client_queue.empty(); });
            if (client_queue.empty()) return;

            client_socket = std::move(client_queue.front());
            client_queue.pop();
        }
        handle_client(client_socket);
    }
}
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
    if (listen(server_socket, 2) == -1) {
        std::cerr << "ERROR: Listen failed" << std::endl;
        close(server_socket);
        return 1;
    }
    for (size_t i = 0; i < max_threads; i++)
    {
        client_threads.emplace_back(worker_thread);
        client_threads.back().detach();
    }
    while(true)
    {
        socklen_t client_addr_size = sizeof(client_ad);
        int client_socket = accept(server_socket, (sockaddr*)&client_ad, &client_addr_size);
        std::cout << "Client connect!" << std::endl;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            client_queue.push(client_socket);
        }
        condition.notify_one();
        
    }   
    close(server_socket);
    return 0;
}


