#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <time.h>

#define BUFF_SIZE 15
#define NUM_OF_CMD 999999999
//pthread
pthread_mutex_t cmd_queue_lock;
pthread_cond_t cmd_buf_not_full;
pthread_cond_t cmd_buf_not_empty;

//errors
#define EINVAL 1
#define E2BIG 2

#define MAXMENUARGS 4
#define MAXCMDLINE 64

//globals
u_int buf_head;
u_int buf_tail;
u_int count;
u_int policy = 0;


void *commandLine();
void *executor();
int dispatch(char *cmd);

void menu_execute(char *line, int isargs);
int cmd_run(int nargs, char **args);
int cmd_quit(int nargs, char **args);
void showmenu(const char *name, const char *x[]);
int cmd_helpmenu(int n, char **a);
int cmd_dispatch(char *cmd);
int cmd_list(int nargs, char **args);
int cmd_fcfs(int nargs, char **args);
int cmd_sjf(int nargs, char **args);
int cmd_priority(int nargs, char **args);

//job properties
struct job_def {
	char name[50];
	int priority, burst, position;
	time_t arrival;
};

//job buffer
struct job_def jobBuffer[BUFF_SIZE - 1];

/*
 *  * The run command - submit a job.
 *   */
int cmd_run(int nargs, char **args) { 
	int i = 0;
	time_t rtime = time(NULL);
	if (nargs != 4) {
		return EINVAL;
	}
	sscanf(args[1], "%s", &jobBuffer[buf_head].name);
	sscanf(args[2], "%u", &jobBuffer[buf_head].priority);
	sscanf(args[3], "%u", &jobBuffer[buf_head].burst);
	jobBuffer[buf_head].arrival = rtime;
	count = count + 1;
	buf_head = buf_head + 1;
	jobBuffer[buf_head].position = buf_head;
	pthread_cond_signal(&cmd_buf_not_empty);
	return 0; /* if succeed */
}
int cmd_fcfs(int nargs, char **args) {
	policy = 0;
	struct job_def tempJob;
	int j;
	int minJ;
	int i;
	int k = buf_head;
	int l;
	for (l = 0; l < count; l++) {
		if (k == BUFF_SIZE) {
			k = 0;
		}
		j = k;
		long int minArrival = jobBuffer[j].arrival;
		for (i = l; i < count; i++) {
			long int temp = jobBuffer[j].arrival;
			if (temp < minArrival) {
				minArrival = temp;
				minJ = j;
			}
			j++;
		}
		if (minJ != k) {
			strcpy(tempJob.name, jobBuffer[k].name);
			tempJob.burst = jobBuffer[k].burst;
			tempJob.priority = jobBuffer[k].priority;
			tempJob.arrival = jobBuffer[k].arrival;
		
			strcpy(jobBuffer[k].name, jobBuffer[minJ].name);
			jobBuffer[k].burst = jobBuffer[minJ].burst;
			jobBuffer[k].priority = jobBuffer[minJ].priority;
			jobBuffer[k].arrival = jobBuffer[minJ].arrival;

			strcpy(jobBuffer[minJ].name, tempJob.name);
			jobBuffer[minJ].burst = tempJob.burst;
			jobBuffer[minJ].priority = tempJob.priority;
			jobBuffer[minJ].arrival = tempJob.arrival;
		}

		k++;
	}
	return 0;
}
int cmd_sjf(int nargs, char **args) {
	policy = 1;
	struct job_def tempJob;
	int j;
	int minJ;
	int i;
	int k = buf_head;
	int l;
	for (l = 0; l < count; l++) {
		if (k == BUFF_SIZE) {
			k = 0;
		}
		j = k;
		int minBurst = jobBuffer[j].burst;
		for (i = l; i < count; i++) {
			int temp = jobBuffer[j].burst;
			if (temp < minBurst) {
				minBurst = temp;
				minJ = j;
			}
			j++;
		}
		if (minJ != k) {
			printf("made it here");
			strcpy(tempJob.name, jobBuffer[k].name);
			tempJob.burst = jobBuffer[k].burst;
			tempJob.priority = jobBuffer[k].priority;
			tempJob.arrival = jobBuffer[k].arrival;
		
			strcpy(jobBuffer[k].name, jobBuffer[minJ].name);
			jobBuffer[k].burst = jobBuffer[minJ].burst;
			jobBuffer[k].priority = jobBuffer[minJ].priority;
			jobBuffer[k].arrival = jobBuffer[minJ].arrival;

			strcpy(jobBuffer[minJ].name, tempJob.name);
			jobBuffer[minJ].burst = tempJob.burst;
			jobBuffer[minJ].priority = tempJob.priority;
			jobBuffer[minJ].arrival = tempJob.arrival;
		}

		k++;
	}
	return 0;
}

