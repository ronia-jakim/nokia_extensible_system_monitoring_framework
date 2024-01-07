#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 256
#define CONST_DATA 0

#define MAX_PLUGIN 2
#define CONFIG_FILE "./config.txt"
const int ram_guid = 0;
const int cpu_guid = 1;

// struct used for opening child processes and communicating with them :3
struct popen2 {
    pid_t child_pid;
    int   from_child, to_child;
};


// function that opens a new process that executes plugin and sets up pipes
int popen2(char * plugin_path, char * plugin_args, struct popen2 * childinfo);

// sends signal to plugin to get information and receives it -> returns the result
char * get_data_plugin(int guid, int node, struct popen2 * plugin);

// sends data gathered by one of the plugins to server (there is a loop that goes through all the plugins)
void send_data(int server_socket, char * data);


int main(int argc, char* argv[]){
    int id_node = 0;
    int guid = ram_guid;
    if (argc >= 2) {
      id_node = atoi(argv[1]);  
    }
    if (argc >= 3) { // not used right now
        guid = atoi(argv[2]);
    }


    // ============================================================================= //
    //                                                                               //
    //                                m                                  "           //
    // m     m  mmm   mmmmm  m   m  mm#mm  m   m m     m  mmm   m mm   mmm     mmm   //
    // "m m m" #"  "     m"  "m m"    #    "m m" "m m m" "   #  #"  #    #    #"  #  //
    //  #m#m#  #       m"     #m#     #     #m#   #m#m#  m"""#  #   #    #    #""""  //
    //   # #   "#mm"  #mmmm   "#      "mm   "#     # #   "mm"#  #   #  mm#mm  "#mm"  //
    //                        m"            m"                                       //
    //                       ""            ""                                        //
    //                                                                               //
    //                                m""    "                                       //
    //          mmm    mmm   m mm   mm#mm  mmm     mmmm  m   m                       //
    //         #"  "  #" "#  #"  #    #      #    #" "#  #   #                       //
    //         #      #   #  #   #    #      #    #   #  #   #                       //
    //         "#mm"  "#m#"  #   #    #    mm#mm  "#m"#  "mm"#                       //
    //                                             m  #                              //
    //                                              ""                               //
    // ============================================================================= //

    char * plugin_list[MAX_PLUGIN];

    char * args_list[MAX_PLUGIN];

    for (int i = 0; i < MAX_PLUGIN; i++) {
      plugin_list[i] = (char *)malloc(sizeof(char) * 100);
      args_list[i] = (char *)malloc(sizeof(char) * 100);
    }

    int plugin_list_length = 0;

    FILE * conf_file = fopen(CONFIG_FILE, "r");

    if (conf_file == NULL) exit(EXIT_FAILURE);
    
    ssize_t line_len;
    char * line;
    size_t l = 0;

    while ((line_len = getline(&line, &l, conf_file)) != -1) {
      int sep_nr = 0;
      int j = 0;

      char id [4];
      for (int i = 0; i < 4; i++) id[i] = ' ';
      char args[100];
      for (int i = 0; i < 100; i++) {
        args[i] = ' ';
      }

      for (int i = 0; i < line_len; i++) {
        if (line[i] == ';') {
          sep_nr++;
          j = 0;
        }

        else if (sep_nr == 0) {
          //id[j] = line[i];
          j++;
        }

        else if (sep_nr == 1) {
          plugin_list[plugin_list_length][j] = line[i];
          j++;
        }

        else {
          args_list[plugin_list_length][j] = line[i];
          j++;
        }
      }
      
      plugin_list_length++;
    }


    // ============================================================================= //
    //                                                                               //
    //  #                      "                                                     //
    //  #   m   mmm   m mm   mmm     mmm    mmm         m     m  mmm   mmmmm  m   m  //
    //  # m"   #" "#  #"  #    #    #"  #  #"  "        "m m m" #"  "     m"  "m m"  //
    //  #"#    #   #  #   #    #    #""""  #             #m#m#  #       m"     #m#   //
    //  #  "m  "#m#"  #   #  mm#mm  "#mm"  "#mm"          # #   "#mm"  #mmmm   "#    //
    //                                                                         m"    //
    //                                                                        ""     //
    //                                                                               //
    //    m                                  "                                       //
    //  mm#mm  m   m m     m  mmm   m mm   mmm     mmm                               //
    //    #    "m m" "m m m" "   #  #"  #    #    "   #                              //
    //    #     #m#   #m#m#  m"""#  #   #    #    m"""#                              //
    //    "mm   "#     # #   "mm"#  #   #  mm#mm  "mm"#                              //
    //          m"                                                                   //
    //         ""                                                                    //
    // ============================================================================= //
   
    struct popen2 children[5];

    for (int i = 0; i < plugin_list_length; i++) {
      popen2(plugin_list[i], args_list[i], &children[i]);
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

              // go through every child process and get data from them
              for (int i = 0; i < plugin_list_length; i++) {
                char * data;
                data = get_data_plugin(guid, id_node, &children[i]);

                send_data(client_socket, data);
                free(data);
                sleep(1);
              }
            }
        }

        // Maybe add a delay to avoid constant checking and reduce CPU usage??
        // sleep(1);
    }

    close(client_socket);
    return 0;
}


int popen2(char * plugin_path, char * plugin_args, struct popen2 * childinfo) {
    pid_t p;
    int pipe_stdin[2], pipe_stdout[2];

    if(pipe(pipe_stdin)) return -1;
    if(pipe(pipe_stdout)) return -1;

    printf("pipe_stdin[0] = %d, pipe_stdin[1] = %d\n", pipe_stdin[0], pipe_stdin[1]);
    printf("pipe_stdout[0] = %d, pipe_stdout[1] = %d\n", pipe_stdout[0], pipe_stdout[1]);

    p = fork();

    char * cmdline = plugin_path;
    strcat(cmdline, plugin_args);

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


char* get_data_plugin(int guid, int node, struct popen2 *plugin) {
    size_t result_size = 0;
    char* result = NULL;

    result = (char*)malloc(sizeof(char) * 1025);


    // I make the child read 
    write(plugin->to_child, "GET_DATA\n", 10);

    // I read what my child has written
    // pilnować, czy dostaliśmy już wszystko
    result_size = read(plugin->from_child, result, 1024);

    sleep(1);

    if (result_size > 0) {
        return result;
    } 
    else {
        free(result);
        return NULL;
    }
}


void send_data(int server_socket, char *data) {
	printf("Data to be sent: %s\n", data);

    send(server_socket, data, strlen(data), 0);
}

