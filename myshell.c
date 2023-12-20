#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd);
void execute_command_with_pipe(char *command1, char *command2);
void handle_sigint(int sig);
void handle_sigtstp(int sig);
void handle_sigquit(int sig);
void handle_sigchld(int sig);

sigjmp_buf jmpbuf;
pid_t foreground_job_pid = 0;
pid_t flag_job_pid = 0;
int job_id = 1;

typedef struct
{
    pid_t pid;
    int job_id;
    char command[256];
} Job;

Job jobs[256];

int main()
{
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    signal(SIGQUIT, handle_sigquit);
    signal(SIGCHLD, handle_sigchld);
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    while (1)
    {
        sigsetjmp(jmpbuf, 1);
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = 0;
        if (strncmp(command, "myshell ", 8) == 0)
        {
            char *filename = command + 8;
            filename[strcspn(filename, "\n")] = 0;

            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                continue;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, sizeof(cwd), oldpwd);
            }

            fclose(file);
        }
        else
        {
            execute_command(command, cwd, sizeof(cwd), oldpwd);
        }
    }
    return 0;
}

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd)
{
    char command_copy[256];
    strncpy(command_copy, command, sizeof(command_copy));

    char *command1 = strtok(command, "|");
    char *command2 = strtok(NULL, "|");
    if (command2 != NULL)
    {
        execute_command_with_pipe(command1, command2);
    }
    else
    {
        char *args[256];
        char *token = strtok(command, " ");
        int i = 0;
        while (token != NULL)
        {
            args[i] = token;
            i++;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        bool run_in_background = false;
        pid_t pidG;
        if (i > 0 && strcmp(args[i - 1], "&") == 0)
        {
            run_in_background = true;
            args[i - 1] = NULL;
            pidG = getpid();
        }

        if (strcmp(args[0], "clr") == 0)
        {
            system("clear");
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            if (run_in_background)
            {
                jobs[job_id - 1].pid = pidG;
                jobs[job_id - 1].job_id = job_id;
                strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                printf("[%d] %d ", job_id, pidG);
                job_id++;
            }
            char *dir = args[1];
            if (dir == NULL)
            {
                printf("%s\n", cwd);
            }
            else if (strcmp(dir, "-") == 0)
            {
                if (oldpwd[0] != '\0')
                {
                    strcpy(dir, oldpwd);
                }
                else
                {
                    printf("myshell: cd: OLDPWD not set\n");
                    return;
                }
            }
            if (chdir(dir) == 0)
            {
                strcpy(oldpwd, cwd);
                if (getcwd(cwd, cwd_size) == NULL)
                {
                    perror("getcwd() error");
                    exit(1);
                }
                setenv("PWD", cwd, 1);
            }
            else
            {
                perror("myshell");
            }
        }
        else if (strcmp(args[0], "echo") == 0)
        {
            char *input_file = NULL;
            char *output_file = NULL;
            if (run_in_background)
            {
                jobs[job_id - 1].pid = pidG;
                jobs[job_id - 1].job_id = job_id;
                strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                printf("[%d] %d ", job_id, pidG);
                job_id++;
            }
            char *arg = args[1];
            int i = 1;
            while (args[i] != NULL)
            {
                if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL)
                {
                    input_file = args[i + 1];
                    break;
                }
                else if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL)
                {
                    output_file = args[i + 1];
                    break;
                }
                i++;
            }
            if (arg[0] == '$')
            {
                char *env_var = getenv(arg + 1);
                if (env_var != NULL)
                {
                    printf("%s\n", env_var);
                }
            }
            else
            {
                if (arg[0] != '<')
                {
                    printf("%s\n", arg);
                }
            }
            if (input_file != NULL)
            {
                char line[256];
                FILE *file = fopen(input_file, "r");
                if (file == NULL)
                {
                    perror("fopen");
                    return;
                }
                while (fgets(line, sizeof(line), file) != NULL)
                {
                    printf("%s\n", line);
                }
                fclose(file);
            }
            if (output_file != NULL)
            {
                FILE *file = fopen(output_file, "w");
                if (file == NULL)
                {
                    perror("fopen");
                    return;
                }
                fprintf(file, "%s", arg);
                fclose(file);
            }
        }
        else if (strcmp(args[0], "quit") == 0)
        {
            exit(0);
        }
        else if (strcmp(args[0], "myshell") == 0)
        {
            char *filename = args[1];
            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                return;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, cwd_size, oldpwd);
            }

            fclose(file);
        }
        else
        {
            char *input_file = NULL;
            char *output_file = NULL;
            int i = 1;
            while (args[i] != NULL)
            {
                if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL)
                {
                    input_file = args[i + 1];

                    break;
                }
                else if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL)
                {
                    output_file = args[i + 1];
                    break;
                }
                i++;
            }
            pid_t pid = fork();
            if (pid < 0)
            {
                perror("fork failed");
                exit(1);
            }
            else if (pid == 0)
            {
                if (input_file != NULL)
                {
                    int in = open(input_file, O_RDONLY);
                    if (in == -1)
                    {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(in, STDIN_FILENO) == -1)
                    {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(in);
                    char *new_args[256];
                    int j = 0;
                    for (int k = 0; args[k] != NULL; k++)
                    {
                        if (strcmp(args[k], "<") != 0 && (k == 0 || strcmp(args[k - 1], "<") != 0))
                        {
                            new_args[j] = args[k];
                            j++;
                        }
                    }
                    new_args[j] = NULL;
                    execvp(args[0], new_args);
                    printf("\n");
                    perror("execvp");
                    close(in);
                    exit(EXIT_FAILURE);
                }
                if (output_file != NULL)
                {
                    int in = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (in == -1)
                    {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(in, STDIN_FILENO) == -1)
                    {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(in);
                    char *new_args[256];
                    int j = 0;
                    for (int k = 0; args[k] != NULL; k++)
                    {
                        if (strcmp(args[k], ">") != 0 && (k == 0 || strcmp(args[k - 1], ">") != 0))
                        {
                            new_args[j] = args[k];
                            j++;
                        }
                    }
                    new_args[j] = NULL;
                    execvp(args[0], new_args);
                    printf("\n");
                    perror("execvp");
                    close(in);
                    exit(EXIT_FAILURE);
                }
                else
                {
                    if (execvp(args[0], args) == -1)
                    {
                        perror("myshell");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            else
            {
                if (run_in_background)
                {
                    jobs[job_id - 1].pid = pidG;
                    jobs[job_id - 1].job_id = job_id;
                    strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                    printf("[%d] %d ", job_id, pidG);
                    job_id++;
                }
                else
                {
                    foreground_job_pid = pid;
                    waitpid(pid, NULL, 0);
                    foreground_job_pid = 0;
                }
            }
        }
    }
}

void execute_command_with_pipe(char *command1, char *command2)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return;
    }
    char oldpwd[1024] = "";
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        return;
    }
    else if (pid1 == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execute_command(command1, cwd, sizeof(cwd), oldpwd);
        exit(EXIT_SUCCESS);
    }
    waitpid(pid1, NULL, 0);
    pid2 = fork();
    if (pid2 == -1)
    {
        perror("fork");
        return;
    }
    if (pid2 == 0)
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execute_command(command2, cwd, sizeof(cwd), oldpwd);
        exit(EXIT_SUCCESS);
    }
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void handle_sigint(int sig)
{
    printf("\033[33mSIGINT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        printf("foreground_job_pid: %d\n", foreground_job_pid);
        kill(foreground_job_pid, SIGINT);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigtstp(int sig)
{
    printf("\033[33mSIGSTP received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGTSTP);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigquit(int sig)
{
    printf("\033[33mSIGQUIT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGQUIT);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigchld(int sig)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}