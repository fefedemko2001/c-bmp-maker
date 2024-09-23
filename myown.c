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


void MyExit(char *message, int num)
/* Exit the program with the appropriate message with the given exit code */
{
    if (num == 0){
        printf("%s\n", message);
        exit(0);
    }
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "HIBAKOD:%d\n", num);

    exit(num);
}


int After_Quarter()
/*Returns how many seconds have past sine the last quarter (00,15,30,45) (min value is 100)*/
{
    time_t T;
    int T2;
    struct tm *T3;
    T2 = time(&T);
    T3 = localtime(&T);

    int result = ((*T3).tm_min % 15) * 60 + (*T3).tm_sec;
    /*minutes after the last quarter *60 + seconds after the last min*/
    if (result < 100){
        return 100;
    }
    return result;

}

int Measurment(int **Values)
/*
    Dynamically allocate memory for an array which size is calculated by the After_Quarter function.
    The first element is alway 0 and the next element is always incremented (with the odds of 42.5871%)
    or decremented (with the odds of 11/31) with one or doesn't change.
    The returned value is the number of values in the array.
*/
{
    int size = After_Quarter();
    srand(time(NULL));
    float r;
    /*Allocating the memory*/
    *Values= malloc(sizeof(int)*size);
    (*Values)[0] = 0;

    for (int i = 1; i < size; i++){
        r = (float)rand()/RAND_MAX;
        if (r < 11.0/31.0){
            (*Values)[i] = (*Values)[i-1]-1;
        }
        else if (r > 1.0-0.425871){
            (*Values)[i] = (*Values)[i-1]+1;
        }
        else{
            (*Values)[i] = (*Values)[i-1];
        }
    }
    return size;
}

void BMPcreator(int* Values, int NumValues)
/*Creates an BMP image which represent the values in the Values array.*/
{
    int x,y;
    int zero = 0;
    int pixoffset = 62;
    int dibsize = 40;
    short int sint = 1;
    int pixmeter = 3937;

    x = open("Chart.bmp", O_CREAT | O_WRONLY | O_TRUNC, 0644);

    /*BM Start*/
    y = write(x, "BM", 2);

    /*
    Calculating the size of the picture.
    The intperrow is the number of ints per row we need to represent correctly the picture.
    The number of data is divided by 32 which is the number of bits in an int.
    */
    int intperrow = (NumValues/32 + 1);
    int size = 62 + (intperrow * NumValues);
    y = write(x, &size, sizeof(int));

    /*unused bytes*/
    y = write(x, &zero, sizeof(int));

    /*pixel offset*/
    y = write(x, &pixoffset, sizeof(int));

    /*DIB part*/

    /*DIB header size*/
    y = write(x, &dibsize, sizeof(int));

    /* Height & width */
    y = write(x, &NumValues, sizeof(int));
    y = write(x, &NumValues, sizeof(int));

    /*Planes & Bit/pixel*/
    y = write(x, &sint, 2);
    y = write(x, &sint, 2);

    /*Compression & Image size*/
    y = write(x, &zero, sizeof(int));
    y = write(x, &zero, sizeof(int));

    /*Pixel/meter*/
    y = write(x, &pixmeter, sizeof(int));
    y = write(x, &pixmeter, sizeof(int));

    /*Colors in palette & Used palette colors*/
    y = write(x, &zero, sizeof(int));
    y = write(x, &zero, sizeof(int));

    /*Palette*/
    char palette[] = {220,180,120,0,0,80,0,0};
    y = write(x, &palette, sizeof(palette));

    /*Pixel array
    1.step: The size of the pixel array will be calculated using the NumValues multiplied with
    the ints needed for each row(it was calculated previously).

    2.step: The reason why I used unsigned int because its byter order goes like this:
    byte 1-8: 2^7->2^0
    byte 9-16: 2^15->2^8
    byte 17-24: 2^23->2^16
    byte 25-32: 2^31->2^24

    3.step: I make sure that each of the values is 0.

    4.step: In the for loop I calculate in which int's bit need to be changed
    middle (the smaller half of the NumValues) multiplied with the number of ints in each row
    + the Value of the i-th element of the Values array multiplied with the number of ints in each row
    + the i/32 which show us which ints bit we need to change
    This calculation will show wich element of the pixel array needs to be increased.

    5.step:We increment the calcuted values with the correct value to change one of its bit from 0 to 1
    */
    unsigned int pixels[intperrow*NumValues];
    memset(pixels, 0, sizeof(pixels));
    unsigned int twopows[32] = {128, 64, 32, 16, 8, 4, 2, 1, 32768, 16384, 8192, 4096, 2048, 1024, 512, 256, 8388608, 4194304, 2097152, 1048576, 524288, 262144, 131072, 65536, 2147483648, 1073741824, 536870912, 268435456, 134217728, 67108864, 33554432, 16777216};
    int middle = NumValues/2;

    for(int i = 0; i < NumValues; i++){
        int Rowindex = middle*intperrow + (Values[i]*intperrow) + (i/32);
        pixels[Rowindex] += twopows[i%32];
    }

    y = write(x, pixels, sizeof(pixels));
}


