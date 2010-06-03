/* kill.c -- terminate a process
 
 There should be a command that does kill and works its way up to -9.
 http://news.ycombinator.com/item?id=1395876
 
 The kill utility initially sends a TERM signal to the processes specified by the pid
 operand(s).  It then waits a second to allow the process to terminate, and then resends
 the signal to check if the process terminated.  If the process didn't terminate the
 signal is escalated to a higher level of termination as follows:

 15      TERM (software termination signal)
 1       HUP (hang up)
 2       INT (interrupt)
 3       QUIT (quit)
 6       ABRT (abort)
 9       KILL (non-catchable, non-ignorable kill)

 Only the super-user may send signals to other users' processes.
 
 The following pids have special meanings:
 -1		If superuser, broadcast the signal to all processes;
		otherwise broadcast to all processes belonging to the user.
 
 Mark Stock
 http://hoop-la.ca/software/contact.html
 
 Compile with: make kill

 Usage: ./kill pid
 
 Example session:

 $ ps
 PID TTY           TIME CMD
 268 ttys000    0:00.38 -bash

 Command-N: create a New Window in Terminal

 $ ps
 PID TTY           TIME CMD
 268 ttys000    0:00.39 -bash
 1459 ttys001    0:00.01 -bash

 $ ./kill 1459
 Sending a "terminate" signal 15 to process id 1459 ...
 Sending a "hang up" signal 1 to process id 1459 ...
 Terminated process id 1459.

 $ ps
 PID TTY           TIME CMD
 268 ttys000    0:00.39 -bash
 
 bugs:
 
 The time to wait to allow the process to terminate is one second.
 This may be a problem because it may not be enough time.
 
 I don't recommend using "./kill 0"
 
 Is "kill" a good name for this program?

 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

typedef struct {
	int sig;
	char * description;
} t_sig;

int main(int argc, char * argv[])
{
	pid_t pid;
	int error = -1;
	int usage = -1;
	t_sig sigs[] = {
		{ SIGTERM, "terminate" },
		{ SIGHUP, "hang up" },
		{ SIGINT, "interrupt" },
		{ SIGQUIT, "quit" },
		{ SIGABRT, "abort" },
		{ SIGKILL, "kill" }
	};
	int siglevel;
	int tries;
	
	if (argc == 2) {
		pid = atol(argv[1]);
		if (pid || (argv[1][0] == '0' && argv[1][1] == 0)) {
			usage = 0;
			siglevel = 0;
			tries = 0;
			do {
				if (tries || siglevel)
					sleep(1);
				if (!tries) {
					printf("Sending a \"%s\" signal %d to process id %s ...\n", sigs[siglevel].description, sigs[siglevel].sig, argv[1]);
					fflush(stdout);
				}
				error = kill(pid, sigs[siglevel].sig);
				if (error) {
					error = errno;
					switch (error) {
						case EINVAL:
							fprintf(stderr, "Sig is not a valid, supported signal number.\n");
							break;
						case EPERM:
							fprintf(stderr, "The sending process is not the super-user and its\n");
							fprintf(stderr, "effective user id does not match the effective\n");
							fprintf(stderr, "user-id of the receiving process.  When signaling\n");
							fprintf(stderr, "a process group, this error is returned if any\n");
							fprintf(stderr, "members of the group could not be signaled.\n");
							break;
						case ESRCH:
							if (pid) {
								if (tries || siglevel) {
									fprintf(stderr, "\tTerminated process id %s.\n", argv[1]);
									fflush(stderr);
								} else {
									fprintf(stderr, "No process or process group can be found corre");
									fprintf(stderr, "sponding to that specified by pid.\n");
								}
							} else {
								fprintf(stderr, "The process id was given as 0, but the sending\n");
								fprintf(stderr, "process does not have a process group.\n");
							}
							break;
						default:
							fprintf(stderr, "unknown error %d\n", error);
					}
				}
				tries++;
				if (tries > 1) {
					tries = 0;
					if (sigs[siglevel].sig == SIGKILL) {
						fprintf(stderr, "Unable to kill.  Tried every signal up to and including KILL.\n");
						error = 0;
					} else
						siglevel++; // escalate!
				}
			} while (!error);
		}
	}
	if (usage)
		printf("usage: kill pid\n");

	return error;
}
