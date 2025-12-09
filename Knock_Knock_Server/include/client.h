#ifndef CLIENT_H
#define CLIENT_H

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
#endif

#include <ctype.h>

// Platform-specific defines
#ifdef _WIN32
    typedef SOCKET socket_t;
    #define close_socket(s) closesocket(s)
    #define sleep_ms(ms) Sleep(ms)
#else
    typedef int socket_t;
    #define close_socket(s) close(s)
    #define sleep_ms(ms) usleep((ms) * 1000)
#endif

// Common constants
#define PORT 8080
#define BUFFER_SIZE 1024

// Function declarations
void init_network();
void cleanup_network();

#endif