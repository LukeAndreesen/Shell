#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

//-------------------- NOTES -------------------//

// replace custom function with strtok use 
// when implementing further parts, check if execvp returns anything
   // if no return, no proble,
//---------------------------------------------------// 
char **seperate_arguments(char *pinput, int *num_args);
char **seperate_commands(char *pinput, int *num_args);
char **split_args(char **args, int num_args, int arg_idx, int idx);
//void seperate_commands(char *pinput, int *num_args);
void execute_command(char **args, int num_args, int *success);
void change_directory(char **args, int num_args);
int blank_line(char *pinput);
int redirect_idx(char **args, int num_args, int *advanced);
char **modified_args(char **args, int num_args, int redirect);
void remove_leading_whitespace(char **args);
int redirect_check(char *arg);
int search_for_redirect(char **args, int num_args, int *arg_number);


void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}


int main(int argc, char *argv[]) 
{
    char cmd_buff[514];
    char *pinput;
	char error_message[30] = "An error has occurred\n";
	int success = 1;

	//Batch mode
	if (argc > 1) { //batch mode
		FILE *file;
//		char line[100];
		file = fopen(argv[1], "r");
		// First line
		pinput = fgets(cmd_buff, 514, file);
		while (pinput != NULL){
				if (strlen(pinput) > 512){
					while (1)
						if(strlen(pinput) > 512){
							myPrint(pinput);
							pinput = fgets(cmd_buff, 514, file);
						}
						else{
							myPrint(pinput);
							myPrint(error_message);
							pinput = fgets(cmd_buff, 514, file);
							break;
					}
					continue;
				}	
			
//				myPrint(pinput);
				if (blank_line(pinput)){
					pinput = fgets(cmd_buff, 512, file);
					continue;
				}
				myPrint(pinput);

				int newline_idx = strcspn(pinput, "\n");
				pinput[newline_idx] = '\0';

				int num_commands;
				char **commands = seperate_commands(pinput, &num_commands);

				int num_args;
				char **args;
				for (int i = 0; i < num_commands; i++){
					args = seperate_arguments(commands[i], &num_args);
					remove_leading_whitespace(args);
					int trailing_space_idx = strcspn(args[0], " ");
					args[0][trailing_space_idx] = '\0';
				//	int trailing_tab_idx = strcspn(args[0], "\t");
				//	args[0][trailing_tab_idx] = '\0';
					execute_command(args, num_args, &success);
					if (success == -1){
						myPrint(error_message);			
						success = 1;
					}
				}
				pinput = fgets(cmd_buff, 514, file);
			} 
		}
	else{
		while (1) {
			myPrint("myshell> ");

			pinput = fgets(cmd_buff, 514, stdin);
			if (strlen(pinput) >= 512){
                    while (1)
                        if(strlen(pinput) >= 512){
                            myPrint(pinput);
                            pinput = fgets(cmd_buff, 514, stdin);
                        }
                        else{
                            myPrint(pinput);
                            myPrint(error_message);
                            pinput = fgets(cmd_buff, 514, stdin);
                            break;
                    }
                    continue;
               }   
			if (blank_line(pinput)){
           		continue;
            }
			int newline_idx = strcspn(pinput, "\n");
			pinput[newline_idx] = '\0';
			
			int num_commands;
			char **commands = seperate_commands(pinput, &num_commands);
			
			int num_args;
			char **args;
			for (int i = 0; i < num_commands; i++){
				args = seperate_arguments(commands[i], &num_args);
				remove_leading_whitespace(args);
				int trailing_space_idx = strcspn(args[0], " ");
				args[0][trailing_space_idx] = '\0';
				int trailing_tab_idx = strcspn(args[0], "\t");
				args[0][trailing_tab_idx] = '\0';
				execute_command(args, num_args, &success);

				if (success == -1){
					myPrint(error_message);
					success = 1;
				}
					
			}
			pinput[newline_idx] = '\n';
			pinput[newline_idx + 1] = '\0';
		}
	}
}

int blank_line(char *pinput){
	if (strcmp(pinput, "\n") == 0){
    	return 1;
    }
	int pinput_idx = 0;
	char c = pinput[pinput_idx];
	while (c != '\n'){
		if ((c != '\t') && (c != ' ')){
			return 0;
		}
		pinput_idx++;
		c = pinput[pinput_idx];
	}
	return 1;
}


void remove_leading_whitespace(char **args){
	char c = args[0][0];
	int idx = 0;
	while ((c == ' ') || (c == '\t')){
		idx++;
		c = args[0][idx];
	}
	if (idx != 0){
		*args +=  idx;
	}
}
	

