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

#define MAXMENUARGS 7
#define MAXCMDLINE 64

//globals
u_int buf_head;
u_int buf_tail;
u_int count;
u_int total = 0;
u_int policy = 0;

u_int totalT = 0;
float turnAroundTime;
float cpuTime;
float waitingTime;
float turnAroundTimeT;
float cpuTimeT;
float waitingTimeT;
//flags
u_int run = 1;
u_int sched = 1;
u_int oneProcess = 0;
u_int twoProcess = 1;
//time of queue
u_int expectedWaitingTime = 0;

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
int cmd_test(int nargs, char **args);

//job properties
struct job_def {
	char name[50];
	int priority, burst, position;
	time_t arrival;
};

//job buffer
struct job_def jobBuffer[BUFF_SIZE - 1];



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
		{ "test",	cmd_test },
		{NULL, NULL}
};



/*
 *  * The run command - submit a job.
 *   */
int cmd_run(int nargs, char **args) {
	int i = 0;
	time_t rtime = time(NULL);
	if (nargs != 4) {
		printf("wrong number of args");
		return EINVAL;
	}
	char *name[50];
	sscanf(args[1], "%s", &jobBuffer[buf_head].name);
	sprintf(name, "%s", jobBuffer[buf_head].name);
	sscanf(args[3], "%u", &jobBuffer[buf_head].priority);
	sscanf(args[2], "%u", &jobBuffer[buf_head].burst);
	int burst = jobBuffer[buf_head].burst;
	expectedWaitingTime = expectedWaitingTime + burst;
	jobBuffer[buf_head].arrival = rtime;
	count = count + 1;
	buf_head = buf_head + 1;
	jobBuffer[buf_head].position = buf_head;
	pthread_cond_signal(&cmd_buf_not_empty);
	char *policySet;
	if (policy == 0) {
		cmd_fcfs(NULL, NULL);
		policySet = "FCFS.";
	}
	else if (policy == 1) {
		cmd_sjf(NULL, NULL);
		policySet = "SJF.";
	}
	else if (policy == 2) {
		cmd_priority(NULL, NULL);
		policySet = "Priority.";
	}
	if (run) {
		printf("Job %s", name);
		printf(" was submitted.\n");
		printf("Total number of jobs in the queue: %u\n", count);
		printf("Expected waiting time: %u seconds\n", expectedWaitingTime);
		printf("Scheduling Policy: %s\n", policySet);
	}
	total++;
	totalT = total;
	return 0; /* if succeed */
}

int cmd_test(int nargs, char **args) {
	if (nargs != 7)
	{
		printf("\nWrong number of arguments\n");
		return EINVAL;
	}
	totalT = 0;
	waitingTimeT = 0;
	turnAroundTimeT = 0;
	cpuTimeT = 0;
	int jobs;
	int priorityMax;
	int minTime;
	int maxTime;
	sscanf(args[3], "%u", &jobs);
	sscanf(args[4], "%u", &priorityMax);
	sscanf(args[5], "%u", &minTime);
	sscanf(args[6], "%u", &maxTime);
	int i;
	if (count != 0) {
		printf("\nWaiting for other jobs to finish...\n");
		while((count != 0) || twoProcess) {
			//do nothing
		}
	}
	for (i = 0; i < jobs; i++) {
		char arg1[50], arg2[50], arg3[50], arg4[50];
		char **newArgs[10];
		int nargs = 4;
		int priority = (rand() % priorityMax + 1);
		int cpuTime = (rand() % (maxTime - minTime + 1) + minTime);
		sprintf(arg1,"%s", "run");
		sprintf(arg2, "%s", "testProcess");
		sprintf(arg3, "%u", cpuTime);
		sprintf(arg4, "%u", priority);
		newArgs[0] = arg1;
		newArgs[1] = arg2;
		newArgs[2] = arg3;
		newArgs[3] = arg4;
		run = 0;
		cmd_run(nargs, newArgs);
		run = 1;
		//call something that prints this out
	}
	sched = 0;
	if (strcmp(args[2], "fcfs") == 0) {
		cmd_fcfs(NULL, NULL);
	}
	else if (strcmp(args[2], "sjf") == 0) {
		cmd_sjf(NULL, NULL);
	}
	else if(strcmp(args[2], "priority") == 0) {
		cmd_priority(NULL, NULL);
	}
	else {
		return EINVAL;
	}
	sched = 1;
	while((count != 0) || twoProcess) {
		//do nothing
	}
	float totalRuns = totalT;
	printf("Total number of jobs submitted: %u\n", totalT);
	float turnAround = turnAroundTimeT / totalRuns;
	float cpu = cpuTimeT / totalRuns;
	float wait = waitingTimeT / totalRuns;
	float throughput = 1 / cpu;
	printf("Average turnaround time:	%.2f seconds\n", turnAround);
	printf("Average CPU time:	%.2f seconds\n", cpu);
	printf("Average waiting time:	%.2f seconds\n", wait);
	printf("Throughput	%.2f No./second\n", throughput);



	return 0;
}


