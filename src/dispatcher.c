#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dispatcher.h"
#include "shell_builtins.h"
#include "parser.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <unistd.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define bool _Bool

int *outputSetup(struct command *pipeline);
int inputSetup(struct command *pipeline, int *previousPipe, bool isFirst);
int recur_fun(struct command *pipeline, int *previousPipe, bool isFirst);
/**
 * dispatch_external_command() - run a pipeline of commands
 *
 * @pipeline:   A "struct command" pointer representing one or more
 *              commands chained together in a pipeline.  See the
 *              documentation in parser.h for the layout of this data
 *              structure.  It is also recommended that you use the
 *              "parseview" demo program included in this project to
 *              observe the layout of this structure for a variety of
 *              inputs.
 *
 * Note: this function should not return until all commands in the
 * pipeline have completed their execution.
 *
 * Return: The return status of the last command executed in the
 * pipeline.
 */
static int dispatch_external_command(struct command *pipeline)
{
	/*
	 * Note: this is where you'll start implementing the project.
	 *
	 * It's the only function with a "TODO".  However, if you try
	 * and squeeze your entire external command logic into a
	 * single routine with no helper functions, you'll quickly
	 * find your code becomes sloppy and unmaintainable.
	 *
	 * It's up to *you* to structure your software cleanly.  Write
	 * plenty of helper functions, and even start making yourself
	 * new files if you need.
	 *
	 * For D1: you only need to support running a single command
	 * (not a chain of commands in a pipeline), with no input or
	 * output files (output to stdout only).  In other words, you
	 * may live with the assumption that the "input_file" field in
	 * the pipeline struct you are given is NULL, and that
	 * "output_type" will always be COMMAND_OUTPUT_STDOUT.
	 *
	 * For D2: you'll extend this function to support input and
	 * output files, as well as pipeline functionality.
	 *
	 * Good luck!
	 */
	//split up pipeline data //need to put all of this inside a recursive function basically then it will recall itself if 
	//output_type is equal to pipe. 

	

	int var = recur_fun(pipeline, NULL, true);

	// int fd = outputSetup(pipeline);
	// int id = inputSetup(pipeline);

	// pid_t pid = fork();//
	// if(pid == 0){
	// 	dup2(id, STDIN_FILENO);
	// 	dup2(fd, STDOUT_FILENO);
	// 	int status = execvp(pipeline->argv[0],pipeline->argv);
	// 	if(status == -1){
	// 		fprintf(stderr, "Not a real command\n");			
	// 		exit(1);
	// 	}
	// }
	// waitpid(pid, &var, 0);
	
	// if(pipeline->output_type == COMMAND_OUTPUT_FILE_TRUNCATE || pipeline->output_type == COMMAND_OUTPUT_FILE_APPEND){
	// 	close(fd);
	// }
	// if(pipeline->input_filename != NULL){
	// 	close(id);
	// } 

	//fprintf(stdree, getpid(pid));

	//fprintf(stderr, "TODO: handle external commands\n");
	return var;
}

//NEED TO CHECK WHILE DOING


int recur_fun(struct command *pipeline, int *previousPipe, bool isFirst) {
	int	*fd = outputSetup(pipeline);
	int id = inputSetup(pipeline, previousPipe, isFirst);

	int var;
	pid_t pid = fork();//
	if(pid == 0){
		if(pipeline->output_type != COMMAND_OUTPUT_PIPE){
			dup2(id, STDIN_FILENO);
			close(id);
			dup2(fd[0], STDOUT_FILENO);
			close(fd[0]);
		}else{
			dup2(id, STDIN_FILENO);
			close(id);
			dup2(fd[1], STDOUT_FILENO);
		}
		int status = execvp(pipeline->argv[0],pipeline->argv);
		if(status == -1){
			fprintf(stderr, "Not a real command\n");			
			exit(1);
		}
	}
	waitpid(pid, &var, 0);
	if(pipeline->output_type == COMMAND_OUTPUT_PIPE){
		close(fd[1]);
	}
	if(!isFirst){
		close(id);
	}
	if(WIFEXITED(var) == 0){
		if(WEXITSTATUS(var) != 0){
			return -1;
		}
	}
	
	if(pipeline->output_type == COMMAND_OUTPUT_FILE_TRUNCATE || pipeline->output_type == COMMAND_OUTPUT_FILE_APPEND){
		close(*fd);
	}
	if(pipeline->input_filename != NULL){
		close(id);
	} 

	if(pipeline->output_type == COMMAND_OUTPUT_PIPE){
		if(isFirst == true){
			isFirst = false;
		}
		//close(fd[1]);
		var = recur_fun(pipeline->pipe_to, fd, isFirst);
		close(*fd);//maybe thats right idk tbh
	}
	
	return var;

}

