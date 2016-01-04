// Shane Fenske (11/16/15)
// process.c
// Backend of a shell.
// Handles the execution of the CMD argument that is created by mainBsh.c
// Each piece of grammar has a function and the original CMD is passed to 
// command which then passes it down through the various types of grammar 
// according to the following grammar: 
// <stage>    = <simple> / (<command>)
// <pipeline> = <stage> / <pipeline> | <stage>
// <and-or>   = <pipeline> / <and-or> && <pipeline> / <and-or> || <pipeline>
// <sequence> = <and-or> / <sequence> ; <and-or> / <sequence> & <and-or>
// <command>  = <sequence> / <sequence> ; / <sequence> &

#include "process.h"

// Executes the CMD struct given from mainBsh.c -- cmdList. 
// Returns the status of executing cmdList and sets envvar "?" to this status.
int process (CMD *cmdList) {
    int st, pid;

    // reap all zombies
    while ( (pid = waitpid(-1, &st, WNOHANG)) > 0) 
        fprintf(stderr,"Completed: %d (%d)\n", pid, st);

    int result = command(cmdList);

    setStatus(result); // set env variable "?" to result

    return result;
}

// Execution/passing on of CMD cmd of type command according to grammar.
// Returns status of this execution.
int command(CMD *cmd) {
    int status;
    setVars(cmd);  // set environment variables

    // Handle ; specially else pass everything on to sequence -- even &
    if(cmd->type == SEP_END) {
        if(cmd->right == NULL)
            status = sequence(cmd->left);
        else
            status = sequence(cmd);
    }   
    else {
        status = sequence(cmd);
    }
    return status;
}

// Execution/passing on of CMD cmd of type sequence according to grammar.
// Returns status of this execution.
int sequence(CMD *cmd){
    int status;
    setVars(cmd);  // set environment variables

    // Send each branch of ; along according to grammar, 
    // handle & using subprocess, and pass any other cmd type on to and_or()
    if(cmd->type == SEP_END) {
        status = sequence(cmd->left);
        status = and_or(cmd->right);
    } 
    else if(cmd->type == SEP_BG) {
        int pid;
        pid = fork();
        // child process
        if (pid == 0) {   
            backgroundHelp(cmd,0);
            status = 0;
            _exit(status); 
        } else if (pid < 0) 
            perror("forking error occurred");       
        
        //parent process
        fprintf(stderr, "Backgrounded: %d\n", pid); 
        status = backgroundHelp(cmd,1);

        //execute right tree normally if it is not NULL
        if(cmd->right != NULL)
            status = and_or(cmd->right);
    }
    else {
        status = and_or(cmd);
    }
    
    return status;
}

// Executes cmds of type SEP_BG. Handles logic of how to execute based on  
// left branch type and if call came from parent or child based on parent flag
int backgroundHelp(CMD *cmd, int parent) {
        //status is zero for anything that is not foregrounded
        int status = 0;
        if(!parent && cmd->left->type == SEP_END) {
            // run cmd->left->right in background
            and_or(cmd->left->right); 
        }
        else if(!parent && cmd->left->type == SEP_BG) {
            // run cmd->left->right in background
            and_or(cmd->left->right);
        }
        else if(parent && cmd->left->type == SEP_BG) {
            // cmd->l->r will be handled elsewhere->NULL. Foreground left branch
            cmd->left->right = NULL;
            status = sequence(cmd->left);
        }
        else if(!parent && cmd->left->type != SEP_END) {
            // can run whole left branch in background if not SEP_END OR SEP_BG
            sequence(cmd->left);       
        }
        else if(parent && cmd->left->type == SEP_END) {
            // run cmd->left->left in foreground
            status = sequence(cmd->left->left);
        }  

        return status; 
}

// Execution/passing on of CMD cmd of type and-or according to grammar.
// && causes cmd following to be skipped if the current cmd exits with status!=0
// || causes cmd following to be skipped if the current cmd exits with 0 status
// Returns status of this execution.
int and_or(CMD *cmd) {
    setVars(cmd);  // set environment variables
    int status = 0;
    if(cmd->type == SEP_OR) {
        // only evaluate right side if left fails
        if((status = and_or(cmd->left)) != 0) {
            status = pipeLine(cmd->right);
        }
    } 
    else if(cmd->type == SEP_AND) {
        // only evaluate right side if left succeeds
        if((status = and_or(cmd->left)) == 0) {
            status = pipeLine(cmd->right);
        }
    }
    else {
        status = pipeLine(cmd);
    }    

    return status;
}

