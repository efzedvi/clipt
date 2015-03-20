/* utils.c part of CLIPT (CLI Pomodoro timer)
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

#include "includes.h"
#include "utils.h"

#define BACKLOG         5
#define	MAX_LINE_SIZE	256

/*-----------------------------------------------------------------------------
    To prevent zombie child processes.
------------------------------------------------------------------------------*/
RETSIGTYPE sig_cld()
{
       //signal(SIGCLD, sig_cld);
       //while (waitpid((pid_t)-1, (int *) NULL, WNOHANG) > 0);

	while (wait3((int *)NULL, WNOHANG, (struct rusage *)NULL) > 0);
}

/*------------------------------------------------------------------
  compare 2 strings 
-------------------------------------------------------------------*/
int strequal(char *s1, char *s2)
{
	if (!s1 || !s2) 
		return(0);
	    
	return(strcasecmp(s1,s2)==0);
}

/*------------------------------------------------------------------
 convert a string to lower case
------------------------------------------------------------------*/
void strlower(char *s)
{
	while (*s) {
		*s = tolower(*s);
	        s++;
        }
}

/*------------------------------------------------------------------
  convert a string to upper case
------------------------------------------------------------------*/
void strupper(char *s)
{
	while (*s) {
		*s = toupper(*s);
	        s++;
	}
}

/*-----------------------------------------------------------------
  string replace
-----------------------------------------------------------------*/
int strnum(char *s)
{
	while (*s) {
		if (!isdigit(*s)) 
			return 0;
		s++;
	}
	return 1;
}

/*-----------------------------------------------------------------
  chomp
-----------------------------------------------------------------*/
char *chomp(char *s)
{
	int i=0;
	if (s) {
		for(i=strlen(s)-1; i>=0; i--) {
			if (s[i] != '\n' && s[i] != '\b' && s[i] != '\t')
				break;
			else
				s[i] = '\0';
		}
	}
	return s;
}

/*-----------------------------------------------------------------
 Calculate the difference in timeout values. Return 1 if val1 > val2,
 0 if val1 == val2, -1 if val1 < val2. Stores result in retval. retval
 may be == val1 or val2
-----------------------------------------------------------------*/
int tval_sub( struct timeval *retval, struct timeval *val1, struct timeval *val2)
{
	long usecdiff = val1->tv_usec - val2->tv_usec;
	long secdiff = val1->tv_sec - val2->tv_sec;
	if(usecdiff < 0) {
		usecdiff = 1000000 + usecdiff;
		secdiff--;
	}
	retval->tv_sec = secdiff;
	retval->tv_usec = usecdiff;
	if(secdiff < 0)
		return -1;
	if(secdiff > 0)
		return 1;
	return (usecdiff < 0 ) ? -1 : ((usecdiff > 0 ) ? 1 : 0);
}

/*-----------------------------------------------------------------
  read data from peer
-----------------------------------------------------------------*/
int read_timeout(int fd, unsigned char *buf, unsigned int len, unsigned short timeout)
{
        int    rc=0;
	fd_set rfds;
	struct timeval tv;
	
	if (fd < 0 || len<=0) return 0;
	if (buf == NULL) return -1;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	rc = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);

	if (rc <=0 ) return rc;

	return read(fd, buf, len);
}

/*-----------------------------------------------------------------
  read data from peer
-----------------------------------------------------------------*/
int read_data(int fd, unsigned char *buf, unsigned int len)
{
        int  nread=0;
        int  total_read = 0;

	if (fd < 0) return -1;

        while (total_read < len) {
                nread = read(fd, buf+total_read, len-total_read);

                if (nread < 0)
                        return -1;

		if (nread==0)
			return total_read;

                total_read += nread;
        }
        return total_read;
}
/*-----------------------------------------------------------------
  read a line from peer
-----------------------------------------------------------------*/
int read_line(int fd, unsigned char *buf, unsigned int len)
{
        int  nread=0;
        int  total_read = 0;
	char c=0;

	if (fd < 0) return -1;

        while (total_read < len) {
                nread = read(fd, &c, 1);
                if (nread < 0) return -1;
		if (nread==0 || c=='\n') {
			break;
		}

		buf[total_read] = c;
                total_read += nread;
        }
	buf[total_read] = '\0';
        return total_read;
}


