/***************************************************************************//**

  @file         CShellMain.c

  @authors      Amira Abdo, Merjema Mujić

  @date         24/04/2022


*******************************************************************************/

#include <sys/statvfs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  Function Declarations for builtin shell commands:
 */
int lsh_mv(char **args);
int lsh_date(char **args);
int lsh_rev(char **args);
int lsh_du(char **args);
int lsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "mv",
  "date",
  "rev",
  "du",
  "exit"
};


int (*builtin_func[]) (char **) = {
  &lsh_mv,
  &lsh_date,
  &lsh_rev,
  &lsh_du,
  &lsh_exit
};


int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Builtin command: move file.
   @param args List of args.  args[0] is "mv".  args[1] is the file to be moved, aargs[2] is the new file.
   @return Always returns 1, to continue executing.
 */
int lsh_mv(char **args)
{
int i, fd1, fd2;
char *file1= args[1];
char *file2= args[2];
char buf[2];
fd1=open(file1,O_RDONLY,0777);
fd2=creat(file2,0777);
while(i=read(fd1,buf,1)>0) write(fd2, buf, 1);
remove(file1);
close(fd1);
close(fd2);
}


/**
   @brief Builtin command: display date.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_date(char **args){
time_t tm;
time(&tm);
printf("%s", ctime(&tm));
  return 1;
}

/**
   @brief Builtin command: reverse the line character-wise
   @param args List of args. args[0] is 'rev', args[1] is the string to be reversed, args[2] can be "-w" if the user wants the reversed string to be printed to a predefined file
   @return Always returns 1, to continue executing.

*/
int lsh_rev(char **args){
char *str=args[1];
printf(strrev(str));
if(strcmp(args[2], "-w")==1){
FILE *fptr= fopen("/home/ListOfReversedStrings.txt", "w");
fprintf(fptr, "%s", strrev(str));
fclose(fptr);
return 1;
}}

/**
   @brief Builtin command: display disk usage
   @param args List of args. args[0] is 'du', args[1] is either undefined or '-h'.
   @return Always return 1, to continue executing.

*/


int lsh_du(char **args){
const unsigned int GB = (1024 * 1024) * 1024;
    struct statvfs buffer;
    int ret = statvfs("/dev/sda4", &buffer);
	
    if (!ret){ if(strcmp(args[1],"-h")==1) {
        const double total = (double)(buffer.f_blocks * buffer.f_frsize) / GB;
        const double available = (double)(buffer.f_bfree * buffer.f_frsize) / GB;
        const double used = total - available;
        const double usedPercentage = (double)(used / total) * (double)100;
        printf("Total: %f --> %.0f GB\n", total, total);
        printf("Available: %f --> %.0f GB\n", available, available);
        printf("Used: %f --> %.1f GB\n", used, used);
        printf("Used Percentage: %f --> %.0f GB\n", usedPercentage, usedPercentage);
    }
    else{
	const double total = (double)(buffer.f_blocks * buffer.f_frsize)/ GB;
        const double available = (double)(buffer.f_bfree * buffer.f_frsize)/ GB;
        const double used = total - available;
        const double usedPercentage = (double)(used / total) * (double)100;
        printf("Total: %f --> %.0f\n", total, total);
        printf("Available: %f --> %.0f\n", available, available);
        printf("Used: %f --> %.1f\n", used, used);
        printf("Used Percentage: %f --> %.0f\n", usedPercentage, usedPercentage);
} }
    return 1;
}

/**

   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int lsh_exit(char **args)
{
  return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void)
{
#ifdef LSH_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We received an EOF
    } else  {
      perror("lsh: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define LSH_RL_BUFSIZE 1024
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */

int main(int argc, char **argv)
{
  
  //
   printf("\033[1;33m"); //Changes color
  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}

