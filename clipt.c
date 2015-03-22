/* clipt.c part of CLIPT (CLI Pomodoro timer)
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

// config file array indexes
#define	WORK		0
#define	BREAK		1
#define	LONG		2
#define LONG_AFTER	3
#define QUIT_AFTER	4
#define	SCRIPT		5

#define WORK_LEN_MIN	1
#define WORK_LEN_MAX	240
#define WORK_LEN_DEF	25
#define BREAK_LEN_MIN	1
#define BREAK_LEN_MAX	240
#define BREAK_LEN_DEF	5
#define LONG_BREAK_LEN_MIN	1
#define LONG_BREAK_LEN_MAX	240
#define LONG_BREAK_LEN_DEF	15

// number of pomodoros need to quit or take a long break after
#define DEF_LONG_BREAK_AFTER	4
#define DEF_QUIT_AFTER		16

#define	SOCKET_NAME		".clipt.sock"
#define CLIPT_RC_FILE		".clipt.rc"

#define MAX_CMD_SIZE	16

#define REQ_PAUSE	"pause"
#define REQ_RESUME	"resume"
#define REQ_RESET	"reset"
#define REQ_QUIT	"quit"
#define REQ_BREAK	"break"
#define REQ_WORK	"work"
#define REQ_STATUS	"status"

#define RESP_OK		"OK"
#define RESP_BAD	"bad request"


typedef enum { WORK_PHASE , BREAK_PHASE, LONG_BREAK_PHASE, QUIT_PHASE }	phase_type;

const char *valid_reqs[] = { 	REQ_PAUSE, REQ_RESUME, REQ_RESET, REQ_QUIT,
				REQ_BREAK, REQ_WORK, REQ_STATUS, NULL };

char *config_keys[] = { "work", "break",
		        "long_break", "long_break_after", "quit_after",
		        "script", NULL };

typedef struct _clipt_cfg {
	unsigned int	work_len;
	unsigned int	break_len;
	unsigned int	long_break_len;
	unsigned int	long_break_after;
	unsigned int	quit_after;
	char 		script[MAX_PATH+1];
	unsigned char	is_daemon;
	char		ttyname[MAX_PATH+1];
	char		sock_path[UNIX_PATH_MAX+1];
	char		rc_file[MAX_PATH+1];
	phase_type	current_phase;
	time_t		current_phase_start;
	time_t		current_phase_time;
	unsigned int	current_phase_len;	// in seconds
	unsigned int	pomodoros;
	unsigned char	paused;
} CLIPT_CFG;

CLIPT_CFG	cfg;

/*------------------------------------------------------------------------------
   Prints version info.
--------------------------------------------------------------------------------*/
void print_version(void)
{
	printf("CLIPT (CLI Pomodoro Timer), V%s By Faraz.V 2015-03-15\n", VERSION);
}

/*------------------------------------------------------------------------------
   Prints usage info.
--------------------------------------------------------------------------------*/
void print_usage(int exit_code)
{
	print_version();
	printf("Usage: %s [options ...] \n\n", PACKAGE);

	printf("Options are:\n\n");
	
	printf(" -w, --work  minutes	Specifies duration of work period in minutes([%d-%d] default: %d)\n",
		WORK_LEN_MIN, WORK_LEN_MAX, WORK_LEN_DEF);
	printf(" -b, --break minutes	Specifies duration of break period in minutes([%d-%d] default: %d)\n",
		BREAK_LEN_MIN, BREAK_LEN_MAX, BREAK_LEN_DEF);
	printf(" -l, --long  minutes	Specifies duration of the long break period in minutes([%d-%d] default: %d)\n",
		LONG_BREAK_LEN_MIN, LONG_BREAK_LEN_MAX, LONG_BREAK_LEN_DEF);
	printf(" -a, --long_after pomodoros	Number of pomodors to run before taking a long break (default: %d)\n",
		DEF_LONG_BREAK_AFTER);
	printf(" -q, --quit pomodoros	Number of pomodors to run before quitting(default: %d).\n",
		DEF_QUIT_AFTER);
	printf(" -r, --run script	Run the specified script after then of each phase.\n");
	printf("			It passes 3 arguments [w|b|l] (short for [work|break|long-break]),\n");
	printf("			lenght of the pase in minutes, and ttyname.\n");
	printf(" -n, --nodaemon		Do NOT become a daemon (default is to become a daemon)\n");
	printf(" -c, --cmd command	Pass command to the running daemon.\n");
	printf(" -h, --help		Display this.\n");
	printf(" -v, --version		Display the version number.\n\n");

	printf("Pomodoro Technique(R) and Pomodoro(TM) are registered and filed trademarks owned by Francesco Cirillo.\nCLIPT is not affiliated by, associated with nor endorsed by Francesco Cirillo.\n\n");

	exit(exit_code);
}

