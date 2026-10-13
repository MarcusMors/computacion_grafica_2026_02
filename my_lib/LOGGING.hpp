#ifndef __LOGGING_H__
#define __LOGGING_H__

#define ENABLE_LOGGING 0

#if ENABLE_LOGGING
void HandleInterruptSignal(int signal);
std::string log_path();
#endif

#endif// __LOGGING_H__
