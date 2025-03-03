#include <iostream>
#include <thread>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9090
#define BUFFER_SIZE 1024

void receive_messages(int socket_fd) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (true) {
        bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);
        if (bytes_received == 0) {
            std::cout << "Server disconnected." << std::endl;
            break;
        }
        if (bytes_received < 0) {
            std::cerr << "Error receiving message: " << strerror(errno) << std::endl;
            break;
        }

        buffer[bytes_received] = '\0';  // Null-terminate the string
        std::cout << "Message from server: " << buffer << std::endl;
    }
}

int main() {
    int socket_fd;
    struct sockaddr_in server_addr;

    // Create socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    // Connect to server
    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    std::cout << "Connected to server " << SERVER_IP << " on port " << SERVER_PORT << std::endl;

    std::string username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, username);

    // Start receiving messages in a separate thread
    std::thread(receive_messages, socket_fd).detach();

    char buffer[BUFFER_SIZE];

    while (true) {
        std::string message;
        std::cout << "You: ";
        std::getline(std::cin, message);

        if (message == "exit") {
            std::cout << "Disconnecting..." << std::endl;
            break;
        }

        // Prepend username to the message
        std::string message_to_send = username + ": " + message;

        // Send the message
        if (send(socket_fd, message_to_send.c_str(), message_to_send.length(), 0) == -1) {
            std::cerr << "Error sending message: " << strerror(errno) << std::endl;
        }
    }

    close(socket_fd);
    return 0;
}
