#include "include/server.h"

// Global variables definition
joke_t jokes[MAX_JOKES];
int joke_count = 0;
int active_clients = 0;
mutex_t client_mutex;

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

void load_jokes() {
    // First try to load from SQLite database
    if (load_jokes_from_db() == 0) {
        printf("Successfully loaded %d jokes from database\n", joke_count);
        return;
    }
    
    // If database fails, try to load from file
    printf("Database not available, loading from jokes.txt...\n");
    if (load_jokes_from_file() == 0) {
        printf("Successfully loaded %d jokes from file\n", joke_count);
        return;
    }
    
    // If both fail, load default jokes
    printf("Both database and file failed, loading default jokes...\n");
}

int load_jokes_from_db() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sql = "SELECT setup, punchline FROM jokes;";
    int rc;
    
    // Try to open the database
    rc = sqlite3_open("jokes.db", &db);
    if (rc != SQLITE_OK) {
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    
    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    
    // Execute the statement and process results
    joke_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && joke_count < MAX_JOKES) {
        const char *setup = (const char*)sqlite3_column_text(stmt, 0);
        const char *punchline = (const char*)sqlite3_column_text(stmt, 1);
        
        if (setup && punchline) {
            strncpy(jokes[joke_count].setup, setup, sizeof(jokes[joke_count].setup) - 1);
            strncpy(jokes[joke_count].punchline, punchline, sizeof(jokes[joke_count].punchline) - 1);
            jokes[joke_count].setup[sizeof(jokes[joke_count].setup) - 1] = '\0';
            jokes[joke_count].punchline[sizeof(jokes[joke_count].punchline) - 1] = '\0';
            joke_count++;
        }
    }
    
    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (joke_count == 0) {
        printf("No jokes found in database\n");
        return -1;
    }
    
    return 0; // Success
}

int load_jokes_from_file() {
    FILE *file = fopen("jokes.txt", "r");
    if (!file) {
        printf("Jokes file does not exist!\n");
        return -1;
    }

    joke_count = 0;
    char line[200];
    while (fgets(line, sizeof(line), file) && joke_count < MAX_JOKES) {
        char *setup = strtok(line, "|");
        char *punchline = strtok(NULL, "|");
        
        if (setup && punchline) {
            setup[strcspn(setup, "\r\n")] = 0;
            punchline[strcspn(punchline, "\r\n")] = 0;
            
            strcpy(jokes[joke_count].setup, setup);
            strcpy(jokes[joke_count].punchline, punchline);
            joke_count++;
        }
    }
    fclose(file);
    
    if (joke_count == 0) {
        printf("No jokes found in file\n");
        return -1;
    }
    
    return 0; // Success
}

joke_t get_unheard_joke(client_data_t *client_data) {
    printf("DEBUG: get_unheard_joke called. Heard count: %d/%d\n", 
           client_data->jokes_heard_count, joke_count);
    
    if (client_data->jokes_heard_count >= joke_count) {
        joke_t no_joke;
        strcpy(no_joke.setup, "NO_MORE_JOKES");
        return no_joke;
    }

    int available_jokes[joke_count];
    int available_count = 0;
    
    for (int i = 0; i < joke_count; i++) {
        if (!client_data->heard_jokes[i]) {
            available_jokes[available_count++] = i;
        }
    }

    printf("DEBUG: Available jokes: %d\n", available_count);
    
    if (available_count == 0) {
        joke_t no_joke;
        strcpy(no_joke.setup, "NO_MORE_JOKES");
        return no_joke;
    }
    
    int random_index = available_jokes[rand() % available_count];
    client_data->heard_jokes[random_index] = 1;
    client_data->jokes_heard_count++;

    printf("DEBUG: Selected joke %d: %s\n", random_index, jokes[random_index].setup);
    
    return jokes[random_index];
}

