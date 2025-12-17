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
#include <sys/time.h>
#include "protocol.h"
#include "utils.h"
const int max_threads = 20;
std::queue<std::pair<int, std::string>> msg_queue;
std::mutex msg_queue_mutex;
std::condition_variable msg_cond;
std::queue<int> client_queue;
std::mutex queue_mutex;
std::condition_variable condition;
void handle_client(int client_socket) {
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    while (true) {
        char buffer[1024];
        int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            break;
        }
        buffer[bytesReceived] = '\0';
        std::string msg_str(buffer);
        {
            std::lock_guard<std::mutex> lock(msg_queue_mutex);
            msg_queue.push({client_socket, msg_str});
        }
        msg_cond.notify_one();

        if (msg_str == "exit") break;
    }
    close(client_socket);
}
void worker_thread() {
    while (true) {
        int client_socket;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [] { return !client_queue.empty(); });
            client_socket = client_queue.front();
            client_queue.pop();
        }
        handle_client(client_socket);
    }
}
void Send_Messages() {
    while (true) {
        std::pair<int, std::string> task;
        {
            std::unique_lock<std::mutex> lock(msg_queue_mutex);
            msg_cond.wait(lock, [] { return !msg_queue.empty(); });
            task = std::move(msg_queue.front());
            msg_queue.pop();
        }
        if (task.second == "ping") {
            const char* res = "pong";
            send(task.first, res, strlen(res), 0);
        } else if (task.second == "exit") {
            std::cout << "Server shutting down by exit command." << std::endl;
            exit(0);
        } else {
            const char* res = "[Server]: Message received";
            send(task.first, res, strlen(res), 0);
        }
    }
}
int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) die("Socket failed");
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in server_ad{};
    server_ad.sin_family = AF_INET;
    server_ad.sin_addr.s_addr = INADDR_ANY;
    server_ad.sin_port = htons(1111);
    if (bind(server_socket, (sockaddr*)&server_ad, sizeof(server_ad)) == -1) die("Bind failed");
    listen(server_socket, 10);
    std::cout << "Wait client ..." << std::endl;
    std::thread(Send_Messages).detach();
    for (int i = 0; i < max_threads; i++) {
        std::thread(worker_thread).detach();
    }
    while (true) {
        int client_sock = accept(server_socket, nullptr, nullptr);
        if (client_sock >= 0) {
            std::lock_guard<std::mutex> lock(queue_mutex);
            client_queue.push(client_sock);
            condition.notify_one();
        }
    }
    return 0;
}