// Execution or passing on of CMD cmd of type pipeline according to grammar.
// Execute left side in 1 subshell right in another. Returns status of execution
int pipeLine(CMD *cmd) {
    setVars(cmd);  // set environment variables
    int status,status2;
    if(cmd->type == PIPE) {
        int fd[2], pid, pid2;
        if(pipe(fd) < 0)
            perror("error in call to pipe()");
        pid = fork();
        // child process -- Bsh'
        if (pid == 0) {
            close(fd[0]);
            dup2(fd[1],1);
            close(fd[1]);
            status = pipeLine(cmd->left);
            _exit(status);
        } 
        else if (pid < 0) {
            perror("pipeline first fork error occurred");
        } 
        else {
            // first fork parent process -- Bsh
            close(fd[1]);
            pid2 = fork();
            if (pid2 == 0) {
            // second fork child -- Bsh''
                dup2(fd[0],0);
                close(fd[0]);
                status = doStage(cmd->right);
                _exit(status);
            } 
            else if (pid2 < 0) {
                perror("pipline second fork error occurred");
            } 
            else {
                // second fork parent process -- Bsh
                close(fd[0]);
                signal(SIGINT,SIG_IGN); // Ignore signal interrupts 
                waitpid(pid2,&status2,WUNTRACED);  
                waitpid(pid,&status,WUNTRACED);
                signal(SIGINT,SIG_DFL); // Stop ignoring signal interrupts
                if(WIFEXITED(status)) 
                    status = WEXITSTATUS(status);
                else
                    status = 128+WTERMSIG(status);
                
                if(WIFEXITED(status2)) 
                    status2 = WEXITSTATUS(status2);
                else
                    status2 = 128+WTERMSIG(status2);

                // if right side was successful, set status to the left side
                if(status == 0)
                    status = status2;
            }          
        }
    } 
    else {
        status = doStage(cmd);
    }

    return status;
}

// Execution/passing on of CMD cmd of type stage according to grammar.
// Either a simple or else create a subshell and send command->left back to top 
// through command in this subshell. Returns status of this execution.
int doStage(CMD *cmd) {
    setVars(cmd);  // set environment variables
    int status;
    if(cmd->type == SIMPLE) 
        status = simple(cmd);
    else {
        // if not a SIMPLE, it is a subcommand which will be ran in a subshell
        // adapted from:http://stephen-brennan.com/2015/01/16/write-a-shell-in-c
        int pid;
        pid = fork();
        // child process
        if (pid == 0) {
            if(cmd->fromType == RED_IN 
                || cmd->toType == RED_OUT 
                || cmd->toType == RED_OUT_APP) {
                redirection(cmd);
            }            
            status = command(cmd->left);
            _exit(status);
        } else if (pid < 0) {
            perror("forking error occurred");
        } else { 
            // parent process
            signal(SIGINT,SIG_IGN); // Ignore signal interrupts 
            waitpid(pid,&status,WUNTRACED); 
            signal(SIGINT,SIG_DFL); // Stop ignoring signal interrupts                    
        }
       status =(WIFEXITED(status) ? WEXITSTATUS(status) : 128+WTERMSIG(status));        
    }
    return status;
}

// Executes CMD cmd of grammar type: simple. Returns status of this execution.
// Simples are the building blocks of all grammar types.
// adapted from http://stephen-brennan.com/2015/01/16/write-a-shell-in-c/
int simple(CMD *cmd) {
    setVars(cmd);  // set environment variables
    if(is_built_in(cmd)) {
        return built_in(cmd);
    }
    else {
        int pid, status;

        pid = fork();
        // child process
        if (pid == 0) {
            if(    cmd->fromType == RED_IN 
                || cmd->toType == RED_OUT 
                || cmd->toType == RED_OUT_APP) {
                        redirection(cmd);
                }
            execvp(cmd->argv[0], cmd->argv);
            perror("exec should not return");
            setStatus(errno);
            exit(1);
        } 
        else if (pid < 0) 
            perror("forking error occurred");
        else 
            signal(SIGINT,SIG_IGN); // Ignore signal interrupts  
            waitpid(pid,&status,WUNTRACED);
            signal(SIGINT,SIG_DFL); // Stop ignoring signal interrupts
        
        return (WIFEXITED(status) ? WEXITSTATUS(status) : 128+WTERMSIG(status));
    }
}

