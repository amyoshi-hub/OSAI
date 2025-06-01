#include "udp_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <err.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>

#define CHUNK_SIZE 1024
#define END_CHUNK 0xFFFFFFFF

struct pseudo_hdr {
    struct in_addr src;
    struct in_addr dst;
    unsigned char zero;
    unsigned char proto;
    unsigned short len;
};

/* 内部関数のプロトタイプ */
static unsigned short in_cksum(unsigned short *addr, int len);
static void build_ip(char *p, struct in_addr *src, struct in_addr *dst, size_t len);
static void build_udp(char *p, struct in_addr *src, struct in_addr *dst, unsigned short dport, uint32_t chunk, char *data, long datasize);
static char *img_read(const char *filepath, long *filesize);

int send_image_over_udp(const char *src_ip, const char *dst_ip, unsigned short port, const char *imgfile) {
    int sd;
    int on = 1;
    char *buf;
    struct in_addr src, dst;
    struct sockaddr_in to;
    socklen_t tolen = sizeof(struct sockaddr_in);
    size_t packetsiz;
    long filesize;
    uint32_t chunk = 0;
    long sent_bytes = 0;
    long chunk_len = CHUNK_SIZE;

    char *data = img_read(imgfile, &filesize);

    /* RAWソケット作成 */
    if ((sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
        errx(1, "socket");
    if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
        errx(1, "setsockopt");

    src.s_addr = inet_addr(src_ip);
    dst.s_addr = inet_addr(dst_ip);

    while (sent_bytes < filesize) {
        if (filesize - sent_bytes < CHUNK_SIZE) {
            chunk_len = filesize - sent_bytes;
        }

        packetsiz = sizeof(struct ip) + sizeof(struct udphdr) + 4 + chunk_len;
        if ((buf = malloc(packetsiz)) == NULL)
            errx(1, "malloc");

        build_ip(buf, &src, &dst, packetsiz);
        build_udp(buf, &src, &dst, port, chunk, data + sent_bytes, chunk_len);

        memset(&to, 0, sizeof(struct sockaddr_in));
        to.sin_addr = dst;
        to.sin_port = htons(port);
        to.sin_family = AF_INET;

        if (sendto(sd, buf, packetsiz, 0, (struct sockaddr *)&to, tolen) < 0) {
            perror("sendto");
        }

        free(buf);
        sent_bytes += chunk_len;
        chunk++;
    }

    /* 終了チャンクの送信 */
    packetsiz = sizeof(struct ip) + sizeof(struct udphdr) + 4;
    buf = malloc(packetsiz);
    if (!buf)
        errx(1, "malloc (end chunk)");

    build_ip(buf, &src, &dst, packetsiz);
    build_udp(buf, &src, &dst, port, END_CHUNK, NULL, 0);

    memset(&to, 0, sizeof(struct sockaddr_in));
    to.sin_addr = dst;
    to.sin_port = htons(port);
    to.sin_family = AF_INET;

    if (sendto(sd, buf, packetsiz, 0, (struct sockaddr *)&to, tolen) < 0) {
        perror("sendto");
    }

    close(sd);
    free(data);
    return 0;
}

