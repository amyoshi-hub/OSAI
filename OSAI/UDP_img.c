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

static unsigned short in_cksum(unsigned short *addr, int len) {
    int sum = 0;
    unsigned short result;

    while (len > 1) {
        sum += *addr++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(unsigned char *)addr;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

static void build_ip(char *p, struct in_addr *src, struct in_addr *dst, size_t len) {
    struct ip *ip_hdr = (struct ip *)p;
    ip_hdr->ip_hl = 5;
    ip_hdr->ip_v = 4;
    ip_hdr->ip_tos = 0;
    ip_hdr->ip_len = htons(len);
    ip_hdr->ip_id = 0;
    ip_hdr->ip_off = 0;
    ip_hdr->ip_ttl = 64;
    ip_hdr->ip_p = IPPROTO_UDP;
    ip_hdr->ip_sum = 0;
    ip_hdr->ip_src = *src;
    ip_hdr->ip_dst = *dst;
    ip_hdr->ip_sum = in_cksum((unsigned short *)ip_hdr, sizeof(struct ip));
}
static void build_udp(char *p, struct in_addr *src, struct in_addr *dst, unsigned short dport, uint32_t chunk, char *data, long datasize) {
    struct udphdr *udp_hdr = (struct udphdr *)(p + sizeof(struct ip));
    struct pseudo_hdr phdr = { *src, *dst, 0, IPPROTO_UDP, htons(sizeof(struct udphdr) + datasize + 4) };

    udp_hdr->uh_sport = htons(12345); // 任意の送信元ポート番号
    udp_hdr->uh_dport = htons(dport);
    udp_hdr->uh_ulen = htons(sizeof(struct udphdr) + datasize + 4);
    udp_hdr->uh_sum = 0;

    memcpy((char *)(udp_hdr + 1), &chunk, 4);
    if (datasize > 0) {
        memcpy((char *)(udp_hdr + 1) + 4, data, datasize);
    }

    udp_hdr->uh_sum = in_cksum((unsigned short *)&phdr, sizeof(phdr));
}

#include <stdio.h>
#include <stdlib.h>

static char *img_read(const char *filepath, long *filesize) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("fopen");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *filesize = ftell(file);
    rewind(file);

    char *data = malloc(*filesize);
    if (!data) {
        perror("malloc");
        fclose(file);
        return NULL;
    }

    fread(data, 1, *filesize, file);
    fclose(file);

    return data;
}



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