/*-----------------------------------------------------------------------------
   Shutdown the server
------------------------------------------------------------------------------*/
RETSIGTYPE shutdown_server(int sig)
{
	// using global cfg;

	unlink(cfg.sock_path);
	_exit(0);
}

/*-----------------------------------------------------------------------------
   Initialize the server data structures and variables.
------------------------------------------------------------------------------*/
void init_server(CLIPT_CFG *cfg)
{
	signal(SIGPIPE, SIG_IGN);

	signal(SIGTERM, shutdown_server);
	signal(SIGINT, shutdown_server);
	signal(SIGQUIT, shutdown_server);
	signal(SIGHUP, shutdown_server);

	if (cfg && cfg->is_daemon)
		signal(SIGCLD, sig_cld);

	umask( (mode_t) S_IRWXG | S_IRWXO);
}

/*------------------------------------------------------------------------------
 *   Signal handler for SIGPIPE (write on a disconnected socket)
 *----------------------------------------------------------------------------*/
void do_abort(void)
{
        _exit(9);
}

/*-----------------------------------------------------------------------------
  clipt alarm
------------------------------------------------------------------------------*/
int clipt_load_rc(CLIPT_CFG *cfg)
{
	char 		**rc;
	unsigned int 	n;
	struct stat 	script_stat;
	int 		rv;

	if (!cfg) return -1;

	rc = read_cfg(cfg->rc_file, config_keys);
	if (!rc) return -1;

	if (rc[WORK]) {
		n = atoi(rc[WORK]);
		if (!strnum(rc[WORK]) || n<WORK_LEN_MIN || n>WORK_LEN_MAX) {
			fprintf(stderr, "Invalid work length '%s' in rc file\n", optarg);
			exit(2);
		}
		cfg->work_len = n;
	}

	if (rc[BREAK]) {
		n = atoi(rc[BREAK]);
		if (!strnum(rc[BREAK]) || n<BREAK_LEN_MIN || n>BREAK_LEN_MAX) {
			fprintf(stderr, "Invalid break length '%s' in rc file\n", optarg);
			exit(2);
		}
		cfg->break_len = n;
	}

	if (rc[LONG]) {
		n = atoi(rc[LONG]);
		if (!strnum(rc[LONG]) || n<LONG_BREAK_LEN_MIN || n>LONG_BREAK_LEN_MAX) {
			fprintf(stderr, "Invalid long break length '%s' in rc file\n", optarg);
			exit(2);
		}
		cfg->long_break_len = n;
	}

	if (rc[LONG_AFTER]) {
		n = atoi(rc[LONG_AFTER]);
		if (!strnum(rc[LONG_AFTER]) || n<=0) {
			fprintf(stderr, "Invalid value for long_after '%s' in rc file\n", optarg);
			exit(2);
		}
		cfg->long_break_after = n;
	}

	if (rc[QUIT_AFTER]) {
		n = atoi(rc[QUIT_AFTER]);
		if (!strnum(rc[QUIT_AFTER]) || n<=0) {
			fprintf(stderr, "Invalid value for quit_after '%s' in rc file\n", optarg);
			exit(2);
		}
		cfg->quit_after = n;
	}

	if (rc[SCRIPT]) {
		rv = stat(rc[SCRIPT], &script_stat);
		if (rv || !S_ISREG(script_stat.st_mode) ||
		    access((const char *) rc[SCRIPT], X_OK)) {
			fprintf(stderr, "Invalid script '%s' in rc file\n", rc[SCRIPT]);
			exit(2);
		}
		strncpy(cfg->script, rc[SCRIPT], MAX_PATH);
		cfg->script[MAX_PATH] = '\0';
	}

	return 0;
}