int cmd_fcfs(int nargs, char **args) {
	if (policy == 0) {
		return 0;
	}
	policy = 0;
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
		j = k;
		minJ = k;
		int limit = count - l;
		long int minArrival = jobBuffer[j].arrival;
		for (i = l; i < limit; i++) {
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
	if (sched) {
		printf("Scheduling policy is switched to FCFS. All the %u waiting jobs\nhave been rescheduled.", count);
	}

	return 0;
}
int cmd_sjf(int nargs, char **args) {
	if (policy == 1) {
		return 0;
	}
	policy = 1;
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
		j = k;
		minJ = k;
		int limit = count - l;
		int minBurst = jobBuffer[j].burst;
		for (i = l; i < limit; i++) {
			int temp = jobBuffer[j].burst;
			if (temp < minBurst) {
				minBurst = temp;
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
	if (sched) {
		printf("Scheduling policy is switched to SJF. All the %u waiting jobs\nhave been rescheduled.", count);
	}

	return 0;
}

int cmd_priority(int nargs, char **args) {
	if (policy == 2) {
		return 0;
	}
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
		j = k;
		minJ = k;
		int limit = count - l;
		int minPriority = jobBuffer[j].priority;
		for (i = l; i < limit; i++) {
			int temp = jobBuffer[j].priority;
			if (temp < minPriority) {
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
	if (sched) {
		printf("Scheduling policy is switched to Priority. All the %u waiting jobs\nhave been rescheduled.", count);
	}
	return 0;
}


/*
 *  * The quit command.
 *   */
int cmd_quit(int nargs, char **args) {
	float totalRuns = total - count;
	if(totalRuns < 1) {
		printf("Cannot show statistics as not enough processes have finished");
		exit(0);
	}
	printf("Total number of jobs submitted: %u\n", total);
	float turnAround = turnAroundTime / totalRuns;
	float cpu = cpuTime / totalRuns;
	float wait = waitingTime / totalRuns;
	float throughput = 1 / cpu;
	printf("Average turnaround time:	%.2f seconds\n", turnAround);
	printf("Average CPU time:	%.2f seconds\n", cpu);
	printf("Average waiting time:	%.2f seconds\n", wait);
	printf("Throughput	%.2f No./second\n", throughput);
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
        "run <job> <time> <priority>: submit a job named <job>,    ",
	"				 execution time is <time>, ",
	"				 priority is <priority>.   ",
	"list: display the job status.				   ",
	"fcfs: change the scheduling policy to FCFS.		   ",
	"sjf: change the scheduling policy to SJF.		   ",
	"priority: change the scheduling policy to priority.	   ",
	"test: <benchmark> <policy> <num_of_jobs> <priority_levels>",
	"<min_CPU_time> <max_CPU_time>				   ",
	"quit: exit AUbatch                 			   ",
        "help: print this menu              			   ",
        /* Please add more menu options below */
        NULL
};

int cmd_helpmenu(int n, char **a)
{
        (void)n;
        (void)a;
        //showmenu("AUbatch help menu", helpmenu);
	char *help;
	printf("run <job> <time> <priority>: submit a job named <job>,  	\n");
	printf("			     execution time is <time>,  	\n");
	printf("			     priority is <priority>.		\n");
	printf("list: display the job status.					\n");
	printf("fcfs: change the scheduling policy to FCFS.			\n");
	printf("sjf: change the scheduling policy to SJF.			\n");
	printf("priority: change the scheduling policy to priority.		\n");
	printf("test: <benchmark> <policy> <num_of_jobs> <priority_levels>	\n");
	printf("      <min_CPU_time> <max_CPU_time>				\n");
	printf("quit: exit AUbatch                 			   	\n");
        printf("help: print this menu              			   	\n");
        return 0;
}

//char *cmd_buffer[BUFF_SIZE];

int main() {
	//initialize var
	count = 0;
	buf_head = 0;
	buf_tail = 0;
		
	printf("\n Welcome to Theo's batch job scheduler version 1.0\n Type 'help' to find more about AUbatch commands.");
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
	time_t startWait = jobBuffer[buf_tail].arrival;
	int cTime = jobBuffer[buf_tail].burst;
	time_t startTime = time(NULL);
	oneProcess = 1;
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
	time_t finishTime = time(NULL);
	time_t runTime = finishTime - startTime;
	time_t waitTime = startTime - startWait;
	cpuTime = cpuTime + runTime;
	cpuTimeT = cpuTime;
	expectedWaitingTime = expectedWaitingTime - cTime;
	waitingTime = waitingTime + waitTime;
	waitingTimeT = waitingTime;
	turnAroundTime = turnAroundTime + runTime + waitTime;
	turnAroundTimeT = turnAroundTime;
	if (count == 0) {
		sleep(20);
		twoProcess = 0;
	}
	}
}