int strcasecmp_custom(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower(*s1) != tolower(*s2)) {
            return tolower(*s1) - tolower(*s2);
        }
        s1++;
        s2++;
    }
    return tolower(*s1) - tolower(*s2);
}

void handle_joke(client_data_t *client_data) {
    char buffer[BUFFER_SIZE];
    socket_t client_socket = client_data->socket;
    
    while (1) {
        joke_t current_joke = get_unheard_joke(client_data);

        if (strcmp(current_joke.setup, "NO_MORE_JOKES") == 0) {
            char *no_more_msg = "Server: I have no more jokes to tell.\n";
            send(client_socket, no_more_msg, strlen(no_more_msg), 0);
            printf("Client has heard all jokes. Disconnecting.\n");
            break;
        }
        
        // Send "Knock knock!"
        memset(buffer, 0, BUFFER_SIZE);
        strcpy(buffer, "Server: Knock knock!\n");
        send(client_socket, buffer, strlen(buffer), 0);
        
        // Wait for "Who's there?"
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) break;
        
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        if (strcasecmp_custom(buffer, "Who's there?") != 0) {
            char error_msg[150];
            snprintf(error_msg, sizeof(error_msg), 
                    "Server: You are supposed to say, \"Who's there?\". Let's try again.\n");
            send(client_socket, error_msg, strlen(error_msg), 0);
            continue;
        }
        
        // Send the setup
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, sizeof(buffer), "Server: %s.\n", current_joke.setup);
        send(client_socket, buffer, strlen(buffer), 0);
        
        // Wait for "[Setup] who?"
        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) break;
        
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        char expected[100];
        snprintf(expected, sizeof(expected), "%s who?", current_joke.setup);
        
        if (strcasecmp_custom(buffer, expected) != 0) {
            char error_msg[150];
            snprintf(error_msg, sizeof(error_msg), 
                    "Server: You are supposed to say, \"%s\". Let's try again.\n", expected);
            send(client_socket, error_msg, strlen(error_msg), 0);
            continue;
        }
        
        // Send the punchline
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, sizeof(buffer), "Server: %s\n", current_joke.punchline);
        send(client_socket, buffer, strlen(buffer), 0);
        
        if (client_data->jokes_heard_count >= joke_count) {
            char *no_more_msg = "Server: I have no more jokes to tell.\n";
            send(client_socket, no_more_msg, strlen(no_more_msg), 0);
            printf("Client has heard all jokes. Disconnecting.\n");
            break;
        }
        
        // Ask if they want another joke
        memset(buffer, 0, BUFFER_SIZE);
        strcpy(buffer, "Server: Would you like to listen to another? (Y/N)\n");
        send(client_socket, buffer, strlen(buffer), 0);
        
        // Set receive timeout
#ifdef _WIN32
        DWORD timeout = 30000; // 30 seconds in milliseconds
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif

        // Get response
        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);

        // Restore default timeout
#ifdef _WIN32
        timeout = 0;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif

        if (bytes_read <= 0) {
#ifdef _WIN32
            if (WSAGetLastError() == WSAETIMEDOUT) {
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
                printf("DEBUG: Receive timeout, client took too long to respond\n");
            }
            break;
        }
        
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        if (tolower(buffer[0]) == 'y') {
            continue;
        } else if (tolower(buffer[0]) == 'n') {
            break;
        }
    }
}