void change_directory(char **args, int num_args){
	int result;
	char error_message[30] = "An error has occurred\n";
	
	if (num_args > 1){
		myPrint(error_message);
	}
	else if (num_args == 0){
		//char cwd[514];
		//printf("%s\n", getenv("HOME"));
		//char *wd = getcwd(cwd, 514);
		chdir(getenv("HOME"));
	}
	else{
		if (strcmp(args[1], "..") == 0){
			result = chdir(args[1]);
			if (result == -1){
     	    	myPrint(error_message);
            }
		}
		else if (args[1][0] == '/'){
			result = chdir(args[1]);
			if (result == -1){
         		myPrint(error_message);
            }
		}
		else if (args[1][0] == '~'){
			if ((args[1][1] == ' ') || (args[1][1] == '\0')){
				chdir(getenv("HOME"));
			}
			else if (args[1][1] == '/'){
				char *home = strdup(getenv("HOME"));
				char *new_directory = (char *)malloc(sizeof(char) * (strlen(args[1] + strlen(home))) + 1);
				new_directory =  home;
				
				strcat(new_directory, (args[1] + 1));
				result = chdir(new_directory);
				if (result == -1){
					myPrint(error_message);
				}
			}
				
		}
		else{
			char cwd[514];
			char *wd = getcwd(cwd, 514);
			//char *new_directory = (char *)malloc(sizeof(char) * (strlen(args[1] + strlen(wd))) + 2);
			char *new_directory = strdup(wd);
			new_directory[strlen(new_directory)] = '/';
			strcat(new_directory, strdup(args[1]));
			result = chdir(new_directory);
			if (result == -1){
				myPrint(error_message);
			}
			free(new_directory);

		}
				
	}
	
	
}
void execute_command(char **args, int num_args, int *success){
	
//	pid_t forkret = fork();
	char cwd[514];
	char error_message[30] = "An error has occurred\n";
	int arg_number = -1;
	int red = search_for_redirect(args, num_args, &arg_number);

	//int redirect_in_arg = -1;
	int advanced = 0;

	if (strcmp(args[0], "exit") == 0){
		if (num_args > 0){//  && (strcmp(args[1], ">") == 0)){
			myPrint(error_message);
		}
		else{
//		myPrint("\n");
			exit(0);
		}
	}
	else if (strcmp(args[0], "pwd") == 0){
		if ((num_args > 0) || (redirect_idx(args, num_args, &advanced) != -1) || (arg_number != -1)){//         {//  && (strcmp(args[1], ">") == 0)){
			myPrint(error_message);
        }
        else{
			char *wd = getcwd(cwd, 514);
			int end_idx = strcspn(wd, "\0");
       		wd[end_idx] = '\n';
			wd[end_idx + 1] = '\0';
			myPrint(wd);
		}
	}
	else if (strcmp(args[0], "cd") == 0){
	//	if ((num_args > 0)  && (strcmp(args[1], ">") == 0)){
		if (redirect_idx(args, num_args, &advanced) != -1){
            myPrint(error_message);
        }
		else{
			change_directory(args, num_args);
		}
	}
	else if{

		pid_t forkret = fork();
		if (forkret == 0){
			advanced = 0;
				
			int redirect = redirect_idx(args, num_args, &advanced);
			if (advanced != 0){
				myPrint(error_message);
				exit(0);
			}
			else if ((redirect == -1) && (red != -1)){
				if (red == 0){
					//printf("%s\n", args[1]);

					if (num_args - arg_number + 1 > 1){
						myPrint(error_message);
					}
					else{
						char **split = split_args(args, num_args, arg_number, red);
						int output = open(split[arg_number], O_RDWR);
						if (output >= 0){
							myPrint(error_message);
						}
						else{
							mode_t mode = S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR;
							output = open(split[arg_number], O_RDWR | O_CREAT, mode);
							dup2(output, STDOUT_FILENO);
						//	split[arg_idx] =
							if (execvp(split[0], split) != 0){
								myPrint(error_message);
							}	
						}
						free(split);
					}	
				}
				else if (num_args - arg_number + 2 > 1){
					myPrint(error_message);
					
				}
				else{

					char **split = split_args(args, num_args, arg_number, red);
					int output = open(split[arg_number+1], O_RDWR);
					if (output >= 0){
						myPrint(error_message);
					}
					else if ((strcmp(split[0], "cd") == 0) || (strcmp(split[0], "pwd") == 0) || (strcmp(split[0], "exit") == 0)){
						myPrint(error_message);
					}
					
					else{
						mode_t mode = S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR;
						output = open(split[arg_number+1], O_RDWR | O_CREAT, mode);
						dup2(output, STDOUT_FILENO);
						if (execvp(split[0], split) != 0){
							myPrint(error_message);
						}
					}
					free(split);
				}
				exit(0);
			}
			//if ((redirect_in_arg != -1) && (num_args > 0)){
			//	mode_t mode = S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR; // | S_IRGRP | S_IROTH;
			//	int output = open(args[1], O_RDWR | O_CREAT, mode);
			//	dup2(output, STDOUT_FILENO);
			//	args[0][redirect_in_arg] = '\0';
			//	if (execvp(args[0], modified_args(args, num_args, redirect)) != 0){
			//		myPrint(error_message);
			//	}
			//	exit(0);
		//	}
			else if (redirect == -1){ // no redirection
				if (execvp(args[0], args) != 0){
					myPrint(error_message);
				}
        //      perror("execvp");
                exit(0);
            }
			else{	
				if ((num_args <= redirect) || (num_args - redirect > 1)){
					myPrint(error_message);
				}
				else{
				
						//mode_t mode = S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR; // | S_IRGRP | S_IROTH;
						int output = open(args[redirect+1], O_RDWR);
						if (output >= 0){
							myPrint(error_message);
						}
						else{
							mode_t mode = S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR;
							output = open(args[redirect+1], O_RDWR | O_CREAT, mode);
							dup2(output, STDOUT_FILENO);
							if (execvp(args[0], modified_args(args, num_args, redirect)) != 0){
								myPrint(error_message);
						}
						}
						//int output = open(args[redirect+1], O_RDWR | O_CREAT, mode);
					//	dup2(output, STDOUT_FILENO);
						
					//	if (execvp(args[0], modified_args(args, num_args, redirect)) != 0){
					//		myPrint(error_message);
					//	}
					}
				}
				exit(0);
			}
		}     
		else {
			wait(NULL);
		}
	
}