// Executes CMD cmd which is one of: cd, dirs, or wait.
// Returns status of this execution.
int built_in(CMD *cmd) {
    setVars(cmd);  // set environment variables
    int status = 0;
    if(!strcmp("cd",cmd->argv[0])) {
        if(cmd->argc > 2) {
            perror("Useage: cd");
            return 1;
        }
        
        // switch current directory to provided argument else switch to $HOME
        if(cmd->argv[1] == NULL) {
            if (chdir( getenv("HOME") ) != 0)  {
                perror("cd fail");
                return errno;
            }
        }
        else {
            if (chdir(cmd->argv[1]) != 0)  {
                perror("cd fail");
                return errno;
            }
        }
    }
    else if(!strcmp("dirs",cmd->argv[0])) {
        if(cmd->argc > 1) { 
            perror("Useage: dirs");
            return 1;
        }
        char* cwd;
        char buff[PATH_MAX + 1];
        // print current working directory
        if((cwd = getcwd( buff, PATH_MAX + 1 )) == NULL) {
            perror("getcwd fail");
            return errno;
        }

        printf("%s\n",cwd);
    }
    else if(!strcmp("wait",cmd->argv[0])){
        if(cmd->argc > 1) {
            perror("Useage: wait");
            return 1;
        }
        int st, pid;
        // arg 0 means we wait until all childs have finished executing
        while ((pid = waitpid(-1, &st, 0))) {
            fprintf(stderr,"Completed: %d (%d)\n", pid, st);
            if (errno == ECHILD) 
              break;
        }               
        status = 0;
    }

    return status;
}

// Returns bool = to whether CMD cmd is one of: cd, dirs, or wait.
bool is_built_in(CMD *cmd) {
    if(    !strcmp("cd",cmd->argv[0]) 
        || !strcmp("dirs",cmd->argv[0])
        || !strcmp("wait",cmd->argv[0])) 
        return true;
    return false;
}

// Switches what is used as stdin stdout according to cmd's fromType & toType
// adapted from http://www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html
int redirection(CMD *cmd) {
    int in, out, status = 0;
    // handle < stdin will now come from cmd->fromFile
    if(cmd->fromType == RED_IN) {
        if((in = open(cmd->fromFile, O_RDONLY)) < 0) 
            errorExit("open() fail",errno);
        dup2(in, 0);
        close(in);
    } 
    //handle > (RED_OUT) or >> (RED_OUT_APP)stdout will now write to cmd->toFile 
    if(cmd->toType == RED_OUT) {
        if ((out = open(cmd->toFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR 
                        | S_IRGRP | S_IWGRP | S_IWUSR)) < 0) 
            errorExit("open() fail",errno);
        dup2(out, 1);
        close(out); 
    } else if (cmd->toType == RED_OUT_APP) {
        if((out = open(cmd->toFile, O_APPEND | O_WRONLY | O_CREAT, S_IRUSR 
                        | S_IRGRP | S_IWGRP | S_IWUSR)) < 0) 
            errorExit("open() fail",errno);

        dup2(out, 1);
        close(out); 
    }
    return status;
}

// Set envvar '?' to result
void setStatus(int result) {
    char stat[15];
    memset(stat, 0, sizeof(char) * 15);
    sprintf(stat, "%d", result);
    setenv("?", stat, 1);
}

// Sets env var for each name in cmd->locVar[] to corresponding cmd->locVal[] 
void setVars(CMD *cmd) {
    int i;
    if(cmd->nLocal > 0) {
        for(i=0; i < cmd->nLocal; i++)
            setenv(cmd->locVar[i], cmd->locVal[i], 1);
    }
}

// Throw perror = to msg, and exit process with envvar '?' set to status
void errorExit(char * msg, int status) {
    perror(msg);
    setStatus(status);
    _exit(EXIT_FAILURE);
}