void FileWalk(char *path, int* result)
/*
This function will walk through the given path and finds the correct PID number which:
-in a directroy which name is made of numbers
-the filename is status
-the first line of the file is like "Bash:\tchart\n"
-contains the line which is like "Pid:\t%d"
*/
{
    DIR *dir;
    struct dirent *entry;
    char buffer[1024];
    dir = opendir(path);
    if (!dir) return;

    while ((entry = readdir(dir)))
    {
        if (entry->d_type == DT_DIR) 
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)continue;

            char sub_path[1024];
            snprintf(sub_path, sizeof(sub_path), "%s/%s", path, entry->d_name);
            if (isdigit(entry->d_name[0]))
            {
                FileWalk(sub_path, result);
            }
        }
        else
        {
            if (strcmp(entry->d_name, "status") == 0)
            {
                chdir(path);
                FILE *fp = fopen(entry->d_name, "r");
                int bash = 0;
                while(fgets(buffer, 1024, fp) != NULL) {
                    if(strncmp(buffer, "Name:\tchart\n", 12) == 0)bash = 1;
                    if (bash && strncmp(buffer, "Pid:\t", 5) == 0)
                    {
                        char * token = strtok(buffer, "\t");
                        token = strtok(NULL, "\t");
                        if (atoi(token) != getpid()) *result = atoi(token);
                    }
                }
                fclose(fp);
            }
        }
    }
    closedir(dir);
    return;
}

int FindPid()
/*
This function returns the PID as an integr.
*/
{
    int result = -1;
    FileWalk("/proc", &result);
    return result;
}

void SendViaFile(int *Values, int NumValues)
/*
This function writes the values into a file called "Measurments.txt"
*/
{
    FILE *fp = fopen("Measurements.txt", "w");

    for(int i = 0; i < NumValues; i++){
        fprintf(fp, "%d\n", Values[i]);
    }

    fclose(fp);
    if (FindPid() == -1) MyExit("A program nem talalt kuldo folyamatot\n",15);
    kill(FindPid(), SIGUSR1);

}

void ReceiveViaFile(int sig)
/*
This function reads the values from a file called "Measurments.txt" and dinamically allocates them.
After reading it creates the BMP picture and frees the memory.
*/
{
    int counter = 0;
    int size = 0;
    int *values = NULL;
    char buf[1024];

    FILE *fp = fopen("Measurements.txt", "r");

    if (fp == NULL) MyExit("Measurements.txt is not found!",3);

    while(fgets(buf,1024,fp) != NULL)
    {
        if (counter == size)
        {
            if (size == 0)size = 1;
            size *= 2;
            int* newValues = realloc(values,size*sizeof(int));
            values = newValues;
        }
        counter++;
        values[counter-1] = atoi(buf);
    }
    BMPcreator(values, counter);
    free(values);
    fclose(fp);
}



void SignalHandler(int sig)
/*
SignaHandler for the SIGINT, SIGUSR1 and SIGALRM signals.
This function is able to handle this signals.
*/
{
    if (sig == SIGINT) MyExit("Good Night!\n",0);
    else if (sig == SIGUSR1) MyExit("A fajlon keresztuli szolgaltatas nem elerheto!",4);
    else if (sig == SIGALRM) MyExit("A szerver nem valaszolt idokereten belul!",5);
}

