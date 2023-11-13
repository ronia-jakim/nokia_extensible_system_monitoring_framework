#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 256

void send_data(int server_socket) {
    char data[] = "Client data to send";
    send(server_socket, data, strlen(data), 0);
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Client socket error");
        exit(1);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Server connection error");
        exit(1);
    }

    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        // Receive data from the server
        if (recv(client_socket, buffer, sizeof(buffer) - 1, 0) <= 0) {
            perror("Error receiving command from server");
            break;
        }

        // Check if the received command is "GATHER_INFO"
        if (strcmp(buffer, "GATHER_INFO") == 0) {
            // Send data to the server
            send_data(client_socket);
            memset(buffer, 0, sizeof(buffer)); // Clear the buffer
        }

        // Maybe add a delay to avoid constant checking and reduce CPU usage??
        // sleep(1);
    }

    close(client_socket);
    return 0;
}
