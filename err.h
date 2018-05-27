#ifndef SIK3_ERR_H
#define SIK3_ERR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Prints system call error info and ends program
extern void syserr(const char *fmt, ...);

// Prints error info and ends program
extern void fatal(const char *fmt, ...);

#endif //SIK3_ERR_H
