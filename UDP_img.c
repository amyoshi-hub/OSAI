#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifndef __FAVOR_BSD
# define __FAVOR_BSD
#endif
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <err.h>
#define CHUNK_SIZE 1024
#define END_CHUNK 0xFFFFFFFF

/* チェックサム計算用UDP疑似ヘッダー */
struct pseudo_hdr {
    struct in_addr src;
    struct in_addr dst;
    unsigned char zero;
    unsigned char proto;
    unsigned short len;
};


static void
usage(char *prog)
{

    fprintf(stderr, "Usage: %s <src ip> <dst ip> <port> <imgfile>", prog);
    exit(EXIT_FAILURE);
}

/*
 * チェックサム計算コード
 * ping.cから流用
 */
static unsigned short
in_cksum(unsigned short *addr, int len)
{
  int nleft, sum;
  unsigned short *w;
  union {
    unsigned short us;
    unsigned char  uc[2];
  } last;
  unsigned short answer;

  nleft = len;
  sum = 0;
  w = addr;

  /*
   * Our algorithm is simple, using a 32 bit accumulator (sum), we add
   * sequential 16 bit words to it, and at the end, fold back all the
   * carry bits from the top 16 bits into the lower 16 bits.
   */
  while (nleft > 1)  {
    sum += *w++;
    nleft -= 2;
  }

  /* mop up an odd byte, if necessary */
  if (nleft == 1) {
    last.uc[0] = *(unsigned char *)w;
    last.uc[1] = 0;
    sum += last.us;
  }

  /* add back carry outs from top 16 bits to low 16 bits */
  sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
  sum += (sum >> 16);                     /* add carry */
  answer = ~sum;                          /* truncate to 16 bits */
  return(answer);
}

static void
build_udp(char *p, struct in_addr *src, struct in_addr *dst, unsigned short dport, uint32_t chunk, char *data, long datasize)
{
    char *ubuf;
    struct ip *ip;
    struct udphdr *udp;
    struct pseudo_hdr *pse;
    int needlen;

    /* チェックサム計算用にUDPヘッダーとデータ、疑似ヘッダの合計サイズを計算する */
    //4はchunk
    needlen = sizeof(struct pseudo_hdr) + sizeof(struct udphdr) + 4 + datasize;
    if ((ubuf = malloc(needlen)) == NULL)
        errx(1, "malloc");
    memset(ubuf, 0, needlen);

    pse = (struct pseudo_hdr *)ubuf;
    pse->src.s_addr = src->s_addr;
    pse->dst.s_addr = dst->s_addr;
    pse->proto = IPPROTO_UDP;
    pse->len = htons(sizeof(struct udphdr) + 4 + datasize);

    udp = (struct udphdr *)(ubuf + sizeof(struct pseudo_hdr));
    udp->uh_sport = htons(65001);
    udp->uh_dport = htons(dport);
    udp->uh_ulen = pse->len;
    udp->uh_sum = 0;
    /* データ部分の書き込み */
    uint32_t netchunk = htonl(chunk);

    memcpy((char *)udp + sizeof(struct udphdr), &netchunk, 4);
    memcpy((char *)udp + sizeof(struct udphdr) + 4, data, datasize);
    /* チェックサム計算 */
    udp->uh_sum = in_cksum((unsigned short *)ubuf, needlen);

    /* UDPヘッダーとデータ部分をIPヘッダーの後ろへ書き込む */
    ip = (struct ip *)p;
    memcpy(p + (ip->ip_hl << 2), udp, needlen - sizeof(struct pseudo_hdr));

    free(ubuf);
}