/*-----------------------------------------------------------------------------
  clipt alarm
------------------------------------------------------------------------------*/
void clipt_alarm(int signum)
{
	char		*phase, slen[6];
	extern char 	**environ;
	unsigned int	len=0, quit=0;
	// using global cfg

	if (cfg.current_phase == BREAK_PHASE || cfg.current_phase == LONG_BREAK_PHASE) {
		// START OF WORK_PHASE
		cfg.current_phase = WORK_PHASE;
		phase = "w";
		len = cfg.work_len;
	} else { // END OF WORK_PHASE
		cfg.pomodoros++;
		if (cfg.pomodoros < cfg.quit_after || cfg.quit_after == 0) { 
			if (cfg.pomodoros % cfg.long_break_after == 0) {
				cfg.current_phase = LONG_BREAK_PHASE;
				phase = "l";
				len = cfg.long_break_len;
			} else {
				cfg.current_phase = BREAK_PHASE;
				phase = "b";
				len = cfg.break_len;
			}
		} else {
			quit = 1;
			phase = "q";
			len = 0;
			cfg.current_phase = QUIT_PHASE;
		}
	}

	if (cfg.script && cfg.script[0] && fork() == 0) {
		snprintf(slen, 6, "%d", len);
		execle( (const char *) cfg.script, (const char *) cfg.script, 
			(const char *) phase, (const char *) slen, (const char *) cfg.ttyname,
			(const char *) NULL,
			environ);
	}

	if (quit)
		shutdown_server(0);

	cfg.current_phase_start = time(NULL);
	cfg.current_phase_time  = time(NULL);
	cfg.current_phase_len = len * 60;
	cfg.paused = 0;
	alarm(cfg.current_phase_len);
}

/*-----------------------------------------------------------------------------
  clipt client
------------------------------------------------------------------------------*/
int clipt_client(CLIPT_CFG *cfg, char *cmd)
{
	int	sd, n;
	char	buf[MAX_PATH+1];

	if (!cfg) return -1;

	sd = connect_server((const char *) cfg->sock_path);
	if (sd<0) {
		fprintf(stderr, "Failed connecting to server\n");
		return -1;
	}

	snprintf(buf, MAX_PATH, "%s\n", cmd);
	n = strlen(buf);
	if (write(sd, buf, n) != n) {
		fprintf(stderr, "Failed sending request to server\n");
		return -1;
	}
	n = read_data(sd, (unsigned char*) buf, MAX_PATH);
	if (n<0) {
		fprintf(stderr, "Failed receiving any reply from server\n");
		return -1;
	}
	buf[n]='\0';
	printf("%s\n", buf);

	return 0;
}
/*-----------------------------------------------------------------------------
  processes clipt client requests and provides a reply
------------------------------------------------------------------------------*/
int clipt_process(CLIPT_CFG *cfg, char *req, char *reply)
{
	char *tmp;
	unsigned min, sec;

	if (!cfg || !req || !reply || !req[0]) {
		strcpy(reply, RESP_BAD);
		return -1;
	}

	chomp(req);
	strlower(req);
	reply[0] = '\0';

	if (strcmp(req, REQ_PAUSE) == 0) {
		if (!cfg->paused) {
			cfg->current_phase_time = time(NULL);
			cfg->paused = 1;
			alarm(0);
		}
	} else if (strcmp(req, REQ_RESUME) == 0) {
		if (cfg->paused) {
			cfg->current_phase_len = cfg->current_phase_len -
						 (cfg->current_phase_time - cfg->current_phase_start);
			cfg->paused = 0;
			cfg->current_phase_start = time(NULL);
			cfg->current_phase_time = time(NULL);
			alarm(cfg->current_phase_len);
		}
        } else if (strcmp(req, REQ_RESET) == 0) {
		if (cfg->current_phase == WORK_PHASE) 
 			cfg->current_phase_len = cfg->work_len;
		else if (cfg->current_phase == BREAK_PHASE) 
 			cfg->current_phase_len = cfg->break_len;
                else if (cfg->current_phase == LONG_BREAK_PHASE) 
 			cfg->current_phase_len = cfg->long_break_len;
 		cfg->current_phase_len *= 60;
		cfg->current_phase_start = time(NULL);
		cfg->current_phase_time = time(NULL);
		cfg->paused = 0;
		alarm(cfg->current_phase_len);
        } else if (strcmp(req, REQ_QUIT) == 0) {
		shutdown_server(0);
        } else if (strcmp(req, REQ_BREAK) == 0) {
		if (cfg->current_phase == WORK_PHASE) {
			raise(SIGALRM);
		}
        } else if (strcmp(req, REQ_WORK) == 0) {
		if (cfg->current_phase != WORK_PHASE) {
			raise(SIGALRM);
		}
        } else if (strcmp(req, REQ_STATUS) == 0) {
		switch (cfg->current_phase) {
			case WORK_PHASE:
				tmp = "work";
				break;
			case BREAK_PHASE:
				tmp = "break";
				break;
			case LONG_BREAK_PHASE:
				tmp = "long_break";
				break;
			default:
				tmp = "unknown phase";
		}

		if (!cfg->paused)
			cfg->current_phase_time = time(NULL);

		sec = cfg->current_phase_len - (cfg->current_phase_time - cfg->current_phase_start);
		min = sec / 60;
		sec %= 60;

		snprintf(reply, MAX_PATH, "phase: %s\nstarted: %s\nleft: %d:%d\npaused: %d\npomodoros: %d\n",
					  tmp,
					  chomp(ctime(&(cfg->current_phase_start))),
					  min, sec,
					  cfg->paused,
					  cfg->pomodoros);
        } else {	
		strcpy(reply, RESP_BAD);
		return -1;
	}

	strncat(reply, RESP_OK, MAX_PATH);
	return 0;
}
/*-----------------------------------------------------------------------------
  clipt
------------------------------------------------------------------------------*/
int clipt(CLIPT_CFG *cfg)
{
	int 	sd, client_sd, n;
	char	req[MAX_CMD_SIZE+1], reply[MAX_PATH+1];

	if (!cfg) return -1;

	sd = open_server_socket(cfg->sock_path);
	if (sd < 0) {
		if (!cfg->is_daemon)
			fprintf(stderr, "Failed to listen on unix domain socket\n");
		return -1;
	}
	signal(SIGALRM, clipt_alarm);
	// start with work phase as it gets switched in the sig handler
	//cfg->current_phase = BREAK_PHASE;
	//raise(SIGALRM);
	cfg->current_phase = WORK_PHASE;
	cfg->current_phase_start = time(NULL);
	cfg->current_phase_time  = time(NULL);
	cfg->current_phase_len = cfg->work_len * 60;
	alarm(cfg->current_phase_len);

	while ( (client_sd = accept_connection(sd)) > 0 ) {
		memset(req, 0, sizeof(req));
		if ((n=read_line(client_sd, (unsigned char *)req, MAX_CMD_SIZE)) <=0 ) {
			close(client_sd);
			continue;
		}
		req[n] = '\0';
		clipt_process(cfg, req, reply);
		n = write(client_sd, reply, strlen(reply));
		close(client_sd);
	}

	return 0;
}

