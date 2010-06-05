/* kill.c -- terminate a process
 
There should be a command that does kill and works its way up to -9.
 http://news.ycombinator.com/item?id=1395876
 
Mark Stock
 http://hoop-la.ca/software/contact.html
 
Compile with: make kill
 
Example session:
 
 faster:pussycat kill$ ps
 PID TTY           TIME CMD
 666 ttys000    0:00.38 -bash
 
 * open a new terminal/shell window
 
 faster:pussycat kill$ ps
 PID TTY           TIME CMD
 666 ttys000    0:00.39 -bash
 1965 ttys001    0:00.01 -bash
 
 faster:pussycat kill$ ./kill -v 1965
 Sending a "terminate" signal 15 to process id 1965 ...
 Sending a "hang up" signal 1 to process id 1965 ...
	Terminated process id 1965.
 
 faster:pussycat kill$ ps
 PID TTY           TIME CMD
 666 ttys000    0:00.39 -bash
 
Description:
 
 This kill utility initially sends a TERM signal to the processes specified by the pid.
 It then waits a second to allow the process to terminate, and then resends the signal
 to check if the process terminated.  If the process didn't terminate the signal is
 escalated to a higher level of termination as follows:
	15      TERM (software termination signal)
	1       HUP (hang up)
	2       INT (interrupt)
	3       QUIT (quit)
	6       ABRT (abort)
	9       KILL (non-catchable, non-ignorable kill)

 See also: kill(2)

*/

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#define k_eperm "Operation not permitted"
#define k_einval "Invalid argument"
#define k_esrch "No such process"

typedef struct {
	int sig;
	char * description;
} t_sig;
		 
int main(int argc, char * argv[])
{
	int error = 1, usage = 1, verbose = 0, siglevel = 0, tries = 0, check = 0;
	char * message = NULL;
	char * version[] = { "", "\tVersion 0.1 http://github.com/mmphosis/kill\n" };
	t_sig sigs[] = { // { 99999, "invalid" },
		{ SIGTERM, "terminate" },
		{ SIGHUP,  "hang up" },
		{ SIGINT,  "interrupt" },
		{ SIGQUIT, "quit" },
		{ SIGABRT, "abort" },
		{ SIGKILL, "kill" }
	};
	pid_t pid;
	char * pid_arg;
	
	if (argc > 1 && argv[1][0] == '-')
		verbose = (argv[1][1] == 'v');
	if (argc == 2 + verbose) {
		pid_arg = argv[1 + verbose];
		if (pid = atol(pid_arg)) { // I don't recommend using "./kill 0"
			do {
				if (verbose && !check) {
					printf("Sending a \"%s\" signal %d to process id %s ...\n",
						sigs[siglevel].description, sigs[siglevel].sig, pid_arg);
					fflush(stdout);
				}
				if (kill(pid, sigs[siglevel].sig)) {
					error = errno;
					if ((error == ESRCH) && pid && check) {
						if (verbose) printf("\tTerminated process id %s.\n", pid_arg);
						error = 0;
					}
					tries = 0;
				} else if (error = (check && sigs[siglevel].sig == SIGKILL)) {
					message = "Unable to kill";
					tries = 0;
				} else {
					tries++;
					siglevel = tries / 2;
					check = (tries & 1);
					sleep(1); // one second may not be enough time.
				}
			} while (tries);
			if (error) {
				if (!message) {
					message = "";
					switch (error) {
						case EINVAL: message = k_einval; break;
						case EPERM:  message = k_eperm; break;
						case ESRCH:  message = k_esrch; break;
						default: printf("unknown error %d", error);
					}
				}
				printf("%s\n", message);
			}
			usage = 0;
		}
	}
	if (usage) printf("usage: kill [-v] pid\n%s", version[verbose]);

	return error;
}
