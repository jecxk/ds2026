#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BACKLOG 5
#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <output_file>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    const char *output_file = argv[2];

    // 1. Create socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. Allow reuse of address
    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // 3. Bind to all interfaces on given port
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(port);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // 4. Listen
    if (listen(listenfd, BACKLOG) < 0) {
        perror("listen");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    // 5. Accept one client
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &cli_len);
    if (connfd < 0) {
        perror("accept");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // 6. Receive file size (4 bytes)
    uint32_t size_net;
    ssize_t n = recv(connfd, &size_net, sizeof(size_net), MSG_WAITALL);
    if (n != sizeof(size_net)) {
        fprintf(stderr, "Failed to receive file size.\n");
        close(connfd);
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    uint32_t file_size = ntohl(size_net);
    printf("Incoming file size: %u bytes\n", file_size);

    // 7. Open output file
    FILE *fp = fopen(output_file, "wb");
    if (!fp) {
        perror("fopen");
        close(connfd);
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // 8. Receive file data
    char buffer[BUF_SIZE];
    uint32_t received = 0;

    while (received < file_size) {
        uint32_t remaining = file_size - received;
        size_t to_read = remaining > BUF_SIZE ? BUF_SIZE : remaining;

        n = recv(connfd, buffer, to_read, 0);
        if (n < 0) {
            perror("recv");
            fclose(fp);
            close(connfd);
            close(listenfd);
            exit(EXIT_FAILURE);
        }
        if (n == 0) {
            // EOF from client
            break;
        }

        size_t written = fwrite(buffer, 1, (size_t)n, fp);
        if (written < (size_t)n) {
            perror("fwrite");
            fclose(fp);
            close(connfd);
            close(listenfd);
            exit(EXIT_FAILURE);
        }

        received += (uint32_t)n;
    }

    printf("File received: %u bytes written to %s\n", received, output_file);

    fclose(fp);
    close(connfd);
    close(listenfd);

    return 0;
}
