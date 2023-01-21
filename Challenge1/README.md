# Challenge_1

## Code Explanation
The myshell program simulates a shell environment, allowing the user to pass commands throuhg the terminal input, and then run them as if they were inputed in a Linux shell. The program utilizes the parser functions, myshell_parser.c and myshell_parser.h, to parse through the user input and put in a format that is easily useable by the execvp call.

## Code Flow
### Startup
To compile and run the code, the user simply needs to run make in the terminal and the included makefile will generate the necessary myshell executable. After that, run the ./myshell command and the program should start. Note: running ./myshell -n will run the process without the $myshell prompt

### Normal Process
After startup, the user will be prompted with $myshell (or nothing if executed with -n) and will put in their commands. After their input, the program will run the pipeline_build function declared in the myshell_parser files and run the various error checks before continueing to execution. The code will then run a while loop which will fork a process, connect the input/output to any redirects or pipes, execute a command, and continue onto the next command until there are none left. After the complete list of commands has been executed, the program will prompt for another input until the user exits.

### Exiting the Program
To exit the program, the user can input ctrl+d and the program will quit.

## Known Code Issues
1.) The current command flow works perfectly for a single command or two commands connected by a single pipe, however, for an input with 2 or more pipes, the program will enter a hold state that it can never exit due to a pipe close issue. If this happens, use ctrl+c to exit. 

2.) The program currently does not support background commands as the implemntation caused a wide range of issues including a constant print of the $myshell prompt, so that code has been commented out for now.

3.) There is a memory management issue stemming from the pipeline_free function call as it leaves extra letters in the pipeline that will stop the next inputs commands from running properly, so right now pipeline is only freed when the user exits.

Note: I am unsure why the "The shell can report redirection errors" test fails in gradescope. The myshell_parser.c file scans for <<, >>, and & not at the end of line and sets an error bool in the pipeline_build function which then sets an error in the myshell.c file. The myshell.c file checks for redirect in/out in improper places and if the redirect in or out files cannot open and sends an error message.
