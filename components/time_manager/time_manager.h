#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H


void initialize_sntp(void);
void obtain_time(char* buffer, size_t buffer_size);

#endif