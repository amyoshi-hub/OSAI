#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

char* secrity(char *filename){
	//..か/が２個以上入っていないかどうか
	//denney word を設定
}

char *parse(char* buffer){
	static char filename[256];
	char method[8], path[256];
	if(sscanf(buffer, "%s %s", method, path) == 2){
		if(path[0] == '/')	
			memmove(path, path + 1, strlen(path));
	}
	if(strlen(path) == 0){
		strcpy(filename, "index.html");
	}else{
		strncpy(filename, path, sizeof(filename) - 1);	
		//TODO
		/*
		FILE *f = fopen(filename, "r");
    		if (!f) {
        		perror("Error opening HTML file");
			strncpy(filename, "err.html", sizeof(filename) - 1);
    		}
		if(f) fclose(f);
		*/
		//TODO security
		//filename = secrity(filename);
		return filename;
	}

	return "error.html";
}

void handle_request(int client_socket) {
    char buffer[1024];
    int bytes_received;
    
    // リクエストを受け取る
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        perror("Error receiving data");
        return;
    }
    buffer[bytes_received] = '\0';

    // HTML ファイルの内容を読み込む
    char *filename = parse(buffer);
    FILE *html_file = fopen(filename, "r");
    if (!html_file) {
        perror("Error opening HTML file");
        return;
    }

    // ファイルの内容をメモリに読み込む
    fseek(html_file, 0, SEEK_END);
    long file_size = ftell(html_file);
    fseek(html_file, 0, SEEK_SET);

    char *html_content = malloc(file_size + 1);
    if (html_content == NULL) {
        perror("Memory allocation failed");
        fclose(html_file);
        return;
    }

    fread(html_content, 1, file_size, html_file); // HTML 内容を読み込む
    html_content[file_size] = '\0'; // Null-terminate the content

    // HTTPレスポンスを作成
    const char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

    // クライアントにレスポンスを送る
    send(client_socket, response_header, strlen(response_header), 0);
    send(client_socket, html_content, file_size, 0); // HTML ファイルの内容を送信

    // メモリの解放
    free(html_content);
    fclose(html_file);

    // ソケットを閉じる
    close(client_socket);
}

int main() {


    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // ソケットを作成
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // サーバーアドレス構造体を設定
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // ソケットにアドレスをバインド
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(1);
    }

    // 接続待機状態にする
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(1);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
	//connect accept
	//TODO UDPの入力に対応させる
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

	//process reqest
        handle_request(client_socket);
    }

    close(server_socket);
    return 0;
}
