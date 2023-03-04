#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
int enable = 0;
int rediction = 0;
int wpath = 0;
char *out;
char *read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0;

  if (getline(&line, &bufsize, stdin) == -1)
  {
    if (feof(stdin))
    {
      exit(0);
    }
    else
    {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
  }

  return line;
}
#define TOK_BUFSIZE 128
#define TOK_DELIM " \t\r\n\a"
char **split_line(char *line)
{
  int bufsize = TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens)
  {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(0);
  }

  token = strtok(line, TOK_DELIM);
  while (token != NULL)
  {
    tokens[position] = token;
    position++;

    if (position >= bufsize)
    {
      bufsize += TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens)
      {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
      }
    }

    token = strtok(NULL, TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int cd(char **args);
int path(char **args);
int w_if(char **args);
int w_exit();

char *builtin_str[] = {
    "cd",
    "path",
    "exit",
    "if"};

int num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}

int cd(char **args)
{
  if (args[1] == NULL)
  {

    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
  else
  {
    if (chdir(args[1]) != 0)
    {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
  }
  return 1;
}
char *dir[512];
int path(char **args)
{
  int j = 0;
  while (dir[j])
  {
    dir[j] = malloc(128 * sizeof(char *));
    dir[j] = NULL;
    j++;
  }
  // printf("dir0=%s\n",dir[0]);
  //  if (args[1] == NULL)
  //  {
  //    int k = 0;
  //    dir[k]=malloc(1024 * sizeof(char *));
  //    while (dir[k])
  //    {
  //      dir[k] = NULL;
  //    }
  //  }
  //  else
  //  {
  int i = 1;
  while (args[i] != NULL)
  {
    dir[i - 1] = malloc(128 * sizeof(char *));
    strcpy(dir[i - 1], args[i]);
    // printf("dir1=%s",dir[i-1]);
    i++;
  }
  dir[i - 1] = NULL;
  wpath = 1;
  // }
  return 1;
}

int w_exit()
{
  exit(0);
  return 0;
}

int launch(char **args)
{
  pid_t pid;
  // printf("wpath is:%d\n",wpath);
  char **path = malloc(128 * sizeof(char));
  if (wpath == 0)
  {
    char *a = malloc(128 * sizeof(char));
    path[0] = malloc(128 * sizeof(char *));
    // printf("err\n");
    strcpy(path[0], "/bin");
    strcpy(a, "/");
    strcat(a, args[0]);
    strcat(path[0], a);
  }
  else
  {
    int k = 0;
    while (dir[k])
    {
      path[k] = malloc(128 * sizeof(char));
      strcpy(path[k], dir[k]);
      // printf("dir[k] is:%s\n",dir[k]);
      k++;
    }
    char *a = malloc(128 * sizeof(char));
    strcpy(a, "/");
    // strcat(a, args[0]);
    int g = 0;
    while (path[g])
    {
      int h = 0;
      while (path[g][h])
      {
        h++;
      }
      if (path[g][h - 1] != '/')
      {
        strcat(a, args[0]);
        strcat(path[g], a);
      }
      else
      {
        strcat(path[g], args[0]);
      }
      g++;
    }
  }
  int i = 0;
  while (path[i] != NULL && access(path[i], X_OK) == -1)
  {
    i++;
  }
  int j = 0;
  while (path[j])
  {
    j++;
  }
  if (i == j)
  {
    // printf("path i=%s\n",path[i-1]);
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
  //  printf("redirection:%d\n",rediction);
  //  printf("args0:%s\n",args[0]);
  // printf("pathi%d:%s\n", i, path[i]);
  // int p=0;
  // while(args[p]!=NULL){
  //   printf("args%d:%s\n",p,args[p]);
  //   p++;
  // }
  // int fd = fopen(output, "w");
  int status;
  int a;
  int val;
  pid = fork();
  if (pid == 0)
  {
    if (rediction == 1)
    {
      // int fd = fopen(out, "w");
      // dup2(fd,1);
      // dup2(fd,2);
      (void)close(STDOUT_FILENO);
      open(out, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      execv(path[i], args);
      // close(fd);
    }
    else
    {
      execv(path[i], args);
    }
  }
  else if (pid < 0)
  {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
  else
  {
    do
    {
      waitpid(pid, &status, 0); // WUNTRACED can be 0

    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  if (WEXITSTATUS(status) != 0)
  {
    return WEXITSTATUS(status);
  }
  return 1;
}
int w_if(char **args)
{
  int i = 0;
  int operator= 0;
  int then = 0;
  int fi = 0;
  int fcount = 0;
  int ocount = 0;
  int ifcount = 0;
  int j = 0;
  while (args[j])
  {
    if (strcmp(args[j], "then") == 0)
    {
      ifcount++;
    }
    j++;
  }
  while (args[i])
  {
    if (strcmp(args[i], "==") == 0 || strcmp(args[i], "!=") == 0)
    {
      operator= i;
      ocount++;
    }
    if (strcmp(args[i], "then") == 0)
    {
      then = i;
    }
    if (strcmp(args[i], "fi") == 0)
    {
      fi = i;
      fcount++;
    }
    i++;
  }
  // printf("args i is!%s\n",args[i]);
  if (operator== 0 || then == 0 || fi == 0 || args[fi + 1] != NULL || fcount != ocount)
  {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(0);
  }
  if (ifcount == 1)
  {
    char **sec_arg = malloc(128 * sizeof(char *));
    char **first_arg = malloc(128 * sizeof(char *));
    // printf("2\n");
    int j = 0;
    while ((1 + j) != (operator));
    {
      first_arg[j] = malloc(128 * sizeof(char *));
      strcpy(first_arg[j], args[j + 1]);
      j++;
    }
    // printf("3\n");
    int n = 1;
    while ((then + n) != (fi))
    {
      sec_arg[n - 1] = malloc(128 * sizeof(char *));
      strcpy(sec_arg[n - 1], args[then + n]);
      n++;
    }
    // printf("4\n");
  int integer = atoi(args[operator+1]);
  // printf("integer is:%d\n",integer);
  int a = execute(first_arg);
  // printf("a is:%d\n",a);
  if(strcmp(args[operator],"==")==0&&a==integer)
  {
    execute(sec_arg);
  }
  else if(strcmp(args[operator],"!=")==0&&a!=integer)
  {
    execute(sec_arg);
  }
  }
  else if (ifcount == 2)
  {
    char **first_arg = malloc(128 * sizeof(char *));
    char **sec_arg = malloc(128 * sizeof(char *));
    char **third_arg = malloc(128 * sizeof(char *));
    first_arg[0] = malloc(128 * sizeof(char *));
    strcpy(first_arg[0], args[1]);
    sec_arg[0] = malloc(128 * sizeof(char *));
    sec_arg[1] = malloc(128 * sizeof(char *));
    strcpy(sec_arg[0], args[6]);
    strcpy(sec_arg[1], args[7]);
    third_arg[0] = malloc(128 * sizeof(char *));
    third_arg[1] = malloc(128 * sizeof(char *));
    strcpy(third_arg[0], args[11]);
    int fir_int = atoi(args[3]);
    int sec_int = atoi(args[9]);
    int a = execute(first_arg);
    if ((strcmp(args[2], "==") == 0 && a == fir_int) || (strcmp(args[2], "!=") == 0 && a != fir_int))
    {
      int b = execute(sec_arg);
      if ((strcmp(args[8], "==") == 0 && b == sec_int) || (strcmp(args[8], "!=") == 0 && b != sec_int))
      {
        execute(third_arg);
      }
    }
  }
  return 1;
}
int execute(char **args)
{
  if (args[0] == NULL)
  {
    return 1;
  }
  if (enable == 1 && (strcmp(args[0], "path") != 0 && strcmp(args[0], "exit") != 0 && strcmp(args[0], "cd") != 0 && strcmp(args[0], "if") != 0))
  {
    // wpath = 0;
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    return 1;
  }
  if (strcmp(args[0], "path") == 0 && args[1] == NULL)
  {
    enable = 1;
  }
  if (strcmp(args[0], "path") == 0)
  {
    return path(args);
  }
  if (strcmp(args[0], "exit") == 0)
  {
    if (args[1])
    {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
    return w_exit();
  }
  if (strcmp(args[0], "cd") == 0)
  {
    return cd(args);
  }
  if (strcmp(args[0], "if") == 0)
  {
    return w_if(args);
  }
  int j = 0;
  int count = 0;
  int countx, county;
  while (args[j])
  {
    int k = 0;
    while (args[j][k] != NULL)
    {
      if (args[j][k] == '>')
      {
        if (j == 0)
        {
          char error_message[30] = "An error has occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message));
          return 1;
        }
        else if (args[j][k + 1] == NULL && args[j + 1] == NULL)
        {
          char error_message[30] = "An error has occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message));
          return 1;
        }
        else if (args[j][k + 1] != NULL && args[j + 1] != NULL)
        {
          char error_message[30] = "An error has occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message));
          return 1;
        }
        else if (args[j + 2] != NULL && strcmp(args[j + 2], "fi") != 0)
        {
          char error_message[30] = "An error has occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message));
          return 1;
        }
        count++;
        countx = j;
        county = k;
      }
      // printf("k is err:%d\n",k);
      k++;
    }
    j++;
    // printf("j:%d",j);
  }
  if (count == 1)
  {
    rediction = 1;
    int i = 0;
    char *ab[512];
    // printf("count==1\n");
    if (args[countx][county + 1] == NULL)
    {
      // printf("1\n");
      out = malloc(128 * sizeof(char *));
      strcpy(out, args[countx + 1]);
      // out = args[countx + 1];
      // printf("a\n");
      while (strcmp(args[i], ">"))
      {
        ab[i] = malloc(128 * sizeof(char *));
        strcpy(ab[i], args[i]);
        i++;
      }
      // printf("count==1end\n");
    }
    else
    {
      out = malloc(128 * sizeof(char *));
      int n = 0;
      while (args[countx][county + n + 1] != NULL)
      {
        out[n] = args[countx][county + n + 1];
        n++;
      }
      while (i < countx)
      {
        ab[i] = malloc(128 * sizeof(char *));
        strcpy(ab[i], args[i]);
        i++;
      }
      const char s[2] = ">";
      char *token;
      token = strtok(args[countx], s);
      ab[countx] = malloc(128 * sizeof(char *));
      strcpy(ab[countx], token);
      ab[countx + 1] = malloc(128 * sizeof(char *));
      ab[countx + 1] = NULL;
    }
    // printf("2\n");
    return launch(ab);
  }
  else if (count >= 1)
  {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
  return launch(args);
}
int main(int argc, char **argv) // argv[0] is ./wish argv[1] is a.txt argc 1 or 2
{
  char *line;
  char **args;
  int status;
  if ((strcmp(argv[0], "./wish") == 0))
  {
    if (argc == 1)
    {
      do
      {
        rediction = 0;
        printf("wish> ");
        line = read_line();
        args = split_line(line);
        status = execute(args);

        free(line);
        free(args);
      } while (status);
    }
    else if (argc == 2)
    {
      // int i = 0;
      // int count = 0;
      // while (argv[1][i])
      // {
      //   if (argv[1][i] == '/')
      //   {
      //     count++;
      //   }
      //   i++;
      // }
      // if (count >1 )
      // {
      //   char error_message[30] = "An error has occurred\n";
      //   write(STDERR_FILENO, error_message, strlen(error_message));
      //   exit(0);
      // }
      FILE *fp = fopen(argv[1], "r");
      if (fp == NULL)
      {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
      }
      else
      {
        fseek(fp, 0, SEEK_SET);
      }
      do
      {
        rediction = 0;
        char *line = NULL;
        ssize_t bufsize = 0;
        if (getline(&line, &bufsize, fp) == -1)
        {
          if (feof(fp))
          {
            exit(0);
          }
          else
          {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
          }
        }
        args = split_line(line);
        status = execute(args);
        free(line);
        free(args);
      } while (status);
      fclose(fp);
    }
    else if (argc >= 3)
    {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
  }
  return 1;
}