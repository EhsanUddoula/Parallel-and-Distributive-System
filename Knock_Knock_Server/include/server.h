#ifndef SERVER_H
#define SERVER_H

// Platform detection
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <pthread.h>
    #include <sys/time.h>
    #include <errno.h>
#endif

#include <ctype.h>
#include <time.h>
#include <sqlite3.h> 

// Platform-specific defines
#ifdef _WIN32
    typedef SOCKET socket_t;
    #define close_socket(s) closesocket(s)
    #define sleep_ms(ms) Sleep(ms)
    typedef HANDLE thread_t;
    typedef CRITICAL_SECTION mutex_t;
    #define mutex_init(m) InitializeCriticalSection(m)
    #define mutex_lock(m) EnterCriticalSection(m)
    #define mutex_unlock(m) LeaveCriticalSection(m)
    #define mutex_destroy(m) DeleteCriticalSection(m)
    #define thread_create(thread, attr, func, arg) \
        ((*(thread) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL)) != NULL ? 0 : -1)
    #define thread_detach(thread) CloseHandle(thread)
#else
    typedef int socket_t;
    #define close_socket(s) close(s)
    #define sleep_ms(ms) usleep((ms) * 1000)
    typedef pthread_t thread_t;
    typedef pthread_mutex_t mutex_t;
    #define mutex_init(m) pthread_mutex_init(m, NULL)
    #define mutex_lock(m) pthread_mutex_lock(m)
    #define mutex_unlock(m) pthread_mutex_unlock(m)
    #define mutex_destroy(m) pthread_mutex_destroy(m)
    #define thread_create(thread, attr, func, arg) pthread_create(thread, attr, func, arg)
    #define thread_detach(thread) pthread_detach(thread)
#endif

// Common constants
#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define MAX_JOKES 100

typedef struct {
    socket_t socket;
    struct sockaddr_in address;
    int *heard_jokes;
    int jokes_heard_count;
} client_data_t;

typedef struct {
    char setup[100];
    char punchline[100];
} joke_t;

// Global variables
extern joke_t jokes[MAX_JOKES];
extern int joke_count;
extern int active_clients;
extern mutex_t client_mutex;

// Function declarations
void load_jokes();
joke_t get_unheard_joke(client_data_t *client_data);
int strcasecmp_custom(const char *s1, const char *s2);
void handle_joke(client_data_t *client_data);
int load_jokes_from_db();
int load_jokes_from_file();
#ifdef _WIN32
    DWORD WINAPI handle_client(LPVOID arg);
#else
    void *handle_client(void *arg);
#endif
void init_network();
void cleanup_network();

#endif