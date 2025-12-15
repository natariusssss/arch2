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
std::queue<std::pair<int, std::string>> msg_queue;
std::vector<std::thread> client_threads;
std::queue<int> client_queue;
std::mutex queue_mutex;
std::mutex msg_queue_mutex;
std::condition_variable condition;
std::condition_variable msg_cond;
void handle_client(int client_socket)
{
    while (true)
    {
        char buffer[1024];
        int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if(bytesReceived == 0){ break; }
        buffer[bytesReceived] = '\0';
        if (bytesReceived > 0)
        {
            std::cout << "Received: " << buffer << std::endl;
        }
        {
            std::lock_guard<std::mutex> lock(msg_queue_mutex);
            msg_queue.push(std::make_pair(client_socket, buffer));
        }
        msg_cond.notify_one();
    }
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
void Send_Messages()
{
    while (true)
    {
        std::pair<int, std::string> value;
        {
            std::unique_lock<std::mutex> lock(msg_queue_mutex);
            msg_cond.wait(lock, [] {return !msg_queue.empty();});
            value = std::move(msg_queue.front());
            msg_queue.pop();
        }
        if (value.second == "exit")
        {
            close(value.first);
        }
        if (value.second == "ping")
        {
            const char* response = "pong";
            send(value.first, response, strlen(response), 0);
            continue;
        }
        const char* response = "Message received. Response from server - pong";
        send(value.first, response, strlen(response), 0);
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
    std::thread thread(Send_Messages);
    thread.detach();
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



