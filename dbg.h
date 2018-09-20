#ifndef _DBG_
#define _DBG_

#ifdef debug
#define DEBUG_INFO(args...) printf(args)
#else
#define DEBUG_INFO(args...)
#endif
#endif