/*-----------------------------------------------------------------------------
  Initialize clipt
------------------------------------------------------------------------------*/
void clipt_init(CLIPT_CFG *cfg)
{
	struct passwd 	*pw;
	char		*tmp=NULL;

	if (!cfg) return;

	memset(cfg, 0, sizeof(CLIPT_CFG));
	cfg->work_len  = WORK_LEN_DEF;
	cfg->break_len = BREAK_LEN_DEF;
	cfg->long_break_len   	= LONG_BREAK_LEN_DEF; 
	cfg->long_break_after 	= DEF_LONG_BREAK_AFTER;
	cfg->quit_after 	= DEF_QUIT_AFTER;

	cfg->is_daemon = 1; // damon by default

	pw = getpwuid(getuid());
	if (!pw) {
		fprintf(stderr, "Failed determining home directory of user->\n");
		exit(4);
	}
	snprintf(cfg->sock_path, UNIX_PATH_MAX, "%s/%s", pw->pw_dir, SOCKET_NAME);
	snprintf(cfg->rc_file, MAX_PATH, "%s/%s", pw->pw_dir, CLIPT_RC_FILE);
	tmp = ttyname(0);
	if (!tmp) {
		fprintf(stderr, "Failed getting tty name->\n");
		exit(5);
	}
	strncpy(cfg->ttyname, tmp, MAX_PATH);
	cfg->current_phase = BREAK_PHASE;
	cfg->pomodoros = 0;
	cfg->paused = 0;
}

