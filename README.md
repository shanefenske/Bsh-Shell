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


