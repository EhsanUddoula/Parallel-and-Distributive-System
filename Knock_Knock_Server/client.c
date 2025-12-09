#include "include/client.h"

#ifdef _WIN32
// Windows network initialization
void init_network() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

void cleanup_network() {
    WSACleanup();
}
#else
// Linux network initialization (dummy functions)
void init_network() {}
void cleanup_network() {}
#endif

int main() {
    socket_t sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    
    // Initialize network
    init_network();
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (sock == INVALID_SOCKET) {
        printf("Socket creation error: %d\n", WSAGetLastError());
        cleanup_network();
        return -1;
    }
#else
    if (sock < 0) {
        printf("Socket creation error\n");
        return -1;
    }
#endif
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IP address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        close_socket(sock);
        cleanup_network();
        return -1;
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
#ifdef _WIN32
        printf("Connection Failed: %d\n", WSAGetLastError());
#else
        printf("Connection Failed\n");
#endif
        close_socket(sock);
        cleanup_network();
        return -1;
    }
    
    printf("Connected to Knock Knock server!\n");
    
    while (1) {
        // Receive message from server
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_read <= 0) {
#ifdef _WIN32
            if (bytes_read == 0) {
                printf("Server disconnected gracefully\n");
            } else {
                printf("Server disconnected with error: %d\n", WSAGetLastError());
            }
#else
            printf("Server disconnected\n");
#endif
            break;
        }
        
        buffer[bytes_read] = '\0'; // Null-terminate the received data
        printf("%s", buffer);
        
        // Check if server has no more jokes
        if (strstr(buffer, "no more jokes") != NULL) {
            printf("Server has no more jokes. Disconnecting.\n");
            break;
        }
        
        // Check if server is asking for input
        if (strstr(buffer, "Knock knock!") != NULL) {
            // Send "Who's there?"
            printf("Client: Who's there?\n");
            if (send(sock, "Who's there?", strlen("Who's there?"), 0) <= 0) {
                printf("Failed to send response\n");
                break;
            }
        }
        else if (strstr(buffer, "Server: ") != NULL && strstr(buffer, ".") != NULL && 
                 strstr(buffer, "supposed to say") == NULL) {
            char *setup_start = strstr(buffer, "Server: ");
            if (setup_start) {
                setup_start += 8; // Skip "Server: "
                char setup[50] = {0};
                
                // Extract only the word before the dot
                int i = 0;
                while (setup_start[i] != '.' && setup_start[i] != '\0' && i < 49) {
                    setup[i] = setup_start[i];
                    i++;
                }
                setup[i] = '\0';
                
                // Remove any trailing spaces
                while (i > 0 && setup[i-1] == ' ') {
                    setup[i-1] = '\0';
                    i--;
                }
                
                char response[100];
                snprintf(response, sizeof(response), "%s who?", setup);
                printf("Client: %s\n", response);
                if (send(sock, response, strlen(response), 0) <= 0) {
                    printf("Failed to send response\n");
                    break;
                }
            }
        }
        else if (strstr(buffer, "another?") != NULL) {
            // Ask user if they want another joke
            printf("Your choice (Y/N): ");
            fflush(stdout); // Ensure prompt is displayed
            
            char choice[10];
            if (fgets(choice, sizeof(choice), stdin) == NULL) {
                break; // EOF or error
            }
            choice[strcspn(choice, "\n")] = 0;
            
            printf("Client: %s\n", choice);
            if (send(sock, choice, strlen(choice), 0) <= 0) {
                printf("Failed to send response\n");
                break;
            }
            
            if (tolower(choice[0]) == 'n') {
                printf("Disconnecting...\n");
                break;
            }
        }
        else if (strstr(buffer, "supposed to say") != NULL) {
            // Server corrected us, wait for next message
            continue;
        }
        else if (strstr(buffer, "punchline") != NULL || 
                 strstr(buffer, "better!") != NULL || 
                 strstr(buffer, "freezing.") != NULL ||
                 strstr(buffer, "Exactly!") != NULL ||
                 strstr(buffer, "joke!") != NULL ||
                 strstr(buffer, "outside!") != NULL ||
                 strstr(buffer, "door!") != NULL) {
            // Server sent a punchline - just wait for next question
            continue;
        }
    }
    
    close_socket(sock);
    cleanup_network();
    return 0;
}