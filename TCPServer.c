#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 20
#define BUFFER_SIZE 1024

typedef struct {
    int costumer_cnt;
    int barber_sleep;
    int done_haircuts;
    int total_done;
    int max_clients;
    pthread_mutex_t mutex;
} Queue;

typedef struct {
    int clients_socket;
    int barber_socket;
    int viewer_socket;
    Queue *queue;
} ClientData;

void* handle_barber_client(void *arg) {
    ClientData *client_data = (ClientData*)arg;
    int barber_socket = client_data->barber_socket;
    Queue *queue = client_data->queue;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        if (queue->costumer_cnt == 0 && queue->barber_sleep == 0) {
            write(barber_socket, "Queue empty", sizeof("Queue empty"));
            pthread_mutex_lock(&(queue->mutex));
            queue->barber_sleep = 1;
            pthread_mutex_unlock(&(queue->mutex));
            continue;
        } else if (queue->costumer_cnt == 0 && queue->barber_sleep == 1) {
            continue;
        }

        write(barber_socket, "New costumer", sizeof("New costumer"));
        pthread_mutex_lock(&(queue->mutex));
        queue->barber_sleep = 0;
        pthread_mutex_unlock(&(queue->mutex));
        read(barber_socket, buffer, BUFFER_SIZE - 1);
        if (strcmp(buffer, "Haircut done") == 0) {
            queue->total_done++;
            pthread_mutex_lock(&(queue->mutex));
            queue->costumer_cnt--;
            queue->done_haircuts++;
            printf("Haircut done.\n");
            pthread_mutex_unlock(&(queue->mutex));
        }
        if (queue->costumer_cnt + queue->total_done == queue->max_clients) {
            break;
        }
    }

    close(barber_socket);
    free(client_data);
    pthread_exit(NULL);
}

void* handle_clients_client(void *arg) {
    ClientData *client_data = (ClientData*)arg;
    int clients_socket = client_data->clients_socket;
    Queue *queue = client_data->queue;
    char buffer[BUFFER_SIZE];
    while (read(clients_socket, buffer, BUFFER_SIZE - 1) > 0) {
        if (strcmp(buffer, "New costumer") == 0) {
            printf("New costumer came.\n");
            pthread_mutex_lock(&(queue->mutex));
            queue->costumer_cnt++;
            pthread_mutex_unlock(&(queue->mutex));
            char str[BUFFER_SIZE];
            snprintf(str, BUFFER_SIZE - 1, "%d", queue->costumer_cnt);
            char str1[BUFFER_SIZE] = "Queue length: ";
            strcat(str1, str);
            strcat(str1, ". Also ");
            char str2[BUFFER_SIZE];
            snprintf(str2, BUFFER_SIZE - 1, "%d", queue->done_haircuts);
            strcat(str1, str2);
            strcat(str1, " haircuts has ended.");
            pthread_mutex_lock(&(queue->mutex));
            queue->done_haircuts = 0;
            pthread_mutex_unlock(&(queue->mutex));
            write(clients_socket, str1, sizeof(str1));
        }
        if (queue->costumer_cnt + queue->total_done == queue->max_clients) {
            break;
        }
        memset(buffer, 0, BUFFER_SIZE);
    }

    printf("Closing clients client.\n");

    close(clients_socket);
    free(client_data);
    pthread_exit(NULL);
}

void* handle_viewer_client(void *arg) {
    ClientData *client_data = (ClientData*)arg;
    int viewer_socket = client_data->viewer_socket;
    Queue *queue = client_data->queue;
    char buffer[BUFFER_SIZE];
    int costumer_cnt = 0;
    int barber_sleep = 1;
    int done_haircuts = 0;
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        if (barber_sleep != queue->barber_sleep && barber_sleep == 1) {
            write(viewer_socket, "Barber started work.\n", sizeof("Barber started work.\n"));
            barber_sleep = 0;
        } else if (barber_sleep != queue->barber_sleep && barber_sleep == 0) {
            write(viewer_socket, "Barber go to sleep.\n", sizeof("Barber go to sleep.\n"));
            barber_sleep = 1;
        } else if (costumer_cnt < queue->costumer_cnt) {
            write(viewer_socket, "New costumer came.\n", sizeof("New costumer came.\n"));
            costumer_cnt = queue->costumer_cnt;
        } else if (costumer_cnt > queue->costumer_cnt && done_haircuts < queue->done_haircuts) {
            write(viewer_socket, "Haircut done.\n", sizeof("Haircut done.\n"));
            costumer_cnt = queue->costumer_cnt;
            done_haircuts = queue->done_haircuts;
        }
        if (queue->costumer_cnt + queue->total_done == queue->max_clients) {
            break;
        }
    }

    close(viewer_socket);
    free(client_data);
    pthread_exit(NULL);
}


