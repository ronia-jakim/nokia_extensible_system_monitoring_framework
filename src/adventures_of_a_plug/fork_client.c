#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 256
#define CONST_DATA 0
const int ram_guid = 0;
const int cpu_guid = 1;


struct popen2 {
    pid_t child_pid;
    int   from_child, to_child;
};

int popen2(const char *cmdline, struct popen2 *childinfo) {
    pid_t p;
    int pipe_stdin[2], pipe_stdout[2];

    if(pipe(pipe_stdin)) return -1;
    if(pipe(pipe_stdout)) return -1;

    printf("pipe_stdin[0] = %d, pipe_stdin[1] = %d\n", pipe_stdin[0], pipe_stdin[1]);
    printf("pipe_stdout[0] = %d, pipe_stdout[1] = %d\n", pipe_stdout[0], pipe_stdout[1]);

    p = fork();

    if(p < 0) return p;
    
    if(p == 0) {
        // child must write to themselves
        close(pipe_stdin[1]);
        dup2(pipe_stdin[0], 0);

        // child's handwritting is terrible and thus it is incapable of reading its own scribbles
        close(pipe_stdout[0]);
        dup2(pipe_stdout[1], 1);
        
        // we hop into our terminal and do some stuff there and if everything goes north (appreciate the pun) we will end the program here
        execl("/bin/sh", "sh", "-c", cmdline, 0);
        
        // something went south and there was an error in execl
        perror("execl"); 
        exit(99);
    }
    
    // we know that we are in parent process so we just assign info about the child were it should go
    childinfo->child_pid = p;
    childinfo->to_child = pipe_stdin[1];
    childinfo->from_child = pipe_stdout[0];
    
    return 0; 
}

char* get_data_plugin(int guid, int node, struct popen2 * plugin) {
    
    // I make the child read 
    write(plugin->to_child, "GET_DATA", 8);

    size_t result_size = 0;
    char* result = NULL;
    char buffer[1024];


    result = (char*)malloc(sizeof(char) * 1025);

    printf("CHUJ\n");

    // I read what my child has written 
    int x = read(plugin->from_child, result, 1024);

    printf("DUPA\n");
    
    printf("%s\n", result);

    result[x] = '\0';

    if (result_size > 0) {
        return result;
    } else {
        free(result);
        return NULL;
    }
}

void send_data(int server_socket, char *data) {
	printf("Data to be sent: %s\n", data);

    send(server_socket, data, strlen(data), 0);
}

int main(int argc, char* argv[]){
    int id_node = 0;
    int guid = ram_guid;
    if (argc >= 2) {
      id_node = atoi(argv[1]);  
    }
    if (argc >= 3) { // not used right now
        guid = atoi(argv[2]);
    }

    char * plugin_list [] = {
      "./ram_usage.py",
      "./cpu_usage.py"
    };

    int plugin_list_length = 2;

    struct popen2 children[5];

    for (int i = 0; i < plugin_list_length; i++) {
      popen2(plugin_list[i], &children[i]);
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
              send_data(client_socket, "Client data to send");
              memset(buffer, 0, sizeof(buffer)); // Clear the buffer
            } 
            else {
              printf("dupa\n");
              for (int i = 0; i < plugin_list_length; i++) {
                printf("DUPA\n");
                char * data;
                data = get_data_plugin(guid,id_node, &children[i]);

                printf("%s\n", data);
  
                send_data(client_socket, data);
                free(data);
              }
            }
        }

        // Maybe add a delay to avoid constant checking and reduce CPU usage??
        // sleep(1);
    }

    close(client_socket);
    return 0;
}
