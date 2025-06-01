#ifndef UDP_IMAGE_SENDER_H
#define UDP_IMAGE_SENDER_H

#include <netinet/in.h>
#include <stdint.h>

/* 画像データの送信 */
int send_image_over_udp(const char *src_ip, const char *dst_ip, unsigned short port, const char *imgfile);

#endif // UDP_IMAGE_SENDER_H

