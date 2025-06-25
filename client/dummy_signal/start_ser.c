#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SIGNAL_MESSAGE "Server started signal"

void send_server_signal(const char *dst_ip, unsigned short dst_port) {
    int sock;
    struct sockaddr_in sa;
    unsigned char buf[1024];
    int offset = 0;

    // session_id (16 bytes) - 固定値例
    unsigned char session_id[16] = {
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
        0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10
    };

    // chunk (8 bytes) - 固定例 (0xFF * 4 + 0x00 * 4)
    unsigned char chunk[8] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00
    };

    // format (2 bytes) - サーバー起動シグナルとして0xFF 0xFF
    unsigned char format[2] = {0xFF, 0xFF};

    // data_vec (14 bytes) - 0で埋める
    unsigned char data_vec[14] = {0};

    // データペイロード
    const char *data_payload = SIGNAL_MESSAGE;
    size_t data_len = strlen(data_payload);

    // UDPペイロード組み立て
    memcpy(buf + offset, session_id, sizeof(session_id));
    offset += sizeof(session_id);

    memcpy(buf + offset, chunk, sizeof(chunk));
    offset += sizeof(chunk);

    memcpy(buf + offset, format, sizeof(format));
    offset += sizeof(format);

    memcpy(buf + offset, data_vec, sizeof(data_vec));
    offset += sizeof(data_vec);

    memcpy(buf + offset, data_payload, data_len);
    offset += data_len;

    // ソケット作成
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(dst_port);
    if (inet_pton(AF_INET, dst_ip, &sa.sin_addr) <= 0) {
        perror("inet_pton dst_ip");
        exit(EXIT_FAILURE);
    }

    // パケット送信
    ssize_t sent = sendto(sock, buf, offset, 0, (struct sockaddr *)&sa, sizeof(sa));
    if (sent < 0) {
        perror("sendto");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Sent server start signal to %s:%d, bytes=%d\n", dst_ip, dst_port, (int)sent);

    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <dst_ip> <dst_port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *dst_ip = argv[1];
    unsigned short dst_port = (unsigned short)atoi(argv[2]);

    send_server_signal(dst_ip, dst_port);

    return 0;
}

