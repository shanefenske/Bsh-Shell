Note: requires additional fiies written by professor to run.
Bsh is a simple shell, a baby brother of the Bourne-again shell
bash, and offers a limited subset of bash's functionality (plus some extras):

- local variables

- simple command execution with zero or more arguments

- redirection of the standard input (<)

- redirection of the standard output (>, >>)

- pipelines (|) consisting of an arbitrary number of commands, each having zero
  or more arguments

- backgrounded commands;

- multiple commands per line, separated by ; or & or && or ||

- groups of commands (aka subcommands), enclosed in parentheses

- directory manipulation:

    cd directoryName
    cd                  (equivalent to "cd $HOME", where HOME
			 is an environment variable)
    dirs                (print to stdout the current working
			 directory as reported by getcwd())

- other built-in commands

    wait                Wait until all children of the shell process have died.
			The status is 0.  Note:  The bash wait command takes
			command-line arguments that specify which children to
			wait for (vs. all of them).

- reporting the status of the last simple command, pipeline, or subcommand
  executed in the foreground by setting the environment variable $? to its
  "printed" value (e.g., "0" if the value is zero).

Once the command line has been parsed, the exact semantics of Bsh are those
of bash, except for the status variable, and the notes and fine points listed
below.

The assignment is to write the process() function called from Hwk5/mainBsh.c;
thus you should use (i.e., link with)

* Hwk5/mainBsh.o as the main program (source is Hwk5/mainBsh.c)

* Hwk5/getLine.o to read command lines (interface in Hwk5/getLine.h; source
  is Hwk1/getLine.c)

* Hwk5/parse.o to tokenize each command line and parse it into a syntactically
  correct tree of CMD structures (interface in Hwk5/parse.h).

DO NOT MODIFY Hwk5/mainBsh.c or Hwk5/getLine.c or Hwk5/parse.h---the source
code for process() should be in a different file (or files).  To enforce this
the test script may delete local files with the names mainBsh.*, getLine.*, and
parse.* before trying to make your program.  Moreover, Hwk5/mainBsh.c and
Hwk5/parse.o may change without notice, but never in a manner that would change
the data structure being passed to process().