int main(int argc, char **argv) {
    int SERVER_PORT;
    char* SERVER_IP;

    if (argc == 1) {
        SERVER_PORT = 8000;
        SERVER_IP = "127.0.0.1";

        printf("\033[0;33m"); //yellow
        printf("SERVER IS STARTING ON DEFAULT PORT %d AND DEFAULT IP %s", SERVER_PORT, SERVER_IP);
        printf("\033[0m\n");
    } else if (argc == 2) {
        SERVER_PORT = atoi(argv[1]);;
        SERVER_IP = "127.0.0.1";

        printf("\033[0;33m"); //yellow
        printf("SERVER IS STARTING ON PORT %d AND DEFAULT IP %s", SERVER_PORT, SERVER_IP);
        printf("\033[0m\n");
    } else if (argc == 3) {
        SERVER_PORT = atoi(argv[1]);
        SERVER_IP = argv[2];

        printf("\033[0;33m"); //yellow
        printf("SERVER IS STARTING ON PORT %d AND IP %s", SERVER_PORT, SERVER_IP);
        printf("\033[0m\n");
    } else {
        perror("Error: wrong server start parameters.\n");
        exit(1);
    }

    int max_clients;
    printf("WRITE MAX CLIENTS (1-30): ");
    scanf("%d", &max_clients);

    Queue queue = { 0, 1, 0, 0, max_clients,PTHREAD_MUTEX_INITIALIZER};
    pthread_t threads[MAX_CLIENTS];

    // creating server`s socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error: socket() function has failed.\n");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Failed to set socket options");
        exit(1);
    }

    // saving server`s address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

    // bind the socket to our specified IP and port
    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Error: bind() function has failed.\n");
        exit(1);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Error: listen() function has failed.\n");
        exit(1);
    }

    printf("\033[0;33m"); //yellow
    printf("Server is listening...\n");
    printf("\033[0m\n");

    struct sockaddr_in client_address;
    int client_address_len = sizeof(client_address);
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int client_socket;
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_address_len)) < 0) {
            perror("Error: accept() function has failed.\n");
        }

        ClientData *client_data = (ClientData*)malloc(sizeof(ClientData));
        client_data->queue = &queue;
        read(client_socket, buffer, BUFFER_SIZE);
        if (strcmp(buffer, "BARBER") == 0) {
            printf("\033[0;32m"); //green
            printf("Barber connected.");
            printf("\033[0m\n");

            client_data->barber_socket = client_socket;
            if (pthread_create(&threads[i], NULL, handle_barber_client, (void*)client_data) != 0) {
                perror("Error: failed to create barber thread.\n");
                exit(1);
            }
        } else if (strcmp(buffer, "CLIENTS") == 0) {
            printf("\033[0;32m"); //green
            printf("Clients connected.");
            printf("\033[0m\n");

            client_data->clients_socket = client_socket;
            if (pthread_create(&threads[i], NULL, handle_clients_client, (void*)client_data) != 0) {
                perror("Error: failed to create clients thread.\n");
                exit(1);
            }
        } else if (strcmp(buffer, "VIEWER") == 0) {
            printf("\033[0;32m"); //green
            printf("Viewer connected.");
            printf("\033[0m\n");

            client_data->viewer_socket = client_socket;
            if (pthread_create(&threads[i], NULL, handle_viewer_client, (void*)client_data) != 0) {
                perror("Error: failed to create clients thread.\n");
                exit(1);
            }
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