int cmd_priority(int nargs, char **args) {
	policy = 2;
	struct job_def tempJob;
	int j;
	int minJ;
	int i;
	int k = buf_tail;
	int l;
	for (l = 0; l < count; l++) {
		if (k == BUFF_SIZE) {
			k = 0;
		}
		printf("\nOne\n");
		j = k;
		int limit = count - l;
		int minPriority = jobBuffer[j].priority;
		for (i = l; i < limit; i++) {
			printf("\nTwo\n");
			int temp = jobBuffer[j].priority;
			if (temp < minPriority) {
				printf("\nThree\n");
				minPriority = temp;
				minJ = j;
			}
			j++;
		}
		if (minJ != k) {
			strcpy(tempJob.name, jobBuffer[k].name);
			tempJob.burst = jobBuffer[k].burst;
			tempJob.priority = jobBuffer[k].priority;
			tempJob.arrival = jobBuffer[k].arrival;
		
			strcpy(jobBuffer[k].name, jobBuffer[minJ].name);
			jobBuffer[k].burst = jobBuffer[minJ].burst;
			jobBuffer[k].priority = jobBuffer[minJ].priority;
			jobBuffer[k].arrival = jobBuffer[minJ].arrival;

			strcpy(jobBuffer[minJ].name, tempJob.name);
			jobBuffer[minJ].burst = tempJob.burst;
			jobBuffer[minJ].priority = tempJob.priority;
			jobBuffer[minJ].arrival = tempJob.arrival;
		}
		k++;
	}
	return 0;
}


/*
 *  * The quit command.
 *   */
int cmd_quit(int nargs, char **args) {
	printf("Please display performance information before exiting AUbatch!\n");
        exit(0);
}
/*
 *  * The list command.
 *  */
