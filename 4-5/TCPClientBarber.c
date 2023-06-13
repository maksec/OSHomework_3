#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char **argv) {
    srand(time(NULL));
    int CLIENTS_PORT = atoi(argv[1]);
    char* CLIENTS_IP = argv[2];
    printf("BARBERS CLIENT IS STARTING ON PORT %d AND IP %s\n", CLIENTS_PORT, CLIENTS_IP);

    // creating clients socket
    int clients_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (clients_socket == -1) {
        perror("Error: socket() function has failed.\n");
        exit(1);
    }

    // saving clients address
    struct sockaddr_in clients_address;
    clients_address.sin_family = AF_INET;
    clients_address.sin_port = htons(CLIENTS_PORT);
    clients_address.sin_addr.s_addr = inet_addr(CLIENTS_IP);

    // connecting
    if (connect(clients_socket, (struct sockaddr *) &clients_address, sizeof(clients_address)) == -1){
        perror("Error: connect() function has failed.\n");
        exit(1);
    }
    printf("BARBER SUCCESSFULLY CONNECTED.\n");

    write(clients_socket, "BARBER", sizeof("BARBER"));

    char buffer[1024];

    printf("Barber is sleeping... Waiting for clients...\n");

    int keep_barber_alive = 1;
    while(keep_barber_alive) {
        read(clients_socket, buffer, sizeof(buffer));  // "New costumer" or "Queue empty"
        if (strcmp(buffer, "New costumer") == 0) {
            printf("New costumer. Start working.\n");
            sleep(5 + 10 * ((double)(abs(rand())) / RAND_MAX));
            write(clients_socket, "Haircut done", sizeof("Haircut done"));
            printf("Haircut done.\n");
        } else if (strcmp(buffer, "Queue empty") == 0) {
            printf("Queue empty. Barber sleeping...\n");
        }
    }
    return 0;
}