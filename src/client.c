#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
 
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Client socket error");
        exit(1);
    }

    // konfiguracja struktury adresu serwera
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; //ipv4
    server_address.sin_port = htons(12345); 
    server_address.sin_addr.s_addr =INADDR_ANY; //ip – dowolne na urzadzeniu

    // Połączenie z serwerem
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Server connection error");
        exit(1);
    }

    //przykladowy skrypcik
    system("echo '{\"message\": \"Hello, World!\"}' > results.json");

    
    FILE *file = fopen("results.json", "r");
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        send(client_socket, buffer, strlen(buffer), 0); // wysłanie pliku do serwera
    }
    fclose(file);

    system("rm results.json");

    
    close(client_socket);

    return 0;
}
