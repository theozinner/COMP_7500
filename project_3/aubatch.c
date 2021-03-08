#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define BUFF_SIZE 15
#define NUM_OF_CMD 5
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

char *cmd_buffer[BUFF_SIZE];

int main() {
	//initialize var
	count = 0;
	buf_head = 0;
	buf_tail = 0;
		
	printf("\m Welcome to Theo's bath job scheduler version 0.1\n enter help for a list of commands.");
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
	pthread_join(commandLineThread);
	pthread_join(dispatcherThread);
	
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
		{ "r",          cmd_run } ,
	       	{ "run",        cmd_run },
		{ "q\n",        cmd_quit },
		{ "quit\n",     cmd_quit },
		/* Please add more operations below. */
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
		}	

}




void *commandLine() {

	u_int i;
	char num_str[8]
	size_t command_size[64];
	char *input;
	char inputA[100];

	for (i = 0; i < NUM_OF_CMD; i++) {
		pthread_mutex_lock(&cmd_queue_lock);

		while (count == CMD_BUF_SIZE) {
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
	for (i = 0; i < NUM_OF_CMD; i++) {
		pthread_mutex_lock(&cmd_queue_lock);

	while (count == 0) {
		pthread_cond_wait(&cmd_buf_not_empty, &cmd_queue_lock);
	}
	count--;
	
	system(cmd_buffer[buf_tail]); //change later to execv
	free(cmd_buffer[buf_tail]); // change this too
	
	buf_tail++;
	if (buf_tail == BUFF_SIZE) {
		buf_tail = 0;
	}
	pthread_cond_signal(&cmd_buf_notfull);
	pthread_mutex_unlock(&cmd_queue_lock);
	} //end for loop
}
