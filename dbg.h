#ifndef _DBG_
#define _DBG_

long rcv_cnt;
long sent_cnt;
long server_cnt;

#ifdef debug
#define DEBUG_INFO(args...) printf(args)
#else
#define DEBUG_INFO(args...)
#endif

#define ERR_INFO(args...) printf(args)
#define OUT_INFO(args...) printf(args)
#endif