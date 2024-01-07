﻿#include <iostream>
#include <cstring>
#include <string>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 256
#define PULL_TIME 10
#define FILENAME "pomiary.txt"
#define SEQUENCE "@" 


void manage(char *buffer, FILE *file) {
    std::cout << "Received data from client: " << buffer << std::endl;
    fprintf(file, "%s\n", buffer);
    fflush(file);
}

void send_info(int client_socket) {
    char message[] = "GATHER_INFO";
    send(client_socket, message, strlen(message), 0);
}

void handle_client(int client_socket) {
    std::cout << "Client connected. Socket: " << client_socket << std::endl;

    char buffer[BUFFER_SIZE];
    char prebuffer[BUFFER_SIZE];
    prebuffer[0] = '\0';
    bool z = false;

    send_info(client_socket);

    FILE* file = fopen(FILENAME, "a");
    if (!file) {
        perror("Error opening file");
        close(client_socket);
        return;
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);

        
        auto start_time = std::chrono::steady_clock::now();
        std::chrono::seconds timeout(PULL_TIME);

        struct timeval timeval_timeout;
        timeval_timeout.tv_sec = PULL_TIME;
        timeval_timeout.tv_usec = 0;

        int result = select(client_socket + 1, &read_fds, nullptr, nullptr, &timeval_timeout);

        if (result == -1) {
            perror("Error in select");
            break;
        } else if (result == 0) {
            // Timeout occurred, no data received within PULL_TIME seconds
            send_info(client_socket);
            if (!z) {
                std::cout << "No data received within " << PULL_TIME << " seconds. Continuing..." << std::endl;
            }
            z = false;
            continue;
        } else {
            // Data is available, receive it
            if (recv(client_socket, buffer, sizeof(buffer) - 1, 0) <= 0) {
                perror("Error receiving data from client");
                break;
            }
            char *token;
            strcat(prebuffer, buffer);
            token = strtok(prebuffer, "@");
            char *prevToken = NULL;
            while (token != NULL) {
                if(prevToken != NULL)
                    manage(prevToken,file);
                prevToken = token;
                token = strtok(NULL, "@");
                if (token == NULL) {
                    prebuffer[0]='\0';
                    strcat(prebuffer, prevToken);
                }
            }

            z = true;
        }
    }
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Server socket error");
        exit(1);
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, reinterpret_cast<struct sockaddr*>(&server_address), sizeof(server_address)) < 0) {
        perror("Error assigning address");
        exit(1);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        exit(1);
    }

    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket > 0) {
            std::thread client_thread(handle_client, client_socket);
            client_thread.detach();  
        }
    }

    close(server_socket);
    return 0;
}