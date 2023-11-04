#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Server socket error");
    exit(1);
  }

  // konfiguracja struktury adresu serwera
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(12345);
  server_address.sin_addr.s_addr = INADDR_ANY;

  // przypisanie i nasłuchiwanie na gnieździe
  if (bind(server_socket, (struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    perror("Error assigning address");
    exit(1);
  }
  // przykladowo 5 polaczen w kolejce
  if (listen(server_socket, 5) < 0) {
    perror("Error listening on socket");
    exit(1);
  }

  // oczekiwanie na połączenie od klienta
  printf("Waiting for connection...\n");
  int client_socket = accept(server_socket, NULL, NULL);

  // obsługa komunikacji z klientem
  char buffer[256];
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
      break;
    }
    printf("Recieved: %s", buffer);
  }

  close(client_socket);

  close(server_socket);

  return 0;
}
