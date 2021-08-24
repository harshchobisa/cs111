#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <zlib.h>
#include <assert.h>


int serversock, writesockfd;

z_stream fromClient;
z_stream toClient;

struct option getOptStruct[] =
{
    {"port", 1, 0, 1},
    {"compress", 0, 0,2},
    {0, 0, 0, 0}
};

void closeSockets(void)
{
    close(serversock);
    close(writesockfd);
    deflateEnd(&toClient);
    inflateEnd(&fromClient);
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char* argv[])
{
    int returnValue = getopt_long(argc, argv, "", getOptStruct, NULL); 
    int port;
    int portno;
    int compress = 0;
    //parses through command line arguments
    while (1)
    {
        if (returnValue < 0)
        {
            break;
        }
        if (returnValue == 1) //port
        {
            port = 1;
            portno = atoi(optarg);
        }
        else if (returnValue == 2) //compress arg
        {
            compress = 1;
        }
        else
        {
            fprintf(stderr, "Invalid inputs formatting. --port is a required argument. Arguments should be formatted as such: ./lab1b --port \n");
            exit(EXIT_FAILURE);
        }
        returnValue = getopt_long(argc, argv, "", getOptStruct, NULL);
    }

    if (port == 0)
    {
        fprintf(stderr, "Invalid inputs formatting. --port is a required argument. Arguments should be formatted as such: ./lab1b --port \n");
        exit(EXIT_FAILURE);
    }

    //this code is copied from the rpi resource linked on the spec

    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    serversock = socket(AF_INET, SOCK_STREAM, 0);
    if (serversock < 0){
        fprintf(stderr, "ERROR opening socket\n");
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(serversock, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0){
        fprintf(stderr, "ERROR on binding\n");
    }
    listen(serversock, 5);
    clilen = sizeof(cli_addr);
    writesockfd = accept(serversock, (struct sockaddr *)&cli_addr, &clilen);
    if (writesockfd < 0){
        fprintf(stderr, "ERROR on accept\n");
    }

    //need to make sure to close the sockets and deflate/inflate ends
    //when program exits 
    atexit(closeSockets);

    //declare pipes
    int parentToChildPipe[2];
    int childToParentPipe[2];
    int writeConstant = 1;
    int readConstant = 0;
        
    int checkone = pipe(childToParentPipe);
    int checktwo = pipe(parentToChildPipe);
    if (checkone == -1 || checktwo == -1)
    {
        fprintf(stderr, "Unable to create pipes successfully.");
        exit(EXIT_FAILURE);
    }

    //sets up inflation and deflation if compression flag is present
    //initialize zlib state for compression, code is from zlib.net
    if (compress > 0)
    {
        toClient.zalloc = Z_NULL;
        toClient.zfree = Z_NULL;
        toClient.opaque = Z_NULL;
        int chec = deflateInit(&toClient, Z_DEFAULT_COMPRESSION);


        if (chec != Z_OK)
        {
            fprintf(stderr, "Deflation error");
            exit(EXIT_FAILURE);
        }
        
        fromClient.zalloc = Z_NULL;
        fromClient.zfree = Z_NULL;
        fromClient.opaque = Z_NULL;
        chec = inflateInit(&fromClient);


        if (chec != Z_OK)
        {
            fprintf(stderr, "Inflation error");
            exit(EXIT_FAILURE);
        }
    }


    int processid = fork();

    if (processid < 0) //error when forking
    {
        fprintf(stderr, "Unable to fork successfully.");
        exit(EXIT_FAILURE);
    }

    if (processid > 0) //parent branch
    {
        //close unecessary ends of pipe
        close(parentToChildPipe[readConstant]);
        close(childToParentPipe[writeConstant]);

        struct pollfd eventDescriptors[] = { 
            {writesockfd, POLLIN | POLLERR | POLLHUP, 0}, 
            {childToParentPipe[readConstant], POLLIN | POLLERR | POLLHUP, 0}
        };

        int breakFlag = 0;

        while (breakFlag == 0)
        {
            int checker = poll(eventDescriptors, 2, 0);

            if (checker == -1)
            {
                fprintf(stderr, "Unable to poll.");
            }

            //changed to socket from previous lab
            short socketRevent = eventDescriptors[readConstant].revents; 
            short shellRevent = eventDescriptors[writeConstant].revents;
            
            if (POLLIN & shellRevent) //send output from shell to terminal
            {
                int stdin = STDIN_FILENO;
                int stdout = STDOUT_FILENO;
                char inputBuf[256];
                int size = read(childToParentPipe[readConstant], inputBuf, 256);
                //read in large chunk
                
                if (size < 0)
                {
                    //print error
                    fprintf(stderr, "Could not read from pipe");
                    exit(EXIT_FAILURE);
                }

                int compsize = 512;
                char compressionBuffer[512];
                //code is taken from TA slides
                if (compress == 1)
                {

                    toClient.avail_in = size;
                    toClient.next_in = (Bytef*) inputBuf;

                    do{
                        toClient.avail_out = compsize;
                        toClient.next_out = (Bytef*) compressionBuffer;
                        int h;
                        h = deflate(&toClient, Z_SYNC_FLUSH);
                        if (h == Z_STREAM_ERROR)
                        {
                            fprintf(stderr, "Error while deflating");
                            exit(1);
                        } 
                    } while(toClient.avail_in > 0);
                    assert(toClient.avail_in == 0);

                    //if compressed, change our buffer to the compressed version
                    memcpy(inputBuf, compressionBuffer, compsize - toClient.avail_out);

                }


                //if we have compressed, we need to adjust the size of the buffer
                if (compress == 1)
                {
                    size = compsize - toClient.avail_out;
                }
                    
                int x;
                for (x = 0; x < size; x++)
                {
                    char inputChar = inputBuf[x];
                    write(writesockfd, &inputChar, 1);
                }
            }

            if (POLLIN & socketRevent) //send text from terminal to shell
            {
                int stdin = STDIN_FILENO;
                int stdout = STDOUT_FILENO;
                int size;

                char inputBuf[256];

                size = read(writesockfd, inputBuf, 256);

                int compsize = 512;
                char compressionBuffer[512];
                
                //code taken from TA slides
                if (compress == 1)
                {

                    fromClient.avail_in = size;
                    fromClient.next_in = (Bytef*) inputBuf;

                    do{
                        fromClient.avail_out = compsize;
                        fromClient.next_out = (Bytef*) compressionBuffer;
                        int h;
                        h = inflate(&fromClient, Z_SYNC_FLUSH);
                        if (h == Z_STREAM_ERROR)
                        {
                            fprintf(stderr, "Error while deflating");
                            exit(1);
                        } 
                    } while(fromClient.avail_in > 0);
                    assert(fromClient.avail_in == 0);
                    
                    //if compressed, change our buffer to the compressed version
                    memcpy(inputBuf, compressionBuffer, compsize - fromClient.avail_out);
                }

                //read input into a large buffer
                if (size < 0)
                {
                    //print error
                    fprintf(stderr, "Could not read from child to parent pipe");
                    exit(EXIT_FAILURE);
                }

                //if we have compressed, we need to adjust the size of the buffer
                if (compress == 1)
                {
                    size = compsize - fromClient.avail_out;
                }

                //process input one character at a time
                int x;
                for (x = 0; x < size; x++)
                {
                    //write back to terminal to show user what command they typed
                    //send it to the shell so it will be executed
                    char inputchar = inputBuf[x];
                    if (inputchar == '\004')
                    {
                        // write(1, "^D", 2);
                        // inputchar = '\015';
                        // write(stdout, &inputchar, 1);
                        close(parentToChildPipe[writeConstant]);
                        break; 
                    }
                    else if (inputchar == '\003')
                    {
                        // write(1, "^C", 2);
                        // inputchar = '\015';
                        // write(stdout, &inputchar, 1);
                        kill(processid, SIGINT);
                        break; 
                    }
                    else if (inputchar == '\015' || inputchar == '\012')
                    {
                        // inputchar = '\015';
                        // write(stdout, &inputchar, 1);
                        //only print line feed to shell, not carraige return
                        inputchar = '\012';
                        // write(stdout, &inputchar, 1);
                        write(parentToChildPipe[writeConstant], &inputchar, 1);
                        break;
                    }
                    else
                    {
                        // write(stdout, &inputchar, 1);
                        write(parentToChildPipe[writeConstant], &inputchar, 1);
                    }
                }
            }

            //if we find errors, quit the program
            if (POLLHUP & shellRevent || POLLERR &shellRevent)
            {
                int exitStat;
                waitpid(processid, &exitStat, 0);
                fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d \n\r", WTERMSIG(exitStat) ,WEXITSTATUS(exitStat));
                exit(0);
            }
        }
    }
    else if (processid == 0) //child pipe
    {
        //close unused pipes
        close(parentToChildPipe[writeConstant]);
        close(childToParentPipe[readConstant]);

        dup2(parentToChildPipe[readConstant], STDIN_FILENO);
        dup2(childToParentPipe[writeConstant], STDOUT_FILENO);
        dup2(childToParentPipe[writeConstant], STDERR_FILENO);

        close(parentToChildPipe[readConstant]);
        close(childToParentPipe[writeConstant]);

        //creates shell
        char *arguments[2];
        arguments[0] = "/bin/bash";
        arguments[1] = NULL;
        int retVal = execvp(arguments[0], arguments);
        if (retVal == -1)
        {
            fprintf(stderr, "Error when running execvp()");
            exit(EXIT_FAILURE);
        }
    }
    
}