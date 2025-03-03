#include <iostream>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <thread>
#include <algorithm>  // Added to resolve std::remove issue
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUFFER_SIZE 1024

std::vector<int> clients;

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';  // Null-terminate the string
        std::cout << "Message received: " << buffer << std::endl;

        // Send the message to all connected clients
        for (int client : clients) {
            if (client != client_socket) {  // Don't send the message back to the sender
                send(client, buffer, bytes_received, 0);
            }
        }
    }

    if (bytes_received == 0) {
        std::cout << "Client disconnected." << std::endl;
    } else if (bytes_received < 0) {
        std::cerr << "Error receiving message: " << strerror(errno) << std::endl;
    }

    // Remove client from the list using the correct method
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());

    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to all available interfaces
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Chat Server running on port " << PORT << "..." << std::endl;

    while (true) {
        // Accept incoming client connections
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        std::cout << "New client connected." << std::endl;

        clients.push_back(client_socket);
        std::thread(handle_client, client_socket).detach();
    }

    close(server_fd);
    return 0;
}
