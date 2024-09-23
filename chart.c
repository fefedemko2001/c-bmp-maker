#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "myown.h"


typedef char* string;

int main(int argc, string argv[])
{


    signal(SIGINT, SignalHandler);
    signal(SIGUSR1, SignalHandler);
    signal(SIGALRM, SignalHandler);

    Start(argc, argv);
    ReceiveViaFile(0);

    return 0;
}