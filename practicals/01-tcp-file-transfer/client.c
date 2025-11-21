#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 4096

// Helper: send all bytes in buffer
ssize_t send_all(int sockfd, const void *buf, size_t len) {
    const char *p = (const char *)buf;
    size_t total = 0;

    while (total < len) {
        ssize_t n = send(sockfd, p + total, len - total, 0);
        if (n <= 0) {
            return n; // error
        }
        total += (size_t)n;
    }
    return (ssize_t)total;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_host> <port> <input_file>\n", argv[0]);
        return 1;
    }

    const char *server_host = argv[1];
    int port = atoi(argv[2]);
    const char *input_file = argv[3];

    // 1. Open file to send
    FILE *fp = fopen(input_file, "rb");
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // 2. Compute file size
    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    long fsize = ftell(fp);
    if (fsize < 0) {
        perror("ftell");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    rewind(fp);

    if (fsize > 0xFFFFFFFFL) {
        fprintf(stderr, "File too large (max 4 GB).\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    uint32_t file_size = (uint32_t)fsize;

    // 3. Resolve server address (gethostbyname)
    struct hostent *server = gethostbyname(server_host);
    if (server == NULL) {
        fprintf(stderr, "ERROR: no such host: %s\n", server_host);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    // 4. Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    // 5. Build server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    // 6. Connect
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    printf("Connected to %s:%d\n", server_host, port);
    printf("Sending file %s (%u bytes)\n", input_file, file_size);

    // 7. Send file size (4 bytes)
    uint32_t size_net = htonl(file_size);
    if (send_all(sockfd, &size_net, sizeof(size_net)) != sizeof(size_net)) {
        perror("send file size");
        close(sockfd);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    // 8. Send file content
    char buffer[BUF_SIZE];
    size_t nread;
    while ((nread = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
        if (send_all(sockfd, buffer, nread) < 0) {
            perror("send file data");
            close(sockfd);
            fclose(fp);
            exit(EXIT_FAILURE);
        }
    }

    printf("File sent successfully.\n");

    fclose(fp);
    close(sockfd);

    return 0;
}