#ifdef _WIN32
DWORD WINAPI handle_client(LPVOID arg) {
#else
void *handle_client(void *arg) {
#endif
    client_data_t *client_data = (client_data_t *)arg;
    socket_t client_socket = client_data->socket;
    char client_ip[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &(client_data->address.sin_addr), client_ip, INET_ADDRSTRLEN);
    
    client_data->heard_jokes = calloc(joke_count, sizeof(int));
    client_data->jokes_heard_count = 0;
    
    mutex_lock(&client_mutex);
    active_clients++;
    printf("Client connected from %s:%d. Active clients: %d\n", 
           client_ip, ntohs(client_data->address.sin_port), active_clients);
    mutex_unlock(&client_mutex);
    
    printf("DEBUG: Start handling joke\n");
    handle_joke(client_data);
    printf("DEBUG: End handling joke\n");
    
    free(client_data->heard_jokes);
    close_socket(client_socket);
    
    mutex_lock(&client_mutex);
    active_clients--;
    printf("Client %s disconnected. Active clients: %d\n", client_ip, active_clients);
    mutex_unlock(&client_mutex);
    
    free(client_data);
    
#ifdef _WIN32
    return 0;
#else
    pthread_exit(NULL);
#endif
}

int main() {
    socket_t server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Initialize network
    init_network();
    
    // Initialize mutex
    mutex_init(&client_mutex);
    
    // Load jokes and seed random
    load_jokes();
    srand((unsigned int)time(NULL));
    
    printf("Loaded %d jokes\n", joke_count);
    if (joke_count < 15) {
        printf("Warning: Only %d jokes loaded. Need at least 15.\n", joke_count);
    }
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (server_fd == INVALID_SOCKET) {
#else
    if (server_fd < 0) {
#endif
        perror("socket failed");
        cleanup_network();
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))) {
        perror("setsockopt");
        close_socket(server_fd);
        cleanup_network();
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close_socket(server_fd);
        cleanup_network();
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        close_socket(server_fd);
        cleanup_network();
        exit(EXIT_FAILURE);
    }
    
    printf("Knock Knock server listening on port %d\n", PORT);
    printf("Server will terminate when no clients connected for 30 seconds\n");

    time_t last_activity_time = time(NULL);
    int timeout_seconds = 30;

    while (1) {
        // Set timeout for accept
#ifdef _WIN32
        DWORD timeout = 1000; // 1 second in milliseconds
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
        
        client_data_t *client_data = malloc(sizeof(client_data_t));
        if (!client_data) {
            perror("malloc failed");
            sleep_ms(1000);
            continue;
        }
        
        client_data->socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
#ifdef _WIN32
        if (client_data->socket == INVALID_SOCKET) {
#else
        if (client_data->socket < 0) {
#endif
            free(client_data);
            
            mutex_lock(&client_mutex);
            int current_active_clients = active_clients;
            mutex_unlock(&client_mutex);
            
            if (current_active_clients == 0) {
                time_t current_time = time(NULL);
                if (difftime(current_time, last_activity_time) >= timeout_seconds) {
                    printf("No clients connected for %d seconds. Server terminating.\n", timeout_seconds);
                    break;
                }
            } else {
                last_activity_time = time(NULL);
            }
            
            continue;
        }

        mutex_lock(&client_mutex);
        if (active_clients >= 5) {
            mutex_unlock(&client_mutex);
            
            // Get client info for logging
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
            
            printf("🚫 Max clients reached (%d). Rejecting connection from %s:%d\n", 
                5, client_ip, ntohs(address.sin_port));
            
            // Send rejection message IMMEDIATELY
            char reject_msg[] = "Server: Maximum client limit (5) reached. Please try again later.\n";
            send(client_data->socket, reject_msg, strlen(reject_msg), 0);
            
            // Close the connection immediately
            close_socket(client_data->socket);
            free(client_data);
            continue;
        }
        mutex_unlock(&client_mutex);
        
        last_activity_time = time(NULL);
        
        // Remove timeout
#ifdef _WIN32
        timeout = 0;
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
        
        client_data->address = address;
        
        thread_t thread_id;
        if (thread_create(&thread_id, NULL, handle_client, (void*)client_data) != 0) {
            perror("thread_create failed");
            close_socket(client_data->socket);
            free(client_data);
            continue;
        }
        
        thread_detach(thread_id);
        printf("New client connected. Active clients: %d\n", active_clients + 1);
    }

    close_socket(server_fd);
    mutex_destroy(&client_mutex);
    cleanup_network();
    printf("Server terminated successfully.\n");
    return 0;
}