char **split_args(char **args, int num_args, int arg_idx, int idx){
	char **result = (char **)malloc(sizeof(char **) * (num_args + 1));
	for (int i = 0; i < arg_idx; i++){
		result[i] = strdup(args[i]);
	}
	if (idx == 0){
		result[arg_idx] = strdup(args[arg_idx] + idx +1 );
		result[arg_idx +1 ] = NULL;
		return result;
	}
	result[arg_idx] = strdup(args[arg_idx]);
	result[arg_idx + 1] = strdup(args[arg_idx] + idx +1 );
	result[arg_idx][idx] = '\0';
	//if result[]
	return result;
}


char **modified_args(char **args, int num_args, int redirect){
	char **result = (char **)malloc(sizeof(char **) * (num_args - redirect + 1));
	for (int i = 0; i < redirect; i++){
		result[i] = args[i];
	}
	result[redirect] = '\0';
	return result;
}

char  **seperate_arguments(char *pinput, int *num_args){
	char **result = (char **)malloc(sizeof(char *) * (strlen(pinput) ));
	char *tok = strtok(pinput, " ");
	//result[0] = strdup(tok);

	int	result_idx = 0;
	while (tok != NULL){
		result[result_idx] = strdup(tok);
		tok = strtok(NULL, " ");
		result_idx++;
	}
	result[result_idx - 1][strcspn(result[result_idx - 1], "\n")] = '\0'; 
	result[result_idx] = '\0';
	*num_args = result_idx - 1;
	return result;
}

int redirect_check(char *arg){
	char c = arg[0];
	int idx = 0;
	while (c != '\0'){
		if (c == '>'){
			break;
		}
		idx++;
		c = arg[idx];
	}
	
	
	return -1;
}
int redirect_idx(char **args, int num_args, int *advanced){
	int idx = 1;
	char *arg = args[idx];
	while (idx <= num_args){
		if (strcmp(arg, ">+") == 0){
			*advanced = 1;
			return idx;
		}
		if (strcmp(arg, ">") == 0){
			return idx;
		}
		
		idx++;
		arg = args[idx];
	}
	return -1;
}
		
char **seperate_commands(char *pinput, int *num_commands){
//void seperate_commands(char *pinput, int *num_commands){
	char **result = (char **)malloc(sizeof(char *) * (strlen(pinput)));
	char *tok = strtok(pinput, ";");
//	printf("command 0:");
//	printf("%s", tok);	
	int result_idx = 0;
	while (tok != NULL){
        result[result_idx] = strdup(tok);
        tok = strtok(NULL, ";");
        result_idx++;
    }
    result[result_idx - 1][strcspn(result[result_idx - 1], "\n")] = '\0';
    result[result_idx] = '\0';
    *num_commands = result_idx;
    return result;
}	

int search_for_redirect(char **args, int num_args, int *arg_number){
	for (int i = 0; i < num_args + 1; i++){
		char c = args[i][0];
		int idx = 0;
		while (c != '\0'){
			if (c == '>'){
				*arg_number = i;
				return idx;
			}
			idx++;
			c = args[i][idx];
		}
	}
	return -1;
	
}

void transfer_data(int original, int ouput){
	
}
//int *arg_idx(char *pinput){

void tst(char * pinput){
	int *result = (int *)malloc(sizeof(int)); 
	//int space_seen = 0;
	int result_idx = 0;
	
	char c = pinput[0];
	int pinput_idx = 0;
	while(c != '\0'){
		if (c == ' '){
			int space_start = pinput_idx;
			int inner_count = 0;
			while (pinput[pinput_idx + 1] == ' '){
				pinput_idx++;
				inner_count++;
			}
			if (inner_count == 0) pinput_idx++; //ensure we iterate even if only one space
			result[result_idx] = space_start + inner_count + 1;
			printf("%d\n", result[result_idx]);
			result_idx++;
			c = pinput[pinput_idx];
//			if (space_seen == 0){
//				space_seen++;
//			}
//			else {
//				result[result_idx] = pinput_idx +1;
//				space_seen = 0;:/				result_idx++;
//				printf("%d\n", result[result_idx-1]);
//			}
//		}
		}
		else{
			pinput_idx++;
			c = pinput[pinput_idx];
		}
	}
//	return result;
}
