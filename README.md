Program Origin and Purpose
This program was developed as a project assignment for my university course in System-Level Programming. During this task, we explored deeper aspects of the C programming language, helping us understand how commands and code snippets transform into processes that can be interpreted by the computer.

General Description
The program enables two concurrently running versions of itself to communicate either through sockets or files. During the communication, the receiver program obtains data from the sender program necessary to generate a two-color depth image. This data includes the image size and specific roaming values.

System Requirements
The program was written using GCC version 11.3.0. For proper functionality, please use this version on a Linux 22.04 operating system. To compile the program, enter the following command in the terminal:

bash
Copy code
gcc chart.c myown.c -fopenmp -o chart
To run the program, use the following command with the appropriate switches separated by spaces:

bash
Copy code
./chart
Available Switches:
--version: Displays the current version of the program, the date of the last modification, and other information in random order.
--help: Provides help on how to properly run the program. (This switch will also be triggered if the user provides an incorrect argument.)
-send/-receive: Runs the program in sender/receiver mode. If no mode is specified, the program defaults to sender mode (-send).
-file/-socket: Specifies the communication method between the programs. If no method is provided, file-based communication will be used by default.
Note: The order of the switches is interchangeable; the program will run correctly regardless of the order.

Program Functions
void MyExit(char *message, int num):
Prints the provided message and exits the program with the corresponding error code. (If 0 is provided, the message will be printed to stdout instead of stderr.)

int After_Quarter():
Returns how many seconds have passed since the last quarter-hour.

int Measurment(int **Values):
The provided Values pointer will point to dynamically allocated memory containing the same number of data entries as indicated by After_Quarter(). The first value is always 0, and subsequent values either increase by 1 or remain the same. The function returns the number of data points generated.

void BMPCreator(int* Values, int NumValues):
Generates a file named chart.bmp in the program’s directory. The Values array contains the data, and NumValues represents the number of data points.

void FileWalk(char *path, int* result):
Iterates through the specified folder (path) and, if it finds a process with the name chart in the status file of a subfolder starting with a number, updates the result variable.

int FindPid():
Calls the FileWalk method on the /proc directory. If only the program’s own PID is found, the function returns -1.

void SendViaFile(int* Values, int NumValues):
Creates a Measurments.txt file in the program’s directory, writing the array data in the format %d\n. It then sends a user-defined signal to the receiver process.

void ReceiveViaFile(int sig):
Opens the Measurments.txt file from the default user directory and reads it into dynamically allocated memory (which can grow as needed).

void SignalHandler(int sig):
A signal handler capable of processing incoming signals. Upon receiving SIGUSR1, it calls ReceiveViaFile(). Upon receiving SIGINT, it bids farewell to the user. When SIGALRM is received, it informs the user of the server’s status.

void SendViaSocket(int* Values, int NumValues):
Sends the NumValues and Values array through a socket to localhost (127.0.0.1) on port 3333. After each transmission, it verifies that the server has acknowledged receipt.

void ReceiveViaSocket():
Sets up a socket server to listen on port 3333 for an integer, which it returns as an acknowledgment. Then, it waits for an array of integers, the size of which is the previously received integer. After receiving the array, it sends back the number of received items as an acknowledgment. The server then waits for the next message.

void Start(int argc, char *argv[]):
The program receives the number of command-line arguments (argc) and an array of arguments (argv). It evaluates them and runs the program based on the specified switches.

Error Messages
The program was called with incorrect arguments.
The executable is not named chart.
Failed to create the Measurments.txt file.
The program received a SIGUSR1 signal while not in receiver mode.
The program received a SIGALRM signal, and the server did not respond in time.
Failed to create the sender socket.
Failed to transmit NumValues over the socket.
Failed to receive an acknowledgment for NumValues.
Received an incorrect acknowledgment for NumValues.
Failed to transmit the Values array over the socket.
Failed to receive an acknowledgment for Values.
Received an incorrect acknowledgment for Values.
Failed to bind the socket.
Failed to receive NumValues via socket.
Failed to send an acknowledgment for NumValues via socket.
Failed to receive Values via socket.
Failed to send an acknowledgment for Values via socket.