void SendViaSocket(int *Values, int NumValues)
/*
Creats a socket client usind UDP connection to send the values array and number of values using
the socket connection to the server socket with the IP of "127.0.0.1" to the 3333 port.
At first it sends the number of Values to the server
After that it checks that the receipt is equal to the number of values
Second it sends the values to the server and checks that the receipt is equal to the
size of the array in bytes.
*/
{
    /************************ Declarations **********************/
   int s;                            // socket ID
   int bytes;                        // received/sent bytes
   int flag;                         // transmission flag
   char on;                          // sockopt option
   char buffer[1024];             // datagram buffer area
   unsigned int server_size;         // length of the sockaddr_in server
   struct sockaddr_in server;        // address of server

   /************************ Initialization ********************/
   on   = 1;
   flag = 0;
   server.sin_family      = AF_INET;
   server.sin_addr.s_addr = inet_addr("127.0.0.1");
   server.sin_port        = htons(3333);
   server_size = sizeof server;

   /************************ Creating socket *******************/
   s = socket(AF_INET, SOCK_DGRAM, 0 );
   if ( s < 0 ) MyExit("Socket creation error.", 6);
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
   setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

   /************************ Sending the Number of Values **********************/
   
    bytes = sendto(s, &NumValues, sizeof(NumValues), flag, (struct sockaddr *) &server, server_size);
    if ( bytes <= 0 ) MyExit("Sending error.", 7);
    signal(SIGALRM, SignalHandler);
    sleep(1);
    signal(SIGALRM,SIG_IGN);

    int receipt=0;
   /************************ Receive the Number of Values receipt **********************/
   bytes = recvfrom(s, &receipt, sizeof(int), flag, (struct sockaddr *) &server, &server_size);
   if ( bytes < 0 ) MyExit("Receiving error.", 8);
   if(receipt != NumValues) MyExit("Receiving error.", 9);


   /************************ Sending array **********************/

   bytes = sendto(s, Values, sizeof(int) * NumValues, flag, (struct sockaddr *) &server, server_size);
   if ( bytes <= 0 ) MyExit("Sending error.\n", 10);

   /************************ Receive the size of the array **********************/
   bytes = recvfrom(s, &receipt, sizeof(int), flag, (struct sockaddr *) &server, &server_size);

   if ( bytes < 0 ) MyExit("Receiving error.\n", 11);
   if(receipt != sizeof(int) * NumValues) MyExit("Receiving error.\n", 12);
   printf("Successfully received\n");

   /************************ Closing ***************************/
   close(s);
   return;
}

void ReceiveViaSocket()
/*
Creats an UDP server which is waiting for incoming data on the port of 3333
At first it receivs an integer number and send it back to the client as a receipt and
after it receives an array of values and send the size back as a receipt.
Finally it creats the BMP picture with the received data.
*/
{

   /************************ Declarations **********************/
   int bytes;                        // received/sent bytes
   int err;                          // error code
   int flag;                         // transmission flag
   char on;                          // sockopt option
   char buffer[1024];             // datagram buffer area
   int numbuffer[1500];             // datagram buffer for the array
   unsigned int server_size;         // length of the sockaddr_in server
   unsigned int client_size;         // length of the sockaddr_in client
   struct sockaddr_in server;        // address of server
   struct sockaddr_in client;        // address of client

   /************************ Initialization ********************/
   on   = 1;
   flag = 0;
   int s;
   server.sin_family      = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port        = htons(3333);
   server_size = sizeof server;
   client_size = sizeof client;
   signal(SIGINT, SignalHandler);

   /************************ Creating socket *******************/
   s = socket(AF_INET, SOCK_DGRAM, 0 );
   if ( s < 0 ) {
      fprintf(stderr, "Socket creation error.\n");
      exit(2);
      }
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
   setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

   /************************ Binding socket ********************/
   err = bind(s, (struct sockaddr *) &server, server_size);
   if ( err < 0 ) MyExit("Binding socket error", 13);
    int valami = 1;
    int NumValues = -1;
    while(1)
    {
        /************************ Receive the number of data **********************/
    printf("\n Waiting for a message...\n");
    bytes = recvfrom(s, &NumValues, sizeof(NumValues), flag, (struct sockaddr *) &client, &client_size );
    if ( bytes < 0 ) MyExit("Receiving error", 14);
    //printf("Megkapott adat db: %d\n", NumValues);
    //int* Values = malloc(NumValues*sizeof(int));


    /************************ Sending data **********************/
    bytes = sendto(s, &NumValues, sizeof(NumValues), flag, (struct sockaddr *) &client, client_size);
    if ( bytes <= 0 ) MyExit("Sending error",15);
    //printf(" Acknowledgement have been sent to client.\n");

    /************************ Receive the array **********************/
    printf("\n Waiting for the array...\n");
    int Values[NumValues];
    
    bytes = recvfrom(s, Values, NumValues * sizeof(int), flag, (struct sockaddr *) &client, &client_size );
    if ( bytes < 0 ) MyExit("Receiving error",16);
        

    BMPcreator(Values, NumValues);

    /************************ Sending data **********************/
    int size = NumValues * sizeof(int);
    bytes = sendto(s, &size, sizeof(int), flag, (struct sockaddr *) &client, client_size);
    if ( bytes <= 0 ) MyExit("Sending error",17);
    }

}