int *outputSetup(struct command *pipeline) {
	static int fd[2];
	if(pipeline->output_type == COMMAND_OUTPUT_FILE_TRUNCATE){
		fd[0] = open(pipeline->output_filename, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU); //S_IWUSR|S_IXUSR);//need to figure out exactly how works but think this is right
		//if(fd == NULL) ///need to add a check to see if open failed

		//dup2(fd, STDOUT_FILENO);	
	}else if(pipeline->output_type == COMMAND_OUTPUT_FILE_APPEND){
		fd[0] = open(pipeline->output_filename, O_CREAT|O_WRONLY|O_APPEND, S_IRWXU);// S_IWUSR|S_IXUSR);

	}else if(pipeline->output_type == COMMAND_OUTPUT_PIPE){
		if(pipe(fd) < 0){
			fprintf(stderr, "pipe error");

		}
		//close(fd[0]); bad
	
	}else{
		fd[0] = STDOUT_FILENO;
	}
	
	return fd;

}

int inputSetup(struct command *pipeline, int* previousPipe, bool isFirst) {
	static int id;
	if(pipeline->input_filename != NULL){
		id = open(pipeline->input_filename, O_RDONLY, S_IWUSR|S_IXUSR);
	}else if(!isFirst){
		id = previousPipe[0]; //dont know if this works tbh gonna need to ask 
		//id[1] = previousPipe[1]; //maybe ask if we even need to set both because we tenically have closed
								 //the write end of the pipe.
	}else{
		id = STDIN_FILENO; 
	}
	return id;
}



// void ioRedirect(int output, int input){//will do the dup2's for the child where you excute it
// 	dup2(input, output);
// }

/**
 * dispatch_parsed_command() - run a command after it has been parsed
 *
 * @cmd:                The parsed command.
 * @last_rv:            The return code of the previously executed
 *                      command.
 * @shell_should_exit:  Output parameter which is set to true when the
 *                      shell is intended to exit.
 *
 * Return: the return status of the command.
 */
static int dispatch_parsed_command(struct command *cmd, int last_rv,
				   bool *shell_should_exit)
{
	/* First, try to see if it's a builtin. */
	for (size_t i = 0; builtin_commands[i].name; i++) {
		if (!strcmp(builtin_commands[i].name, cmd->argv[0])) {
			/* We found a match!  Run it. */
			return builtin_commands[i].handler(
				(const char *const *)cmd->argv, last_rv,
				shell_should_exit);
		}
	}

	/* Otherwise, it's an external command. */
	return dispatch_external_command(cmd);
}

int shell_command_dispatcher(const char *input, int last_rv,
			     bool *shell_should_exit)
{
	int rv;
	struct command *parse_result;
	enum parse_error parse_error = parse_input(input, &parse_result);

	if (parse_error) {
		fprintf(stderr, "Input parse error: %s\n",
			parse_error_str[parse_error]);
		return -1;
	}

	/* Empty line */
	if (!parse_result)
		return last_rv;

	rv = dispatch_parsed_command(parse_result, last_rv, shell_should_exit);
	free_parse_result(parse_result);
	return rv;
}
