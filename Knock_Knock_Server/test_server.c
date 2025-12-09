#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define NUM_CLIENTS 8  // Try to connect 8 clients to test the limit

typedef struct {
    int client_id;
    int connected;
    char status[100];
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
        sprintf(clients[client_id].status, "Socket creation failed");
        clients[client_id].connected = 0;
        return NULL;
    }
    
    // Set receive timeout to detect rejections quickly
    struct timeval tv;
    tv.tv_sec = 2;  // 2 second timeout
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        sprintf(clients[client_id].status, "Invalid address");
        close(sock);
        clients[client_id].connected = 0;
        return NULL;
    }
    
    // Try to connect
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        sprintf(clients[client_id].status, "Connection refused");
        close(sock);
        clients[client_id].connected = 0;
        return NULL;
    }
    
    // Read server response with timeout
    int bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        
        // Check if this is a rejection message
        if (strstr(buffer, "maximum client limit") != NULL || 
            strstr(buffer, "Maximum client") != NULL ||
            strstr(buffer, "limit reached") != NULL) {
            
            sprintf(clients[client_id].status, "REJECTED: %s", buffer);
            clients[client_id].connected = 0;
            printf("Client %d: REJECTED - %s", client_id, buffer);
            
        } else if (strstr(buffer, "Knock knock") != NULL) {
            // This is a successful connection - server started the joke
            sprintf(clients[client_id].status, "CONNECTED (joke started)");
            clients[client_id].connected = 1;
            printf("Client %d: CONNECTED SUCCESSFULLY\n", client_id);
            
            // Send a response and then disconnect to free up the slot
            send(sock, "Who's there?", 12, 0);
            usleep(100000); // 100ms delay
            
            // Read the setup
            recv(sock, buffer, sizeof(buffer) - 1, 0);
            
            // Send "n" to disconnect quickly
            send(sock, "n", 1, 0);
            
        } else {
            // Some other message
            sprintf(clients[client_id].status, "Connected: %s", buffer);
            clients[client_id].connected = 1;
            printf("Client %d: CONNECTED - %s", client_id, buffer);
            
            // Send "n" to disconnect quickly
            send(sock, "n", 1, 0);
        }
    } else if (bytes_read == 0) {
        sprintf(clients[client_id].status, "Server closed connection");
        clients[client_id].connected = 0;
        printf("Client %d: Server closed connection\n", client_id);
    } else {
        // Timeout or error
        sprintf(clients[client_id].status, "No response from server");
        clients[client_id].connected = 0;
        printf("Client %d: No response (timeout)\n", client_id);
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
        strcpy(clients[i].status, "Not started");
        client_ids[i] = i;
    }
    
    // Create all client threads simultaneously
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_create(&threads[i], NULL, test_client, &client_ids[i]);
        usleep(200000); // 200ms delay between connection attempts
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print summary
    printf("\n=== TEST RESULTS ===\n");
    int connected_count = 0;
    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (clients[i].connected) {
            printf("✅ Client %d: %s\n", i, clients[i].status);
            connected_count++;
        } else {
            printf("❌ Client %d: %s\n", i, clients[i].status);
        }
    }
    
    printf("\nTotal connected: %d/%d\n", connected_count, NUM_CLIENTS);
    
    if (connected_count == 5) {
        printf("✅ TEST PASSED: Server correctly limited connections to 5 clients\n");
    } else {
        printf("❌ TEST FAILED: Server allowed %d connections (should be exactly 5)\n", connected_count);
    }
    
    return 0;
}