/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  RTSP Client
 *  -----------
 *  Written by Eduardo Silva P. <edsiper@gmail.com>
 */

#include <errno.h>

#ifndef RTPS_UTILS_H
#define RTPS_UTILS_H

#define PUT_16(p,v) ((p)[0]=((v)>>8)&0xff,(p)[1]=(v)&0xff)
#define PUT_32(p,v) ((p)[0]=((v)>>24)&0xff,(p)[1]=((v)>>16)&0xff,(p)[2]=((v)>>8)&0xff,(p)[3]=(v)&0xff)
#define GET_16(p) (((p)[0]<<8)|(p)[1])
#define GET_32(p) (((p)[0]<<24)|((p)[1]<<16)|((p)[2]<<8)|(p)[3])

/* ansi colors */
#define ANSI_BOLD "\033[1m"
#define ANSI_CYAN "\033[36m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_RED "\033[31m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_GREEN "\033[32m"
#define ANSI_WHITE "\033[37m"
#define ANSI_RESET "\033[0m"

/* request/response debug colors */
#define RES_HEADER  ANSI_BOLD ANSI_GREEN
#define REQ_HEADER  ANSI_BOLD ANSI_YELLOW

/* Debug macros */
#define DEBUG_REQ(...)  if (1) {      \
  printf(REQ_HEADER);                           \
  printf(__VA_ARGS__);                          \
  printf(ANSI_RESET "\n");}

#define DEBUG_RES(...)  if (1) {                              \
    printf(RES_HEADER);                                                 \
    printf(__VA_ARGS__);                                                \
    printf(ANSI_RESET "\n");}

#define RTSP_INFO(...) printf(ANSI_BOLD "[RTSP] " ANSI_RESET);  \
    printf(__VA_ARGS__);
#define RTP_INFO(...)  printf(ANSI_BOLD "[RTP ] " ANSI_RESET);  \
    printf(__VA_ARGS__);

/* Check if a bit is 1 or 0 */
#define CHECK_BIT(var, pos) !!((var) & (1 << (pos)))

/* Error debug */
#define ERR()     str_error(errno, __FILE__, __LINE__, __FUNCTION__)

/* fcntl pipe value */
#ifndef F_LINUX_SPECIFIC_BASE
#define F_LINUX_SPECIFIC_BASE       1024
#endif
#ifndef F_SETPIPE_SZ
#define F_SETPIPE_SZ	(F_LINUX_SPECIFIC_BASE + 7)
#endif
#ifndef F_GETPIPE_SZ
#define F_GETPIPE_SZ	(F_LINUX_SPECIFIC_BASE + 8)
#endif

void str_error(int errnum, const char *file, int line, const char *func);

#endif