static void
build_ip(char *p, struct in_addr *src, struct in_addr *dst, size_t len)
{
    struct ip *ip;

    ip = (struct ip *)p;
    ip->ip_v = 4;                /* もちろんIPv4 */
    ip->ip_hl = 5;               /* IPオプションは使わないので5に決め打ち */
    ip->ip_tos = 1;              /* TOSが設定されることを確認するため1に設定 */
    ip->ip_len = len;            /* 今回送信する全パケット長 */
    ip->ip_id = htons(getpid()); /* IDは何でもいいので、今回はプロセスIDとする */
    ip->ip_off = 0;              /* フラグメント化させない */
    ip->ip_ttl = 0x40;           /* TTL */
    ip->ip_p = IPPROTO_UDP;      /* UDPなので */
    ip->ip_src = *src;           /* 送信元IPアドレス */
    ip->ip_dst = *dst;           /* 送信先IPアドレス */

    /* チェックサム計算 */
    ip->ip_sum = 0;
    ip->ip_sum = in_cksum((unsigned short*)ip, ip->ip_hl << 2);
}

char* img_read(char* filepath, long *filesize){
    FILE *fp = fopen(filepath, "rb");
    if(!fp){
   	perror("foepn"); 
	exit(1);
    }
    fseek(fp, 0, SEEK_END);
    *filesize = ftell(fp);
    rewind(fp);

    char *data = malloc(*filesize);
    if(!data){
   	perror("malloc"); 
	exit(1);
    }
    fread(data, 1, *filesize, fp);

    fclose(fp);
    return data;
}

int main(int argc, char *argv[])
{
    int sd;
    int on = 1;
    char *buf;
    struct in_addr src, dst;
    struct sockaddr_in to;
    socklen_t tolen = sizeof(struct sockaddr_in);
    size_t packetsiz;
    unsigned short dport;
    long filesize;
    uint32_t chunk = 0;
    long sent_bytes = 0;
    long chunk_len = CHUNK_SIZE;

    if (argc != 5)
        usage(argv[0]);

    dport = atoi(argv[3]);

    char *data = img_read(argv[4], &filesize);

      /* RAWソケット */
    if ((sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
        errx(1, "socket");
    /* 送信パケットにIPヘッダーを含めるためのソケットオプション */
    if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
        errx(1, "setsockopt");

    src.s_addr = inet_addr(argv[1]);
    dst.s_addr = inet_addr(argv[2]);

    while(sent_bytes < filesize){
    	if(filesize - sent_bytes < CHUNK_SIZE){
   		chunk_len = filesize - sent_bytes; 
    	}    

    	packetsiz = sizeof(struct ip) + sizeof(struct udphdr) + 4 + chunk_len;
    	if ((buf = malloc(packetsiz)) == NULL)
        	errx(1, "malloc");

    	build_ip(buf, &src, &dst, packetsiz);
    	build_udp(buf, &src, &dst, dport, chunk, data + sent_bytes, chunk_len);

    	memset(&to, 0, sizeof(struct sockaddr_in));
    	to.sin_addr = dst;
    	to.sin_port = htons(dport);
    	to.sin_family = AF_INET;

    	printf("Sending to %s from %sn", argv[2], argv[1]);
    	if (sendto(sd, buf, packetsiz, 0, (struct sockaddr *)&to, tolen) < 0) {
        	perror("sendto");
    	}
    	free(buf);
    	sent_bytes += chunk_len;
    	chunk++;
    }

    uint32_t end_chunk = UINT32_MAX;
    chunk_len = 0;

    packetsiz = sizeof(struct ip) + sizeof(struct udphdr) + 4 + chunk_len;
    buf = malloc(packetsiz);
    if(!buf) errx(1, "malloc (end chunk)");

    build_ip(buf, &src, &dst, packetsiz);
    build_udp(buf, &src, &dst, dport, END_CHUNK, data + sent_bytes, chunk_len);

    memset(&to, 0, sizeof(struct sockaddr_in));
    to.sin_addr = dst;
    to.sin_port = htons(dport);
    to.sin_family = AF_INET;
    if (sendto(sd, buf, packetsiz, 0, (struct sockaddr *)&to, tolen) < 0) {
    	perror("sendto");
    }

    close(sd);
    free(data);

    return 0;
}