void Start(int argc, char* argv[])
/*
Checks that which mode the user want to use.
If the argument is not valid it will print a help message
*/
{
    if (strcmp(argv[0], "./chart")) MyExit("Hibasan leforditott program!\n",2);
    if (argc == 2 )
    {
        if (strcmp(argv[1], "--version") == 0)
        {
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    printf("Nagyon sokadik verzio.\n");
                }
                #pragma omp section
                {
                    printf("Kezdes datuma: 2023.02.20.\n");
                }
                #pragma omp section
                {
                    printf("Befejezes datuma: 2023.04.24.\n");
                }
                #pragma omp section
                {
                    printf("Keszitette: Demko Ferenc\n");
                }
                #pragma omp section
                {
                    printf("Neptun kod: H617W5\n");
                }
                #pragma omp section
                {
                    printf("Kar: IK\n");
                }
                #pragma omp section
                {
                    printf("Szak: mernokinformatikus\n");
                }
            }
            exit(0);
        }
        else if(strcmp(argv[1], "-receive") == 0){
            signal(SIGUSR1, ReceiveViaFile);
            while(1)
            {
                printf("Waitng...\n");
                sleep(5);
            }
        }
        else if(strcmp(argv[1], "-socket") == 0){
            printf("socket\n");
            int *tomb;
            int n = Measurment(&tomb);
            SendViaSocket(tomb, n);
            free(tomb);
            exit(0);
        }
        else if(strcmp(argv[1], "-file") == 0 || strcmp(argv[1], "-send") == 0){
            printf("standard\n");
            int *tomb;
            int n = Measurment(&tomb);
  
            SendViaFile(tomb, n);
            free(tomb);
            exit(0);
        }
        else
        {
            fprintf(stderr,
            "A programot az alabbi agrumentumokkal lehet futtatni:\n"
            "-version: a programrol ad informaciokat\n"
            "Illetve van 2 db kapcsolo, amit lehet allitani:\n"
            "-send/-receive: kuldeni vagy fogadni szeretnenk az adatokat\n"
            "-socket/-file: file-on vagy socketen keresztul tortenjen az adatatvitel\n" 
            );
            fprintf(stderr, "HIBAKOD:%d\n", 1);

            exit(1);
        }
    }
    else if (argc == 3)
    {
        if ((strcmp(argv[1], "-receive") == 0 && strcmp(argv[2], "-socket") == 0)
            ||(strcmp(argv[1], "-socket") == 0 && strcmp(argv[2], "-receive") == 0))
        {

            ReceiveViaSocket();
        }
        else if ((strcmp(argv[1], "-receive") == 0 && strcmp(argv[2], "-file") == 0)
                ||(strcmp(argv[1], "-file") == 0 && strcmp(argv[2], "-receive") == 0))
        {
            signal(SIGUSR1, ReceiveViaFile);
            while(1)
            {
                printf("Waitng...\n");
                sleep(5);
            }
        }
        else if ((strcmp(argv[1], "-send") == 0 && strcmp(argv[2], "-socket") == 0) ||(strcmp(argv[1], "-socket") == 0 && strcmp(argv[2], "-send") == 0))
        {

            int *tomb;
            int n = Measurment(&tomb);
            SendViaSocket(tomb, n);
            free(tomb);
            exit(0);
        }
        else if ((strcmp(argv[1], "-send") == 0 && strcmp(argv[2], "-file") == 0) ||(strcmp(argv[1], "-file") == 0 && strcmp(argv[2], "-send") == 0))
        {
            int *tomb;
            int n = Measurment(&tomb);
  
            SendViaFile(tomb, n);
            free(tomb);
            exit(0);
        }
        else fprintf(stderr,
            "A programot az alabbi agrumentumokkal lehet futtatni:\n"
            "-version: a programrol ad informaciokat\n"
            "Illetve van 2 db kapcsolo, amit lehet allitani:\n"
            "-send/-receive: kuldeni vagy fogadni szeretnenk az adatokat\n"
            "-socket/-file: file-on vagy socketen keresztul tortenjen az adatatvitel\n" 
            );
            fprintf(stderr, "HIBAKOD:%d\n", 1);

            exit(1);

    }
    else fprintf(stderr,
            "A programot az alabbi agrumentumokkal lehet futtatni:\n"
            "-version: a programrol ad informaciokat\n"
            "Illetve van 2 db kapcsolo, amit lehet allitani:\n"
            "-send/-receive: kuldeni vagy fogadni szeretnenk az adatokat\n"
            "-socket/-file: file-on vagy socketen keresztul tortenjen az adatatvitel\n" 
            );
            fprintf(stderr, "HIBAKOD:%d\n", 1);

            exit(1);

}


#endif
