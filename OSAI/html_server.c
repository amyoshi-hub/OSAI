#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "udp_image.h"

#define PORT 8081
#define UDP_PORT 8082

// セキュリティ関数（省略）
// char* secrity(char *filename);



char* get_content_type(const char* filename) {
    if (strstr(filename, ".html")) return "text/html";
    if (strstr(filename, ".css"))  return "text/css";
    if (strstr(filename, ".js"))   return "application/javascript";
    if (strstr(filename, ".jpg"))  return "image/jpeg";
    if (strstr(filename, ".png"))  return "image/png";
    if (strstr(filename, ".glb"))  return "model/gltf-binary";
    return "application/octet-stream";
}

char *parse(char* buffer) {
    static char filename[256];
    char method[8], path[256] = "";
    if (sscanf(buffer, "%s %s", method, path) == 2) {
        if (strcmp(path, "/") == 0) {
            strcpy(filename, "index.html");
        } else {
            memmove(path, path + 1, strlen(path));
            strncpy(filename, path, sizeof(filename) - 1);    
            FILE *f;
            if ((f = fopen(filename, "r")) == NULL) {
                perror("Error opening HTML file");
                strncpy(filename, "err.html", sizeof(filename) - 1);
            } else {
                fclose(f);
            }
            return filename;
        }
    }
    return "error.html";
}

void handle_request(int client_socket) {
    char buffer[1024];
    char header[512];
    int bytes_received;

    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        perror("Error receiving data");
        return;
    }
    buffer[bytes_received] = '\0';

    char *filename = parse(buffer);
    FILE *html_file = fopen(filename, "r");
    if (!html_file) {
        perror("Error opening HTML file");
        return;
    }

    fseek(html_file, 0, SEEK_END);
    long file_size = ftell(html_file);
    fseek(html_file, 0, SEEK_SET);

    char *html_content = malloc(file_size + 1);
    if (html_content == NULL) {
        perror("Memory allocation failed");
        fclose(html_file);
        return;
    }

    fread(html_content, 1, file_size, html_file);
    html_content[file_size] = '\0';

    char *content_type = get_content_type(filename);

    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", content_type);

    send(client_socket, header, strlen(header), 0);
    send(client_socket, html_content, file_size, 0);

    free(html_content);
    fclose(html_file);
    close(client_socket);
}

void *udp_server(void *arg) {
    int udp_socket;
    struct sockaddr_in udp_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];

    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("UDP socket creation failed");
        pthread_exit(NULL);
    }

    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(UDP_PORT);

    if (bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) == -1) {
        perror("UDP bind failed");
        close(udp_socket);
        pthread_exit(NULL);
    }

    printf("UDP server is listening on port %d...\n", UDP_PORT);

    while (1) {
        int bytes_received = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0, 
                                      (struct sockaddr *)&client_addr, &client_len);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Received UDP message: %s\n", buffer);
	    send_image_over_udp("127.0.0.1", "127.0.0.1", 8081, "test.jpg");

	    /*
            const char *response = "Hello, World!";
            sendto(udp_socket, response, strlen(response), 0, 
                   (struct sockaddr *)&client_addr, client_len);
	    */
        }
    }

    close(udp_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t udp_thread;

    if (pthread_create(&udp_thread, NULL, udp_server, NULL) != 0) {
        perror("Failed to create UDP thread");
        exit(1);
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(1);
    }

    printf("HTTP server is listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        handle_request(client_socket);
    }

    close(server_socket);
    return 0;
}

