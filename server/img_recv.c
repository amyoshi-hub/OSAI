#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define CHUNK_SIZE 1024
#define END_CHUNK_NUM 0xFFFFFFFF

// チャンク情報を格納する構造体
typedef struct {
    uint32_t chunk_num;      // チャンク番号（ネットワークバイトオーダー）
    char data[CHUNK_SIZE];   // データ本体
} Chunk;

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    Chunk chunk;
    FILE *output_file;
    uint32_t expected_chunk_num = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // ソケット作成
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // サーバーアドレス設定
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    // バインド
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for incoming data...\n");

    // 出力ファイルオープン
    output_file = fopen("received_image.jpg", "wb");
    if (!output_file) {
        perror("fopen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        ssize_t len = recvfrom(sockfd, &chunk, sizeof(chunk), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (len < 0) {
            perror("recvfrom");
            break;
        }
        if (len < sizeof(uint32_t)) {
            // チャンク番号が受信できていない
            fprintf(stderr, "Received packet too small\n");
            continue;
        }

        // チャンク番号はネットワークバイトオーダーなので変換する
        uint32_t chunk_num = ntohl(chunk.chunk_num);

	if(chunk_num == END_CHUNK_NUM){
		break;	
	}

        if (chunk_num == expected_chunk_num) {
            // チャンク番号分をファイルに書き込む（ヘッダのサイズ分引く）
            fwrite(chunk.data, 1, len - sizeof(uint32_t), output_file);
            expected_chunk_num++;
            printf("Received chunk %u\n", chunk_num);
        } else {
            printf("Out-of-order chunk %u received, expected %u\n", chunk_num, expected_chunk_num);
            // 必要に応じてバッファリングや再送要求などをここで実装可能
        }
    }

    fclose(output_file);
    close(sockfd);

    printf("File received and saved as received_image.jpg\n");

    return 0;
}