/*-----------------------------------------------------------------------------
  The Main function !
------------------------------------------------------------------------------*/
int main(int argc,char *argv[])
{
	int		next_option, rc=0, n=0, is_client=0;
	extern	char 	*optarg;	/* getopt */
	extern  int  	optind;	/* getopt */
	struct stat 	script_stat;
	char            cmd[MAX_CMD_SIZE+1] = { '\0' } ;

	clipt_init(&cfg);
	clipt_load_rc(&cfg);

	/* short options */
	const char *short_options = "w:b:r:c:nhv";
	
	/* long options */
	const struct option long_options[] = {
		{"work",	1, NULL, 'w'},
		{"break",	1, NULL, 'b'},
		{"long",	1, NULL, 'l'},
		{"long_after",	1, NULL, 'a'},
		{"quit",	1, NULL, 'q'},
		{"run",		1, NULL, 'r'},
		{"cmd",		1, NULL, 'c'},
		{"nodaemon",	0, NULL, 'n'},
		{"help",	0, NULL, 'h'},
		{"version",	0, NULL, 'v'},
		{NULL, 		0, NULL, 0}
	};

	// TODO: read the config file

	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch (next_option) {
			case 'w':
				n = atoi(optarg);
				if (!strnum(optarg) || n<WORK_LEN_MIN || n>WORK_LEN_MAX) {
					fprintf(stderr, "Invalid work length '%s'\n", optarg);
					exit(2);
				}
				cfg.work_len = n;
				break;
			case 'b':
				n = atoi(optarg);
				if (!strnum(optarg) || n<BREAK_LEN_MIN || n>BREAK_LEN_MAX) {
					fprintf(stderr, "Invalid break length '%s'\n", optarg);
					exit(2);
				}
				cfg.break_len = n;
				break;
			case 'l':
				n = atoi(optarg);
				if (!strnum(optarg) || n<LONG_BREAK_LEN_MIN || n>LONG_BREAK_LEN_MAX) {
					fprintf(stderr, "Invalid long break length '%s'\n", optarg);
					exit(2);
				}
				cfg.long_break_len = n;
				break;
			case 'a':
				n = atoi(optarg);
				if (!strnum(optarg) || n<=0) {
					fprintf(stderr, "Invalid value for length_break_after '%s'\n", optarg);
					exit(2);
				}
				cfg.long_break_after = n;
				break;
			case 'q':
				n = atoi(optarg);
				if (!strnum(optarg) || n<=0) {
					fprintf(stderr, "Invalid value for quit parameter '%s'\n", optarg);
					exit(2);
				}
				cfg.quit_after= n;
				break;
			case 'r':
				strncpy(cfg.script, optarg, MAX_PATH);
				cfg.script[MAX_PATH] = '\0';
				rc = stat(cfg.script, &script_stat);
				if (rc || !S_ISREG(script_stat.st_mode) || 
				    access((const char *) cfg.script, X_OK)) {
					fprintf(stderr, "Invalid script '%s'\n", optarg);
					exit(2);
				}
				break;
			case 'c':
				strncpy(cmd, optarg, MAX_CMD_SIZE);
				cmd[MAX_CMD_SIZE] = '\0';
				for(n=0; valid_reqs[n]; n++) {
					if (!strcmp(cmd, valid_reqs[n])) break;
				}
				if (!valid_reqs[n]) {
					fprintf(stderr, "Invalid command '%s'\n", cmd);
					exit(2);
				}
				break;
			case 'n':
				cfg.is_daemon = 0;
				break;
			case 'h':
				print_usage(0);
				break;
			case 'v':
				print_version();
				exit(0);
				break;
			case -1:
				/* done with options! */
				break;
			case '?':
			default:
				print_usage(1);
		}
	} while (next_option != -1);

	/* if there are non-option parameters print usage and exit! */
	if (optind<argc) 
		print_usage(1);

	is_client = cmd[0];
	cfg.is_daemon = is_client ? 0 : cfg.is_daemon;

	if (!is_client) {
		// make sure only one instance of the server is running
	}

	if (!is_client && !cfg.script[0])
		fprintf(stderr, "Warning: no script is identified\n");


	if (cfg.is_daemon) {
		if (daemon(0, 0)) {
			fprintf(stderr, "Failed becoming a daemon.\n");
			exit(3);
		}
	}

	if (is_client) {
		clipt_client(&cfg, cmd);
	} else {
		init_server(&cfg);
		clipt(&cfg);
	} 

	return 0;
}

