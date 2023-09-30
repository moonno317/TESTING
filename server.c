#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 9999
#define MAX_MESSAGE_LENGTH 100

void server() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;

    // Create the socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to a specific address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Listening on port %d\n", PORT);

    while (1) {
        // Accept a new client connection
        client_addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Read and print the message from the client
        char message[MAX_MESSAGE_LENGTH];
        ssize_t bytes_received;
        while ((bytes_received = recv(client_fd, message, sizeof(message) - 1, 0)) > 0) {
            message[bytes_received] = '\0';
            printf("Received message from client: %s\n", message);

            // Echo the message back to the client
            ssize_t bytes_sent = send(client_fd, message, bytes_received, 0);
            if (bytes_sent < 0) {
                perror("send failed");
                exit(EXIT_FAILURE);
            }
        }

        if (bytes_received < 0) {
            perror("recv failed");
            exit(EXIT_FAILURE);
        }

        // Close the client socket
        close(client_fd);
    }

    // Close the server socket
    close(server_fd);
}

int main() {
    server();
    return 0;
}