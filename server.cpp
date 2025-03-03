#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>

#define PORT 9090
#define BUFFER_SIZE 1024

std::vector<int> clients;
std::mutex clients_mutex;

// Function to broadcast messages to all clients
void broadcast(const std::string &message, int sender_fd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int client_fd : clients) {
        if (client_fd != sender_fd) {
            send(client_fd, message.c_str(), message.size(), 0);
        }
    }
}

// Function to handle each client
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }

        std::string client_message = buffer;
        std::cout << "Client " << client_socket << ": " << client_message << std::endl;

        std::string formatted_message = "Client " + std::to_string(client_socket) + ": " + client_message;
        broadcast(formatted_message, client_socket);
    }

    // Remove client from list
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    sockaddr_in server_addr{}, client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Bind failed\n";
        return -1;
    }

    if (listen(server_socket, 5) == -1) {
        std::cerr << "Listen failed\n";
        return -1;
    }

    std::cout << "Chat Server running on port " << PORT << "...\n";

    while (true) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket == -1) {
            std::cerr << "Failed to accept client\n";
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client_socket);
        }

        std::cout << "New client connected: " << client_socket << "\n";
        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}
