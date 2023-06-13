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
    int VIEWER_PORT = atoi(argv[1]);
    char* VIEWER_IP = argv[2];
    printf("VIEWER CLIENT IS STARTING ON PORT %d AND IP %s\n", VIEWER_PORT, VIEWER_IP);

    // creating viewer socket
    int viewer_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (viewer_socket == -1) {
        perror("Error: socket() function has failed.\n");
        exit(1);
    }

    // saving viewer address
    struct sockaddr_in viewer_address;
    viewer_address.sin_family = AF_INET;
    viewer_address.sin_port = htons(VIEWER_PORT);
    viewer_address.sin_addr.s_addr = inet_addr(VIEWER_IP);

    // connecting
    if (connect(viewer_socket, (struct sockaddr *) &viewer_address, sizeof(viewer_address)) == -1){
        perror("Error: connect() function has failed.\n");
        exit(1);
    }
    printf("VIEWER SUCCESSFULLY CONNECTED.\n");

    write(viewer_socket, "VIEWER", sizeof("VIEWER"));

    char buffer[1024];

    int keep_viewer_alive = 1;
    while (keep_viewer_alive) {
        read(viewer_socket, buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }

    return 0;
}