int cmd_list(int nargs, char **args) {
	printf("\nTotal Number of jobs in the queue: ");
	printf("%u\n", count);
	printf("Scheduling Policy: ");
	if (policy == 0) {
		printf("FCFS.\n");
	}
	else if(policy == 1) {
		printf("SJF.\n");
	}
	else if(policy == 2) {
		printf("Priority.\n");
	}
	printf("Name	CPU_Time	Priority	Arrival_time	Progress\n");
	int i = buf_tail - 1;
	int j;
	struct tm *ptm;
	time_t time;
	int run = 1;
	int jobs = count + 1;
	for (j = 0; j < jobs; j++) {
		if (i == BUFF_SIZE) {
			i = 0;
		}

		//printf("\nstart loop\n");
		printf("%s\t",jobBuffer[i].name);
		printf("%u\t\t",jobBuffer[i].burst);
		printf("%u\t\t",jobBuffer[i].priority);
		time = jobBuffer[i].arrival;
		ptm = localtime( &time );
		printf("%02d:%02d:%02d\t", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
		if (run) {
			printf("RUN");
			run = 0;
		}
		printf("\n");
		i++;
	}
	return 0;
}


/*
 *  * Display menu information
 *   */
void showmenu(const char *name, const char *x[])
{
	int ct, half, i;
        printf("\n");
        printf("%s\n", name);
        for (i=ct=0; x[i]; i++) {
                ct++;
	}
	half = (ct+1)/2;
        for (i=0; i<half; i++) {
                printf("    %-36s", x[i]);
                if (i+half < ct) {
                        printf("%s", x[i+half]);
                }
		printf("\n");														       
       	}
        printf("\n");
}

static const char *helpmenu[] = {
        "[run] <job> <time> <priority>       ",
	"[quit] Exit AUbatch                 ",
        "[help] Print help menu              ",
        /* Please add more menu options below */
        NULL
};

int cmd_helpmenu(int n, char **a)
{
        (void)n;
        (void)a;
        showmenu("AUbatch help menu", helpmenu);
        return 0;
}

//char *cmd_buffer[BUFF_SIZE];

int main() {
	//initialize var
	count = 0;
	buf_head = 0;
	buf_tail = 0;
		
	printf("\n Welcome to Theo's bath job scheduler version 0.12\n enter help for a list of commands.");
	pthread_t commandLineThread, executorThread;
	
	//concurrent threads
	int iret1, iret2;
	iret1 = pthread_create(&commandLineThread, NULL, commandLine, NULL);
	iret2 = pthread_create(&executorThread, NULL, executor, NULL);
	
	//check error
	if ((iret1 != 0) || (iret2 != 0)) {
		printf("cannot create threads");
		exit(1);
	}

	//intialize the lock and condition variables
	pthread_mutex_init(&cmd_queue_lock, NULL);
	pthread_cond_init(&cmd_buf_not_full, NULL);
	pthread_cond_init(&cmd_buf_not_empty, NULL);

	//join thread
	pthread_join(commandLineThread, NULL);
	pthread_join(executorThread, NULL);
	
	//exit
	exit(0);
}


//cmd table
static struct {
	        const char *name;
		int (*func)(int nargs, char **args);
} cmdtable[] = {
	        /* commands: single command must end with \n */
	        { "?\n",        cmd_helpmenu },
		{ "h\n",        cmd_helpmenu },
		{ "help\n",     cmd_helpmenu },
		{ "r",          cmd_run },
	       	{ "run",        cmd_run },
		{ "q\n",        cmd_quit },
		{ "quit\n",     cmd_quit },
		/* Please add more operations below. */
		{ "l\n",	cmd_list },
		{ "list\n",	cmd_list },
		{ "sjf\n",	cmd_sjf },
		{ "fcfs\n",	cmd_fcfs },
		{ "priority\n",	cmd_priority },
		{NULL, NULL}
};

int dispatch(char *cmd) {
	time_t beforesecs, afersecs, secs;
	u_int32_t berforensec, afternsecs, nsecs;
	char *args[MAXMENUARGS];
	int nargs=0;
	char *word;
	char *context;
	int i, result;
	for (word = strtok_r(cmd, " ", &context);
		word != NULL;
		word = strtok_r(NULL, " ", &context)) {
		if (nargs >= MAXMENUARGS) {
			printf("command line has too many words\n");
			return E2BIG;
		}
		args[nargs++] = word;
	}
		if (nargs==0) {
			return 0;
		}
	for(i = 0; cmdtable[i].name; i++) {
		if(*cmdtable[i].name && !strcmp(args[0], cmdtable[i].name)) {
			assert(cmdtable[i].func != NULL);
			//call function here maybe?i
			result = cmdtable[i].func(nargs, args);
			return result;
		}
	}
	printf("%s: Command not found\n", args[0]);
	return EINVAL;
}




void *commandLine() {

	u_int i;
	char num_str[8];
	size_t command_size[64];
	char *input;
	char inputA[100];

	for (i = 0; i < NUM_OF_CMD; i++) {
		pthread_mutex_lock(&cmd_queue_lock);

		while (count == BUFF_SIZE) {
			pthread_cond_wait(&cmd_buf_not_full, &cmd_queue_lock);
		}
		pthread_mutex_unlock(&cmd_queue_lock);
		//prompt for input
		printf("\n>");
		getline(&input, &command_size, stdin);
		dispatch(input);
	}

}

void *executor() {
	u_int i;

	char arg1[50], arg2[50];
	for (i = 0; i < NUM_OF_CMD; i++) {
		pthread_mutex_lock(&cmd_queue_lock);

	while (count == 0) {
		pthread_cond_wait(&cmd_buf_not_empty, &cmd_queue_lock);
	}
	count--;
	sprintf(arg1,"%s",jobBuffer[buf_tail].name);
	sprintf(arg2,"%u",jobBuffer[buf_tail].burst);
	//system(jobBuffer[buf_tail]); //change later to execv
	//free(jobBuffer[buf_tail]); // change this too
	
	pid_t run = fork();
	
	if(run == 0) {
		execv("./process",(char*[]){"./process",arg1,arg2,NULL});
	}
	buf_tail++;
	if (buf_tail == BUFF_SIZE) {
		buf_tail = 0;
	}
	pthread_cond_signal(&cmd_buf_not_full);
	pthread_mutex_unlock(&cmd_queue_lock);
	wait();
	} //end for loop
}
