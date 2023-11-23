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

char* format(int parametr, char* parametrName, int id_węzła, int id_pluginu, int time) {
    int dlugosc_wyniku = snprintf(NULL, 0, "{Headers:{%d,%d,Opis_DATA{%s: int},LIST[{%d,%d}]}}}", id_węzła, id_pluginu, parametrName, time, parametr);

    char* wynik = malloc((dlugosc_wyniku + 1) * sizeof(char));  // +1 na znak końca ciągu '\0'

    if (wynik == NULL) {
        perror("Błąd alokacji pamięci");
        exit(EXIT_FAILURE);
    }

    snprintf(wynik, dlugosc_wyniku + 1, "{Headers:{%d,%d,Opis_DATA{%s: int},LIST[{%d,%d}]}}}", id_węzła, id_pluginu, parametrName, time, parametr);

    return wynik;
}

char* format_with_guid(int parametr, char* parametrName, int id_węzła, int id_pluginu, int time, int guid) {
    int dlugosc_wyniku = snprintf(NULL, 0, "{Headers:{node_id: %d,\nplugin_id: %d,\nguid: %d,\ndata: [{time: %d,\n%s: %d}]}}}", id_węzła, id_pluginu, guid, time, parametrName, parametr);

    char* wynik = malloc((dlugosc_wyniku + 1) * sizeof(char));  // +1 na znak końca ciągu '\0'

    if (wynik == NULL) {
        perror("Błąd alokacji pamięci");
        exit(EXIT_FAILURE);
    }

    snprintf(wynik, dlugosc_wyniku + 1, "{Headers:{node_id: %d,\nplugin_id: %d,\nguid: %d,\ndata: [{time: %d,\n%s: %d}]}}}", id_węzła, id_pluginu, guid, time, parametrName, parametr);

    return wynik;
}


char* get_data(int id_node) {
    char buffer[256];
    system("./memoryUsage.sh > memory_usage.tmp");
    FILE *file = fopen("memory_usage.tmp", "r"); 
    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Konwersja odczytanej wartości na liczbę
        int memory_usage = atoi(buffer);

        // Odczytanie czasu wykonania z potoku
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            int execution_time = atoi(buffer);

            // Wywołanie funkcji format
            char* wynik = format(memory_usage, "Ram", id_node, 1, execution_time);
            fclose(file);
            system("rm memory_usage.tmp");
            return wynik;

           

        
        } else {
            printf("Błąd odczytu czasu wykonania z potoku\n");
        }
    } else {
        printf("Błąd odczytu z potoku\n");
    }

    char *error_msg = "error";
    return error_msg;
}

char* get_data_ram(int id_node) {
    char buffer[256];
    system("./memoryUsage.sh > memory_usage.tmp");
    FILE *file = fopen("memory_usage.tmp", "r"); 
    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Konwersja odczytanej wartości na liczbę
        int memory_usage = atoi(buffer);

        // Odczytanie czasu wykonania z potoku
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            int execution_time = atoi(buffer);

            // Wywołanie funkcji format
            char* wynik = format_with_guid(memory_usage, "Ram", id_node, 1, execution_time, ram_guid);
            fclose(file);
            system("rm memory_usage.tmp");
            return wynik;

           

        
        } else {
            printf("Błąd odczytu czasu wykonania z potoku\n");
        }
    } else {
        printf("Błąd odczytu z potoku\n");
    }

    char *error_msg = "error";
    return error_msg;
}

char* get_data_cpu(int id_node) {
    char buffer[256];
    system("./cpu.sh > cpu.tmp");
    FILE *file = fopen("cpu.tmp", "r"); 
    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Konwersja odczytanej wartości na liczbę
        int memory_usage = atoi(buffer);

        // Odczytanie czasu wykonania z potoku
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            int cpu_usage = atoi(buffer);

            // Wywołanie funkcji format
            char* wynik = format_with_guid(memory_usage, "CPU", id_node, 1, cpu_usage, cpu_guid);
            fclose(file);
            system("rm cpu.tmp");
            return wynik;

           

        
        } else {
            printf("Błąd odczytu czasu wykonania z potoku\n");
        }
    } else {
        printf("Błąd odczytu z potoku\n");
    }

    char *error_msg = "error";
    return error_msg;
}

void send_formated_data(int server_socket, char *fdata) {
  send(server_socket, fdata, strlen(fdata), 0);
}
void send_data(int server_socket) {
    char data[] = "Client data to send";
    send(server_socket, data, strlen(data), 0);
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
              send_data(client_socket);
              memset(buffer, 0, sizeof(buffer)); // Clear the buffer
            } else {
              char *data;
              if(guid == ram_guid)
                data = get_data_ram(id_node);
              else 
                  data = get_data_cpu(id_node);
              send_formated_data(client_socket, data);
              free(data);
            }
        }

        // Maybe add a delay to avoid constant checking and reduce CPU usage??
        // sleep(1);
    }

    close(client_socket);
    return 0;
}
