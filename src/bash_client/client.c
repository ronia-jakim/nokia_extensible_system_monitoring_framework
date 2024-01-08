#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 256
#define CONST_DATA 1
const int ram_guid = 0;
const int cpu_guid = 1;

char* get_data_plugin(int guid, int node) {
    char command[100];
    snprintf(command, sizeof(command), "python3 plugin.py %d %d > plugin.tmp", guid, node);

    // Wywołanie skryptu Pythona i zapisanie wyniku do pliku
    system(command);

    FILE* file = fopen("plugin.tmp", "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    size_t result_size = 0;
    char* result = NULL;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {

        buffer[strcspn(buffer, "\n")] = '\0';

       
        result = (char*)realloc(result, result_size + strlen(buffer) + 1);
        if (!result) {
            perror("Memory allocation error");
            fclose(file);
            return NULL;
        }

        strcpy(result + result_size, buffer);
        result_size += strlen(buffer);
    }

    fclose(file);
    system("rm plugin.tmp");

    if (result_size > 0) {
        return result;
    } else {
        free(result);
        return NULL;
    }
}

void send_data(int server_socket, char *data) {
    size_t data_length = strlen(data);
    size_t buffer_size = 1024;
    char buffer[buffer_size];

    int formatted_length = snprintf(buffer, buffer_size, "%zu\n%s@", data_length, data);

    if (formatted_length < 0 || (size_t)formatted_length >= buffer_size) {
        fprintf(stderr, "Error formatting data.\n");
        return;
    }

    size_t sent_total = 0;
    ssize_t sent_bytes;

    while (sent_total < (size_t)formatted_length) {
        sent_bytes = send(server_socket, buffer + sent_total, formatted_length - sent_total, 0);

        if (sent_bytes == -1) {
            perror("send");
            break;
        } else if (sent_bytes == 0) {
            fprintf(stderr, "Connection closed.\n");
            break;
        } else {
            sent_total += sent_bytes;
        }
    }

    if (sent_total == (size_t)formatted_length) {
        printf("Data sent successfully.\n");
    } else {
        fprintf(stderr, "Failed to send all data.\n");
    }
}


int main(int argc, char* argv[]){
    int id_node = 0;
    int guid = ram_guid;
    if(argc >= 2) {
      id_node = atoi(argv[1]);  
    }
    if(argc >= 3) { //if guid=0, client sends ram, if guid=1 client sends cpu
        guid = atoi(argv[2]);
    }


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
            if(CONST_DATA) {
              // Send data to the server
    char* data_to_send = "{ \"headers\": { \"node_id\": 7, \"plugin_id\":2, \"data_type\": \"b\" }, \"data_list\": [ { \"time\": 4, \"data\": \"abc\" }, { \"time\": 1, \"data\": \"abdc\" } ] }";
              send_data(client_socket, data_to_send);
              memset(buffer, 0, sizeof(buffer)); // Clear the buffer
            } else {
              char *data;
                  data = get_data_plugin(guid,id_node);
              send_data(client_socket, data);
              free(data);
            }
        }

        // Maybe add a delay to avoid constant checking and reduce CPU usage??
        // sleep(1);
    }

    close(client_socket);
    return 0;
}
