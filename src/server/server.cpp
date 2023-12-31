#include <iostream>
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
#include <mutex>  
#include <queue>

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024
#define PULL_TIME 10
#define FILENAME "pomiary.txt"
#define SEQUENCE "@"

std::mutex file_mutex;
std::queue<std::pair<std::string, int>> write_queue; // A queue to hold data to be written

void write_thread_function() {
    FILE* file = fopen(FILENAME, "a");
    if (!file) {
        perror("Error opening file");
        return;
    }

    while (true) {
        if (!write_queue.empty()) {
            std::lock_guard<std::mutex> lock(file_mutex);
            auto data = write_queue.front();
            fprintf(file, "%d : %s\n", data.second, data.first.c_str());
            fflush(file);
            write_queue.pop();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Sleep for a short time if the queue is empty
        }
    }

    fclose(file); 
}

void manage(std::string buffer, int client_socket) {
    std::cout << "Received data from client(" << client_socket << "): " << buffer << std::endl;
    write_queue.push(std::make_pair(buffer, client_socket));

}

std::vector<std::string> splitString(const std::string& input, char delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = input.find(delimiter);

    while (end != std::string::npos) {
        result.push_back(input.substr(start, end - start));
        start = end + 1;
        end = input.find(delimiter, start);
    }

    result.push_back(input.substr(start));

    return result;
}

void send_info(int client_socket) {
    char message[] = "GATHER_INFO";
    send(client_socket, message, strlen(message), 0);
}

void handle_client(int client_socket) {
    std::cout << "Client connected. Socket: " << client_socket << std::endl;

    char buffer[BUFFER_SIZE];
    std::string prebuffer = "";
    bool z = false;

    send_info(client_socket);


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
            std::string Sbuffer = std::string(buffer);
            std::vector<std::string> parts = splitString(Sbuffer,'@');
            int end = 0;
            if (Sbuffer[Sbuffer.size()-1]=='@'){
                end=1;
            }
            parts[0] = prebuffer + parts[0];

            for(int i = 0; i < parts.size()-1; i++){
                std::vector<std::string> check = splitString(parts[i],'\n');
                int len1 = std::stoi(check[0]);
                int len2 = check[1].size();
                if(len1 == len2) {manage(check[1],client_socket);}
            }
            prebuffer = parts[parts.size()-1];
            
            z = true;
        }
    }

    close(client_socket);
}

int main() {
    std::thread writer_thread(write_thread_function);
    writer_thread.detach();  // Detach the writing thread

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

