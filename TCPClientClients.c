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
    int CLIENTS_PORT;
    char* CLIENTS_IP;

    if (argc == 1) {
        CLIENTS_PORT = 8000;
        CLIENTS_IP = "127.0.0.1";

        printf("\033[0;33m"); //yellow
        printf("CLIENTS CLIENT IS STARTING ON DEFAULT PORT %d AND DEFAULT IP %s", CLIENTS_PORT, CLIENTS_IP);
        printf("\033[0m\n");
    } else if (argc == 2) {
        CLIENTS_PORT = atoi(argv[1]);;
        CLIENTS_IP = "127.0.0.1";

        printf("\033[0;33m"); //yellow
        printf("CLIENTS CLIENT IS STARTING ON PORT %d AND DEFAULT IP %s", CLIENTS_PORT, CLIENTS_IP);
        printf("\033[0m\n");
    } else if (argc == 3) {
        CLIENTS_PORT = atoi(argv[1]);
        CLIENTS_IP = argv[2];

        printf("\033[0;33m"); //yellow
        printf("CLIENTS CLIENT IS STARTING ON PORT %d AND IP %s", CLIENTS_PORT, CLIENTS_IP);
        printf("\033[0m\n");
    } else {
        perror("Error: wrong client CLIENTS start parameters.\n");
        exit(1);
    }

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

    printf("\033[0;33m"); //yellow
    printf("CLIENTS SUCCESSFULLY CONNECTED.");
    printf("\033[0m\n");

    write(clients_socket, "CLIENTS", sizeof("CLIENTS"));

    char buffer[1024];

    while (1) {
        sleep(5 + 10 * ((double)(abs(rand())) / RAND_MAX));
        if (write(clients_socket, "New costumer", sizeof("New costumer")) != sizeof("New costumer")) {
            perror("Error while writing.");
            exit(1);
        }
        printf("New costumer came.\n");
        read(clients_socket, buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }

    return 0;
}