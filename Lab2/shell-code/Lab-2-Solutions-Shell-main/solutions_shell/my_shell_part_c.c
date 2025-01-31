#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
 *
 */
char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));

	memset(tokens,0,sizeof(tokens)); // Initializing to NULL to check if tokens are empty

	int i, tokenIndex = 0, tokenNo = 0;

	for(i =0; i < strlen(line); i++){

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		} else {
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL ;
	return tokens;
}

void free_mem(char **tokens){

	for(int i=0;tokens[i]!=NULL;i++){
		free(tokens[i]);
	}
	free(tokens);
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;


	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line

		tokens = tokenize(line);

		if(tokens[0] && !strcmp(tokens[0],"exit") && !tokens[1]){

			// If first token is exit, and 2nd is NULL then free memory
			free_mem(tokens);
			kill(-getpid(),SIGTERM); //Kill all processes in same pgid
			exit(0);
		}

		// Reap any terminated background processes without waiting for them

		int check;

		while(check = waitpid(-1,NULL,WNOHANG)>0){

			// If any process terminated, print message
			printf("Shell : Background Process Finished\n");
		}

		// If empty, restart shell
		if(!tokens[0]){
			free_mem(tokens);

			continue;
		}

		// Checking if background argument (&) is present
		int background=-1;

		for(int i=0;i<MAX_NUM_TOKENS;i++){

			if(tokens[i]==NULL){

				break;
			}else if(!strcmp(tokens[i],"&")){

				if(tokens[i+1]==NULL){

					background=i;
				}else{

					printf("Shell : Multiple Commands after &\n");
					background=-2;
				}
				break;
			}
		}

		if(background==-2){
			// If background is not last argument, error
			free_mem(tokens);

			continue;
		}

		if(strcmp(tokens[0],"cd")){

			// Treat cd commands differently
			// Use strcmp to compare strings since normal comparision wouldn't work as "cd" and tokens[0] 
			// have different sizes

			int pid=fork();

			if(pid==0){

				// execvp is used as commands have to be looked for in Path

				if(background>0){
					tokens[background]=NULL;
				}
				execvp(tokens[0],tokens);
				// Fork copies the heap memory as well, so this is memory safe
				printf("Shell: Incorrect Command : %s\n",tokens[0]);
				exit(0);
			}else{
				// Shell waits for completion
				if(background==-1){
					// Wait for only the process that was just forked
					waitpid(pid,NULL,0);
				}
			}
		}else{

			if(background>0){

				printf("Shell: cd cannot be executed in background :\n");
				free_mem(tokens);
				continue;
			}

			if(!tokens[1] || tokens[2]){

				// If first argument after cd is empty or 2nd argument contains data, then error
				printf("Shell: Incorrect Command : %s\n",tokens[0]);
				continue;
			}else{

				int ret = chdir(tokens[1]);
				// If return value is not 0, error occurred
				if(ret){

					printf("Shell: Incorrect Command : %s\n",tokens[0]);
					continue;
				}
			}
		}

		// Freeing the allocated memory	
		free_mem(tokens);


	}
	return 0;
}
