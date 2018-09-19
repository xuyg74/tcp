#ifndef _PTHREAD_TOOLS_
#define _PTHREAD_TOOLS_

#include <pthread.h>

int get_thread_policy( pthread_attr_t *attr);
void show_thread_priority( pthread_attr_t *attr, int policy);
int get_thread_priority(pthread_attr_t *attr);
void set_thread_policy( pthread_attr_t *attr,  int policy);

#endif