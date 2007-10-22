#ifndef DMLANGSEL_H
#define DMLANGSEL_H

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#if 1
#define DMLS_TRACE(s, ...) fprintf (stderr, "%d:%s:%s():%d "s"\n", getpid(), __FILE__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define DMLS_TRACE(s, ...)
#endif

#define DMLS_TRACEMSG(s) DMLS_TRACE("%s", (s))
#define DMLS_ENTER DMLS_TRACEMSG("ENTER")
#define DMLS_EXIT DMLS_TRACEMSG("EXIT")

#endif /* DMLANGSEL_H */
