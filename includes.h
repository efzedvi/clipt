/*  
   Copyright (C) 2009	Faraz.V (faraz@fzv.ca)
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h> 
#include <pwd.h> 
#include <unistd.h>
#include <asm/types.h> 
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>
#include <assert.h>
#include <bits/sockaddr.h>
#include <netdb.h>
#include <string.h>

#include <sys/un.h>

#ifdef HAVE_ANSIDECL_H
#include <ansidecl.h>
#endif
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#define HAVE_TIMEZONE
#define HAVE_SYSCONF 1
#define HAVE_FCNTL_LOCK 1

#ifndef ENOTSOCK
#define ENOTSOCK EINVAL
#endif

#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif 

#ifndef GETWD_CACHE
#define GETWD_CACHE 0
#endif

#ifndef HAVE_FCNTL_LOCK
#define HAVE_FCNTL_LOCK 1
#endif

