#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h> 
#include <sys/file.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include "/home/classes/cs323/Hwk5/parse.h"

// Executes the CMD struct given from mainBsh.c -- cmdList. 
// Returns the status of executing cmdList and sets envvar "?" to this status.
int process(CMD *cmdList);

// Execution/passing on of CMD cmd of type command according to grammar.
// Returns status of this execution.
int command(CMD *cmd);

// Execution/passing on of CMD cmd of type sequence according to grammar.
// Returns status of this execution.
int sequence(CMD *cmd);

// Execution/passing on of CMD cmd of type and-or according to grammar.
// && causes cmd following to be skipped if the current cmd exits with status!=0
// || causes cmd following to be skipped if the current cmd exits with 0 status
// Returns status of this execution.
int and_or(CMD *cmd);

// Execution or passing on of CMD cmd of type pipeline according to grammar.
// Execute left side in 1 subshell right in another. Returns status of execution
int pipeLine(CMD *cmd);

// Execution/passing on of CMD cmd of type stage according to grammar.
// Either a simple or else create a subshell and send command->left back to top 
// through command in this subshell. Returns status of this execution.
int doStage(CMD *cmd);

// Executes CMD cmd of grammar type: simple. Returns status of this execution.
// Simples are the building blocks of all grammar types.
// adapted from http://stephen-brennan.com/2015/01/16/write-a-shell-in-c/
int simple(CMD *cmd);

// Executes CMD cmd which is one of: cd, dirs, or wait.
// Returns status of this execution.
int built_in(CMD *cmd);

// Returns bool = to whether CMD cmd is one of: cd, dirs, or wait.
bool is_built_in(CMD *cmd);

// Set envvar '?' to result
void setStatus(int result);

// Switches what is used as stdin stdout according to cmd's fromType & toType
// adapted from http://www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html
int redirection(CMD *cmd);

// Sets env var for each name in cmd->locVar[] to corresponding cmd->locVal[] 
void setVars(CMD *cmd);

// Throw perror = to msg, and exit process with envvar '?' set to status
void errorExit(char * msg, int status);

// Executes cmds of type SEP_BG. Handles logic of how to execute based on  
// left branch type and if call came from parent or child based on parent flag
int backgroundHelp(CMD *cmd, int parent);