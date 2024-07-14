#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

pid_t fg_process = 0;
int fg_processes[MAX_NUM_TOKENS];
pid_t bg_processes[MAX_NUM_TOKENS];
int stop_fg = 0;

/* Splits the string by space and returns the array of tokens
 *
 */

void handle_sigint(int sig)
{
	for (int i = 0; i < MAX_NUM_TOKENS; i++)
	{
		if (fg_processes[i] != 0)
		{
			kill(fg_processes[i], sig);
		}
	}
	stop_fg = 1;
	// if (fg_process != 0)
	// {
	// 	kill(fg_process, sig);
	// 	fg_process = 0;
	// }
}

int check_running()
{
	for (int i = 0; i < MAX_NUM_TOKENS; i++)
	{
		if (fg_processes[i] != 0)
		{
			if (waitpid(fg_processes[i], NULL, WNOHANG) == fg_processes[i])
				fg_processes[i] = 0;
			return 0;
		}
	}
	return 1;
}

char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL;
	return tokens;
}

void freemem(char **tokens)
{
	int i;
	for (i = 0; tokens[i] != NULL; i++)
	{
		free(tokens[i]);
	}
	free(tokens);
}

int check_extra(char **tokens)
{
	if (strcmp(tokens[0], "exit") == 0)
	{
		for (int i = 0; i < MAX_NUM_TOKENS; i++)
		{
			if (bg_processes[i] != 0)
			{
				kill(bg_processes[i], SIGKILL);
			}
		}
		freemem(tokens);
		exit(0);
	}
	if (strcmp(tokens[0], "cd") == 0)
	{
		if (tokens[2] == NULL)
		{
			int i = chdir(tokens[1]);
			if (i != 0 || tokens[2] != NULL)
				printf("Shell:INCORRECT COMMAND\n");
		}
		else
			printf("Too many Arguments for cd\n");
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	char line[MAX_INPUT_SIZE];
	for (int i = 0; i < MAX_NUM_TOKENS; i++)
		bg_processes[i] = 0;
	for (int i = 0; i < MAX_NUM_TOKENS; i++)
		fg_processes[i] = 0;
	char **tokens;
	int bg_process_count = 0;
	int fg_process_count = 0;
	char *split_commands[MAX_NUM_TOKENS][MAX_NUM_TOKENS];
	int i;

	signal(SIGINT, handle_sigint);

	while (1)
	{
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		char *last_dir = strrchr(cwd, '/');
		printf("%s>>> ", last_dir + 1);
		scanf("%[^\n]", line);
		getchar();

		// printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */
		line[strlen(line)] = '\n'; // terminate with new line
		tokens = tokenize(line);
		if (tokens[0] == NULL)
		{
			freemem(tokens);
			continue;
		}
		for (int i = 0; i < MAX_NUM_TOKENS; i++)
		{
			if (bg_processes[i] != 0)
			{
				if (waitpid(bg_processes[i], NULL, WNOHANG) == bg_processes[i])
					bg_processes[i] = 0;
			}
			if (fg_processes[i] != 0)
			{
				if (waitpid(fg_processes[i], NULL, WNOHANG) == fg_processes[i])
					fg_processes[i] = 0;
			}
		}

		int c_index = 0;
		int t_index = 0;
		int series = 0;
		int parallel = 0;
		for (int i = 0; i < MAX_NUM_TOKENS && tokens[i] != NULL; i++)
		{
			if (strcmp(tokens[i], "&&") == 0)
			{
				split_commands[c_index][t_index] = NULL;
				c_index++;
				t_index = 0;
				series = 1;
			}
			if (strcmp(tokens[i], "&&&") == 0)
			{
				split_commands[c_index][t_index] = NULL;
				c_index++;
				t_index = 0;
				parallel = 1;
			}
			else
			{
				split_commands[c_index][t_index] = tokens[i];
				t_index++;
			}
		}
		split_commands[c_index][t_index] = NULL;
		split_commands[c_index + 1][0] = NULL;
		if (c_index == 0)
		{
			if (check_extra(tokens))
			{
				freemem(tokens);
				continue;
			}
			else
			{
				int numTokens;
				for (numTokens = 0; tokens[numTokens] != NULL; numTokens++)
					;
				int background = (strcmp(tokens[numTokens - 1], "&") == 0);
				if (background)
				{
					tokens[numTokens - 1] = NULL;
				}
				int status = fork();
				if (status == 0)
				{
					setpgid(0, 0);
					if (execvp(tokens[0], tokens) < 0)
					{
						perror(tokens[0]);
					}
					freemem(tokens);
					exit(EXIT_FAILURE);
				}
				else
				{
					if (!background)
					{
						while (fg_processes[fg_process_count % MAX_NUM_TOKENS] != 0)
							fg_process_count++;
						fg_processes[fg_process_count % MAX_NUM_TOKENS] = status;
						waitpid(status, NULL, 0);
						fg_processes[fg_process_count % MAX_NUM_TOKENS] = 0;
						// fg_process = status;
						// waitpid(status, NULL, 0);
						// fg_process = 0;
					}
					else
					{
						while (bg_processes[bg_process_count % MAX_NUM_TOKENS] != 0)
							bg_process_count++;
						bg_processes[bg_process_count % MAX_NUM_TOKENS] = status;
					}
				}
			}
			// Freeing the allocated memory
			freemem(tokens);
		}
		else if (series)
		{
			for (int cmd_no = 0; stop_fg != 1 && split_commands[cmd_no][0] != NULL; cmd_no++)
			{
				if (check_extra(split_commands[cmd_no]))
				{
					freemem(split_commands[cmd_no]);
					continue;
				}
				else
				{
					int status = fork();
					if (status == 0)
					{
						setpgid(0, 0);
						if (execvp(split_commands[cmd_no][0], split_commands[cmd_no]) < 0)
						{
							perror(split_commands[cmd_no][0]);
						}
						freemem(split_commands[cmd_no]);
						exit(EXIT_FAILURE);
					}
					else
					{
						while (fg_processes[fg_process_count % MAX_NUM_TOKENS] != 0)
							fg_process_count++;
						fg_processes[fg_process_count % MAX_NUM_TOKENS] = status;
						waitpid(status, NULL, 0);
						fg_processes[fg_process_count % MAX_NUM_TOKENS] = 0;
					}
				}
			}
			stop_fg = 0;
		}
		else if (parallel)
		{
			printf("Parallel\n");
			for (int cmd_no = 0; split_commands[cmd_no][0] != NULL; cmd_no++)
			{
				if (check_extra(split_commands[cmd_no]))
				{
					freemem(split_commands[cmd_no]);
					continue;
				}
				else
				{
					int status = fork();
					if (status == 0)
					{
						setpgid(0, 0);
						if (execvp(split_commands[cmd_no][0], split_commands[cmd_no]) < 0)
						{
							perror(split_commands[cmd_no][0]);
						}
						freemem(split_commands[cmd_no]);
						exit(EXIT_FAILURE);
					}
					else
					{
						while (fg_processes[fg_process_count % MAX_NUM_TOKENS] != 0)
							fg_process_count++;
						fg_processes[fg_process_count % MAX_NUM_TOKENS] = status;
					}
				}
			}
			while (1)
			{
				if (check_running())
					break;
			}
		}
	}
	return 0;
}