/*-----------------------------------------------------------------------------
 * open client socket!
------------------------------------------------------------------------------*/
int connect_server(const char *sock_path)
{
        struct sockaddr_un sock;
        int    sd=0;

	if (!sock_path)
		return -1;

        if ( (sd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0 ) {
                return -1;
        }
        memset(&sock, 0, sizeof(sock));
        sock.sun_family = AF_UNIX;
	strncpy(sock.sun_path, sock_path, sizeof(sock.sun_path)-1);

        if ( connect(sd ,(struct sockaddr *)&sock, sizeof(sock)) < 0 ) {
                close(sd);
                return -1;
        }

        return sd;
}

/*-----------------------------------------------------------------------------
 * open server socket!
------------------------------------------------------------------------------*/
int open_server_socket(const char *sock_path)
{
	struct sockaddr_un addr;
        int    sd=0;

	if (!sock_path)
		return -1;

        sd = socket(PF_UNIX, SOCK_STREAM, 0);
        
        if (sd == -1) return -1;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path)-1);

        /* now we've got a socket - we need to bind it */
        if (bind(sd, (struct sockaddr * ) &addr, sizeof(addr)) < 0) {
                close(sd);
                return -1;
        }

	if (listen(sd, BACKLOG) == -1) {
                close(sd);
                return -1;
        }
	fcntl(sd, F_SETFD, FD_CLOEXEC);

	return sd;
}

/*-----------------------------------------------------------------------------
 * Accept a new connection and take care of the error codes
------------------------------------------------------------------------------*/
int accept_connection(int sd_accept)
{
	int sd = -1;
	struct sockaddr_un 	addr;
	socklen_t		addrlen = sizeof(struct sockaddr_un);

	if (sd_accept < 0)
		return -1;

	while (sd < 0) {
		sd = accept(sd_accept, (struct sockaddr *) &addr, (socklen_t *) &addrlen);

		if (sd==-1) {
			return sd;
		}
	}
	fcntl(sd, F_SETFD, FD_CLOEXEC);

	return sd;
}

/*-----------------------------------------------------------------------------
 * reads a simple config file
------------------------------------------------------------------------------*/
char** read_cfg(const char *filename, char *keys[])
{
	FILE	*fp;
	char 	line[MAX_LINE_SIZE], first[MAX_LINE_SIZE], second[MAX_LINE_SIZE];
	char	ch, **cfg;
	int	i=0;
	
	for(i=0; keys[i]; i++);
	cfg = malloc(i * sizeof(char *));
	for(i=0; keys[i]; i++)
		cfg[i] = NULL;

	if (!filename || !filename[0] || !cfg)
		return NULL;

	fp = fopen(filename, "r");
	if (!fp)
		return NULL;
	
	while (!feof(fp)) {
		if (!fgets(line, MAX_LINE_SIZE, fp))
			break;
		
		line[MAX_LINE_SIZE-1] = '\0';
		i = strlen(line)-1; 
		if (i<0) continue;
		if (line[strlen(line)-1] != '\n' || line[0] == '#' || line[0] == '\b' || line[0] == '\t') {
			// skip the linke or the rest
			do {
				ch = fgetc(fp);
			} while (ch != '\n' || ch != EOF);
			if (line[0] == '#' || line[0] == '\b' || line[0] == '\t')
				continue;
		} else {
			line[i] = '\0';
		}
		if (sscanf((const char *) line, "%s %s", first, second) != 2)
			continue;

		for(i=0; keys[i]; i++) {
			if (strcmp(keys[i], first) == 0) {
				if (cfg[i]) {
					free(cfg[i]);
					cfg[i] = NULL;
				}
				cfg[i] = strdup(second);
				break;
			}
		}
	}
	fclose(fp);

	return cfg;
}

