#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdbool.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 256
#define PULL_TIME 2
#define FILENAME "pomiary.txt"

void send_info(int client_socket) {
    char message[] = "GATHER_INFO";
    send(client_socket, message, strlen(message), 0);
}

void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    free(arg); // Free the memory allocated for the client socket

    printf("Client connected. Socket: %d\n", client_socket);

    char buffer[BUFFER_SIZE];
    bool z = 0;

    send_info(client_socket);

    FILE* file = fopen(FILENAME, "a");
    if (!file) {
        perror("Error opening file");
        close(client_socket);
        return NULL;
    }

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);

        // Set up a timeout of 60 seconds
        struct timeval timeout;
        timeout.tv_sec = PULL_TIME;
        timeout.tv_usec = 0;

        int result = select(client_socket + 1, &read_fds, NULL, NULL, &timeout);

        if (result == -1) {
            perror("Error in select");
            break;
        } else if (result == 0) {
            // Timeout occurred, no data received within 60 seconds
            send_info(client_socket);
            if (z == 0) {
                printf("No data received within %d seconds. Continuing...\n", PULL_TIME);
            }
            z = 0;
            continue;
        } else {
            // Data is available, receive it
            if (recv(client_socket, buffer, sizeof(buffer) - 1, 0) <= 0) {
                perror("Error receiving data from client");
                break;
            }
            // Process the received data from the client as needed
            printf("Received data from client: %s\n", buffer);

            // Save data to file
            fprintf(file, "%s\n", buffer);

            fflush(file);

            z = 1;
        }
    }

    fclose(file);
    close(client_socket);
    printf("Client disconnected. Socket: %d\n", client_socket);

    return NULL;
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Server socket error");
        exit(1);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Error assigning address");
        exit(1);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        exit(1);
    }

    while (1) {
        int* client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, NULL, NULL);
        if (*client_socket > 0) {
            pthread_t client_thread;
            if (pthread_create(&client_thread, NULL, handle_client, (void*)client_socket) != 0) {
                perror("Error creating client thread");
                close(*client_socket);
                free(client_socket);
            }
        }
    }

    close(server_socket);
    return 0;
}
