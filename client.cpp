#include <iostream>
#include <string>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define SERVER_IP "127.0.0.1"
#define PORT 9090
#define BUFFER_SIZE 1024

// Function to receive messages from the server
void receive_messages(int client_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cerr << "Disconnected from server\n";
            break;
        }
        std::cout << "\n" << buffer << "\n> ";
        std::cout.flush();
    }
}

int main() {
    int client_socket;
    sockaddr_in server_addr{};

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Socket creation failed\n";
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Connection to server failed\n";
        return -1;
    }

    std::cout << "Connected to the chat server\n> ";

    std::thread receiver(receive_messages, client_socket);
    receiver.detach();

    std::string message;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);
        if (message == "exit") {
            break;
        }
        send(client_socket, message.c_str(), message.size(), 0);
    }

    close(client_socket);
    return 0;
}
