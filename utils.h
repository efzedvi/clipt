/* utils.h is part of dummyd package
   Copyright (C) 2009	Faraz.V (faraz@fzv.ca)
*/

#ifndef __UTILS_H__
#define __UTILS_H__

#define DEBUG

#ifdef __cplusplus
  extern "C" {
#endif 

#ifndef MAX_PATH
#define  MAX_PATH	254
#endif

#ifndef UNIX_PATH_MAX
#define  UNIX_PATH_MAX	108
#endif

int 	strequal(char *s1, char *s2);
void 	strlower(char *s);
void 	strupper(char *s);
int	strnum(char *s);
char 	*chomp(char *s);
int 	read_data(int fd, unsigned char *buf, unsigned int len);
int 	read_line(int fd, unsigned char *buf, unsigned int len);
int	read_timeout(int fd, unsigned char *buf, unsigned int len, unsigned short timeout);
int 	parse_host(char *hostport, char *host, unsigned short *port);
int 	connect_server(const char *sock_path);
int	open_server_socket(const char *sock_path);
RETSIGTYPE sig_cld();
int 	accept_connection(int sd_accept);

char** 	read_cfg(const char *filename, char *keys[]);
#ifdef __cplusplus
  }
#endif 

#endif

