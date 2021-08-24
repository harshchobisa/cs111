#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


//designed based on getopt_long documentation
struct option getOptStruct[] =
{
    {"input", 1, 0, 1},
    {"output", 1, 0, 2},
    {"segfault", 0, 0, 3},
    {"catch", 0, 0, 4},
    {0, 0, 0, 0}
};

//based on Karen Quadros the TA
void handle_sig() 
{ 
    fprintf(stderr, "Segmentation fault found\n");
    exit(4);
} 

void segfault()
{
    char *dummy = NULL;
    *dummy = 'h';
}

int main(int argc, char **argv)
{

    // int getopt_long(int argc, char * const argv[],
    // const char *optstring,
    // const struct option *longopts, int *longindex);
    char *input = NULL;
    char *output = NULL;

    int foundSegFault = 0;
    int s = 0;

    int returnValue = getopt_long(argc, argv, "", getOptStruct, NULL); 
    
    while (1)
    {
        if (returnValue < 0)
        {
            break;
        }
        if (returnValue == 1)
        {
            input = optarg;
        }
        else if (returnValue == 2)
        {
            output = optarg;
        }
        else if (returnValue == 3)
        {
            //CAUSE A SEG FAULT
            foundSegFault = 1;

        }
        else if (returnValue == 4)
        {
            s = 1;
        }
        else
        {
            fprintf(stderr, "Invalid inputs formatting. Arguments should be formatted as such: ./lab0 --input=sample.txt --output=sample.txt --segfault --catch \n");
            exit(1);
        }
        returnValue = getopt_long(argc, argv, "", getOptStruct, NULL);

    }

    //the below code is taken from Karen Quadros the TA

    int ifd = open(input, O_RDONLY);
    
    if(input)
    {
        if (ifd >= 0)
        {
            close(0);
            dup(ifd);
            close(ifd);
        }
        else
        {
            //could not open file
            fprintf(stderr, "Unable to open file %s. Error message given is: %s. Error code given is: %d\n", input, strerror(errno), errno);
            exit(2);
        }
        
    }
    
    int ofd = creat(output, 0644);
    if(output)
    {

        if (ofd >= 0)
        {
            close(1);
            dup(ofd);
            close(ofd);
        }
        else
        {
            fprintf(stderr, "Unable to open file %s. Error message given is: %s. Error code given is: %d\n", input, strerror(errno), errno);
            exit(3);
        }
        
    }

    if (s == 1)
    {
        //taken from Karen Quadros the TA
        signal(SIGSEGV, handle_sig);
    }

    if (foundSegFault == 1)
    {
        segfault();
    }


    char inputBuffer[1];

    while (read(0, inputBuffer, 1) == 1)
    {
        write(1, inputBuffer, 1);
    }

    exit(0);
}