#ifndef MYOWN
#define MYOWN

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <omp.h>

void MyExit(char *message, int num);
int After_Quarter();
int Measurment(int **Values);
void BMPcreator(int* Values, int NumValues);
void FileWalk(char *path, int* result);
int FindPid();
void SendViaFile(int *Values, int NumValues);
void ReceiveViaFile(int sig);
void SignalHandler(int sig);
void SendViaSocket(int *Values, int NumValues);
void ReceiveViaSocket();
void Start(int argc, char* argv[]);


#endif