/* includes.h  part of CLIPT (CLI Pomodoro timer)
Copyright (c) 2015, Faraz.V (faraz@fzv.ca)

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.  
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
#ifdef HAVE_FCNTL_H
#include <fcntl.h> 
#endif
#include <pwd.h> 
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ASM_TYPES_H
#include <asm/types.h> 
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <getopt.h>
#include <assert.h>
#ifdef HAVE_BITS_SOCKADDR_H 
#include <bits/sockaddr.h>
#endif
#include <netdb.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif

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