Use the submit command to turn in your log file (see Homework #1) and the
source files for Bsh (including a Makefile, but not mainBsh.*, getLine.*,
and parse.*) as assignment 5.

YOU MUST SUBMIT YOUR SOURCE FILE(S) (INCLUDING THE LOG FILE) AT THE END OF ANY
SESSION WHERE YOU HAVE WRITTEN OR DEBUGGED CODE, AND AT LEAST ONCE EVERY TWO
HOURS DURING LONGER SESSIONS.  (All submissions are retained.)


Notes
~~~~~
0. Read and understand the Appendix and Hwk5/parse.h BEFORE you start writing
   code!

1. [Matthew & Stones, Chapter 2] contains a more complete description of bash,
   including the various I/O redirection operators, command separators, and the
   use of parentheses; and "man bash" and "info bash" contain more information.
   But bear in mind that there are many features that Bsh does not implement.
   Moreover, the behavior of Bsh may not match bash in some cases, including:
   a. In bash redirection to/from a file may appear only after a subcommand.
	not before.
   b. In bash $? is a shell variable rather than an environment variable, and
	its value may differ from the status that is reported by Bsh.
   c. In bash the pipefail option is not the default.
   d. In bash the wait command also takes command-line arguments that specify
	which children to wait for (vs. all of them).
   e. In bash background commands and reaped zombies are reported differently.
   f. bash allows multiple input redirections and output redirections, with
	the last encountered taking precedence.  In Bsh the parse() function
	issues an error message instead.
   ...
   Note:  This list will expand as I learn of discrepancies.

2. Bsh uses perror() (see "man perror") to report errors from system calls.
   It may ignore error returns from close(), dup(), dup2(), setenv(), wait(),
   and waitpid() but not from chdir(), execvp(), fork(), getcwd(), open(), and
   pipe().

   Bsh also reports an error if the number of arguments to a built-in command
   is incorrect or an error is detected during execution of that command.

   All error messages are written to stderr and are one line long.

3. Hwk5/mainBsh.c contains functions that you may find useful for debugging:
   dumpList() dumps a token list; and dumpTree() dumps a parse tree.in the
   following format:

     Example:  The command line

       < A B | ( C > D & E < F ) > G ; H I

     is parsed as

     .          ;
     .        /   \
     .      PIPE   H I
     .     /    \
     . <A B    SUBCMD
     .        /
     .       &
     .      / \
     .  >D C   E <F

     and printed as

       CMD (Depth = 2):  SIMPLE,  argv[0] = B  <A
       CMD (Depth = 1):  PIPE
       CMD (Depth = 4):  SIMPLE,  argv[0] = C  >D
       CMD (Depth = 3):  SEP_BG
       CMD (Depth = 4):  SIMPLE,  argv[0] = E  <F
       CMD (Depth = 2):  SUBCMD  >G
       CMD (Depth = 0):  SEP_END
       CMD (Depth = 1):  SIMPLE,  argv[0] = H,  argv[1] = I

4. process() frees all storage it allocated before returning to main().

5. Hwk5/process-stub.h contains all of the #include statements for my solution.

6. The easiest way to implement subcommands (and possibly pipelines as well) is
   to use a subshell.

7. No, you may not use system() or /bin/*sh.

8. For simplicity, you may ignore the possibility of error returns from
   malloc()/realloc().  However, process() should free all storage it allocates
   before returning to main() so that all storage has been freed when the
   program exits.

9. You may use setenv() to set environment variables, but you will have to
   declare it or #define _GNU_SOURCE since it is not part of the ANSI Standard.


Fine Points
~~~~~~~~~~~
1. For a simple command, the status is that of the program executed (*), or
   the global variable errno if some system call failed while setting up to
   execute the program.

     (*) This status is normally the value WEXITSTATUS(status), where the
     variable status contains the value returned by the call to wait(&STATUS)
     that reported the death of that process.  However, for processes that are
     killed (i.e., for which WIFEXITED(status) is false), that value may be
     zero.  Thus you should use the expression

       (WIFEXITED(x) ? WEXITSTATUS(x) : 128+WTERMSIG(x))

     instead.

   For a backgrounded command, the status is 0.

   For a pipeline, the status is that of the latest (i.e., rightmost) stage to
   fail, or 0 if the status of every stage is true.  (This is the behavior of
   bash with the pipefail option enabled.)

   For a subcommand, the status is that of the last command/subcommand to be
   executed.

   For a built-in command, the status is 0 if successful, the value of errno if
   a system call failed, and 1 otherwise (e.g., when the number of arguments is
   incorrect).

   Note that this may differ from the status reported by bash.

2. In bash the status $? is an internal shell variable.  However, since Bsh
   does not expand these variables, it has no mechanism to check their value.
   Thus in Bsh the status is an environment variable, which can be checked
   using /usr/bin/printenv (i.e., printenv ?).

3. The command separators && and || have the same precedence, lower than |, but
   higher than ; or &.

   && causes the command following (a simple command, pipeline, or subcommand)
   to be skipped if the current command exits with a nonzero status (= FALSE,
   the opposite of C).  The status of the skipped command is that of the
   current command.

   || causes the command following to be skipped if the current command exits
   with a zero status (= TRUE, the opposite of C).  The status of the skipped
   command is that of the current command.

4. While executing a command, pipeline, or subcommand, Bsh waits until it
   terminates, unless it is followed by an &.  Bsh ignores SIGINT interrupts
   while waiting, but child processes (other than subshells) do not.  Hint:
   Do not implement signals until everything else seems to be working.

5. An EOF (^D in column 1) causes Bsh to exit since getLine() returns NULL.

6. Anything written to stdout by a built-in command is redirectable.

   When a built-in command fails, Bsh continues to execute commands.

   When a built-in command is invoked within a pipeline, is backgrounded, or
   appears in a subcommand, that command has no effect on the parent shell.
   For example, the commands

     (2)$ cd /c/cs323 | ls

   and

     (3)$ ls & cd .. & ls

   do not work as you might otherwise expect.

7. When a redirection fails, Bsh does not execute the command.  The status of
   the command (or pipeline stage) is the errno of the system call that failed.

8. When Bsh runs a command in the background, it writes the process id to
   stderr using the format "Backgrounded: %d\n".

9. Bsh reaps all zombies periodically (i.e., at least once during each call to
   process()) to avoid running out of processes.  When it does so, it writes
   the process id and status to stderr using the format "Completed: %d (%d)\n".


Limitations
~~~~~~~~~~~
The following will be worth at most 12 points each
 * &&, ||, and &
 * grouped commands
 * the status variable $?
Also, pipes will be worth at most 20 points.  Each "at most" is a crude upper
bound intended to give more flexibility in developing the test script and to
allow interactions between features in different groups above.


Appendix
~~~~~~~~
The syntax for a command is

  <stage>    = <simple> / (<command>)
  <pipeline> = <stage> / <pipeline> | <stage>
  <and-or>   = <pipeline> / <and-or> && <pipeline> / <and-or> || <pipeline>
  <sequence> = <and-or> / <sequence> ; <and-or> / <sequence> & <and-or>
  <command>  = <sequence> / <sequence> ; / <sequence> &

where a <simple> is a single command with arguments and I/O redirection, but no
|, &, ;, &&, ||, (, or ).

A command is represented by a tree of CMD structs corresponding to its simple
commands and the "operators" PIPE, && (SEP_AND), || (SEP_OR), ; (SEP_END), &
(SEP_BG), and SUBCMD.  The tree corresponds to how the command is parsed by a
bottom-up using the grammar above.

Note that I/O redirection is associated with a <stage> (i.e., a <simple> or
subcommand), but not with a <pipeline> (input/output redirection for the
first/last stage is associated with the stage, not the pipeline).

One way to write such a parser is to associate a function with each syntactic
type.  That function calls the function associated with its first alternative
(e.g., <stage> for <pipeline>), which consumes all tokens immediately following
that could be part of it.  If at that point the next token is one that could
lead to its second alternative (e.g., | in <pipeline> | <stage>), then that
token is consumed and the associated function called again.  If not, then the
tree is returned.

A CMD struct contains the following fields:

 typedef struct cmd {
   int type;             // Node type (SIMPLE, PIPE, SEP_AND, SEP_OR,
			 //   SEP_END, SEP_BG, SUBCMD, or NONE)

   int nLocal;           // Number of local variable assignments
   char **locVar;        // Array of local variable names and values to assign
   char **locVal;        //   to them when command executes or NULL (default)

   int argc;             // Number of command-line arguments
   char **argv;          // Null-terminated argument vector

   int fromType;         // Redirect stdin?
			 //  (NONE (default), RED_IN, RED_IN_HERE, RED_IN_CLS)
   char *fromFile;       // File to redirect stdin. contents of here
			 //   document, or NULL (default)

   int toType;           // Redirect stdout?
			 //  (NONE (default), RED_OUT, RED_OUT_APP, RED_OUT_CLS,
			 //   RED_OUT_ERR, RED_OUT_RED)
   char *toFile;         // File to redirect stdout or NULL (default)

   struct cmd *left;     // Left subtree or NULL (default)
   struct cmd *right;    // Right subtree or NULL (default)
 } CMD;

The tree for a <simple> is a single struct of type SIMPLE that specifies its
local variables (nLocal, locVar[], locVal[]) and arguments (argc, argv[]);
and whether and where to redirect its standard input (fromType, fromFile) and
its standard output (toType, toFile).  The left and right children are NULL.

The tree for a <stage> is either the tree for a <simple> or a struct
of type SUBCMD (which may have redirection) whose left child is the tree
representing the <command> and whose right child is NULL.

The tree for a <pipeline> is either the tree for a <stage> or a struct
of type PIPE whose left child is the tree representing the <pipeline> and
whose right child is the tree representing the <stage>.

The tree for an <and-or> is either the tree for a <pipeline> or a struct
of type && (= SEP_AND) or || (= SEP_OR) whose left child is the tree
representing the <and-or> and whose right child is the tree representing
the <pipeline>.

The tree for a <sequence> is either the tree for an <and-or> or a struct of
type ; (= SEP_END) or & (= SEP_BG) whose left child is the tree representing
the <sequence> and whose right child is the tree representing the <and-or>.

The tree for a <command> is either the tree for a <sequence> or a struct of
type ; (= SEP_END) or & (= SEP_BG) whose left child is the tree representing
the <sequence> and whose right child is NULL.

While the grammar above captures the syntax of bash commands, it does not
reflect the semantics of &, which specify that only the preceding <and-or>
should be executed in the background, not the entire preceding <sequence>.


Examples (where A, B, C, D, and E are <simple>):

//                              Expression Tree
//
//   A                          A
//
//   < a A | B | C | D > d                     PIPE
//                                            /    \
//                                        PIPE      D >d
//                                       /    \
//                                   PIPE      C
//                                  /    \
//                              <a A      B
//
//   A && B || C && D                   &&
//                                     /  \
//                                   ||    D
//                                  /  \
//                                &&    C
//                               /  \
//                              A    B
//
//   A ; B & C ; D || E ;                 ;
//                                      /
//                                     ;
//                                   /   \
//                                  &     ||
//                                 / \   /  \
//                                ;   C D    E
//                               / \
//                              A   B
//
//   (A ; B &) | (C || D) && E                 &&
//                                            /  \
//                                        PIPE    E
//                                       /    \
//                                    SUB      SUB
//                                   /        /
//                                  &       ||
//                                 /       /  \
//                                ;       C    D
//                               / \
//                              A   B

								CS-323-11/03/15
