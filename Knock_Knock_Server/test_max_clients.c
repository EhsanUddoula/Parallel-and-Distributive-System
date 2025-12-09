#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define NUM_CLIENTS 8  // Try to connect 8 clients to test the limit

typedef struct {
    int client_id;
    int connected;
} client_info_t;

client_info_t clients[NUM_CLIENTS];

void* test_client(void* arg) {
    int client_id = *(int*)arg;
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    
    printf("Client %d: Attempting to connect...\n", client_id);
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Client %d: Socket creation failed\n", client_id);
        clients[client_id].connected = 0;
        return NULL;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("Client %d: Invalid address\n", client_id);
        close(sock);
        clients[client_id].connected = 0;
        return NULL;
    }
    
    // Try to connect
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Client %d: Connection FAILED (expected for clients > 5)\n", client_id);
        clients[client_id].connected = 0;
        close(sock);
        return NULL;
    }
    
    // Read server response
    int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        if (strstr(buffer, "maximum client limit") != NULL) {
            //printf("Client %d: REJECTED - %s", client_id, buffer);
            clients[client_id].connected = 0;
        } else {
            //printf("Client %d: CONNECTED SUCCESSFULLY\n", client_id);
            clients[client_id].connected = 1;
            
            // Keep connection alive for a while to test
            sleep(10);
        }
    }
    
    close(sock);
    return NULL;
}

int main() {
    pthread_t threads[NUM_CLIENTS];
    int client_ids[NUM_CLIENTS];
    
    printf("=== Testing Server Client Limit ===\n");
    printf("Attempting to connect %d clients to server...\n", NUM_CLIENTS);
    printf("Server should accept only 5 clients maximum.\n\n");
    
    // Initialize client info
    for (int i = 0; i < NUM_CLIENTS; i++) {
        clients[i].client_id = i;
        clients[i].connected = 0;
        client_ids[i] = i;
    }
    
    // Create all client threads simultaneously
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_create(&threads[i], NULL, test_client, &client_ids[i]);
        usleep(100000); // Small delay between connection attempts (100ms)
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print summary
    
    
    
    return 0;
}