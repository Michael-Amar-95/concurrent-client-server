#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#include <errno.h>

#include <dirent.h>


#ifndef MAX_BUF
#define MAX_BUF 256
#endif

/**
 * This function delete to_srv file if exist
 */
void delete_to_srv() {
    //directory descriptor
    DIR *dir;
    struct dirent *dp;

    errno = 0;
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        // if c`ant get path than move file by default
        remove("to_srv");
        return;
    }

    if ((dir = opendir(cwd)) == NULL) {
        // if c`ant open directory than move file by default
        remove("to_srv");
        return;
    }
    while ((dp = readdir(dir)) != NULL) {
        if (!strcmp(dp->d_name, "to_srv"))
            if (remove("to_srv") < 0) {
                if (write(2, "Error in: remove\n", strlen("Error in: remove\n")) < 0);
            };
    }
    closedir(dir);

    free(cwd);
}

/**
 * This function calculate the expression.
 * @param firstNum - the first num
 * @param action - the action between the nums
 * @param num2 - the second num
 * @return the result
 */
int calculator(int firstNum, int action, int secondNum) {
    // look for the action
    switch (action) {
        case 1:
            return firstNum + secondNum;
        case 2:
            return firstNum - secondNum;
        case 3:
            return firstNum * secondNum;
        case 4:
            return firstNum / secondNum;
        default:
            write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
            remove("to_srv");
            system("rm -f to_client_*");
            exit(-1);
    }
}

void readAndWrite() {

    //creat buffer to contain the content in the file.
    char buffer[MAX_BUF];
    memset(buffer, 0, MAX_BUF);

    //file descriptor
    int fd;
    /* read the file content */
    fd = open("to_srv", O_RDONLY);
    if (fd < 0) {

//        printf("cant open to_srv \n");

        write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
        exit(1);
    }

//    printf("open to_srv \n");

    //try to read from the file
    if (read(fd, buffer, MAX_BUF) < 1) {
        write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
        exit(1);
    }

//    printf("read to_srv \n");


    //calculate the expression
    char *token;      /* a strtok return value; */
    int proc_id;
    int firstNum;
    int secondNum;
    int action;
//    char firstNum[MAX_BUF];
//    char secondNum[MAX_BUF];
//    char action[MAX_BUF];

    //get the process pid
    token = strtok(buffer, "\n");
    proc_id = atoi(token);
//    printf("proc_id: %d\n",proc_id);


    //get the first num
    token = strtok(NULL, "\n");
    firstNum = atoi(token);
//    printf("proc_id: %d\n",firstNum);

//    strcpy(firstNum, token);

    //get the action
    token = strtok(NULL, "\n");
    action = atoi(token);
//    printf("proc_id: %d\n",action);

//    strcpy(action, token);

    //get the second num
    token = strtok(NULL, "\n");
    secondNum = atoi(token);
//    printf("proc_id: %d\n",secondNum);

//    strcpy(secondNum, token);

    //try to close to_srv
    if (close(fd) < 0) {
        write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
        exit(1);
    }

//    printf("close to_srv \n");


    //try to delete to_srv
    if (remove("to_srv") < 0) {
        write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
        exit(1);
    }

//    printf("delete to_srv \n");


    //save the answer of client in file
    char to_client[MAX_BUF];
    memset(to_client, 0, MAX_BUF);
    strcpy(to_client, "to_client_");
    char processPid[MAX_BUF];
    sprintf(processPid, "%d", proc_id); //Base 10
    strcat(to_client, processPid);
    //try to creat file with the client pid as name
    int output = open(to_client, O_WRONLY |O_CREAT|O_TRUNC,S_IRWXU);
    if (output < 0) {
        write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
        exit(1);
    }

    char resultString[MAX_BUF] = {0};

    //check if we made forbidden divide
    if (secondNum == 0 && action == 4) {
        if (write(output, "CANNOT_DIVIDE_BY_ZERO", strlen("CANNOT_DIVIDE_BY_ZERO")) < 0) {
            write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
            exit(1);
        }
    } else {
        int result;
        //calculate
        result = calculator(firstNum, action, secondNum);
        sprintf(resultString, "%d", result); //Base 10
        if (write(output, resultString, strlen(resultString)) < 0) {
            write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
            exit(1);
        }
    }

    //try to close to_client file
    if (close(output) < 0) {
        write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
    }

    //send signal to client
//    printf("send signal to server: %d\n", proc_id);
    if (kill(proc_id, SIGUSR2) < 0) {
        write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
        delete_to_srv();
        system("rm -f to_client_*");
        exit(-1);
    }
}

/**
 * This function handle with the signals.
 * @param signum - the signal.
 */
void sig_handler(int signum) {

    if (signum == SIGALRM) {         //signal handler for SIGALRM
        //print that no signal has received
        write(1, "The server was closed because no service request was received for the last 60 seconds\n",
              strlen("The server was closed because no service request was received for the last 60 seconds\n"));
        remove("to_srv");
        system("rm -f to_client_*");
        exit(0);
    }

    if (signum == SIGUSR1) {         // signal handler for SIGINT
//        printf("server received data from client \n");
        pid_t pid = fork(); //fork()
        if (pid == -1) {
            //fork failed
            write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
            exit(1);
        } else if (pid == 0) {

//            printf("server in child\n");


            // child
            readAndWrite(); // read from to+srv and write to_client_x
            exit(0);
        } else {
            // parent
            // Register signal handler for SIGALRM
            signal(SIGALRM, sig_handler);
            // Register signal handler for SIGINT
            signal(SIGUSR1, sig_handler); /* handles signal from other client */
            return;
        }
    }
}

/**
 * This is the main function.
 * @param argc = count how many args receive.
 * @param argv - the args.
 * @return
 */
int main(int argc, char **argv) {

    //delete to_srv file
    delete_to_srv();

    // Register signal handler for SIGALRM
    signal(SIGALRM, sig_handler);
    // Register signal handler for SIGINT
    signal(SIGUSR1, sig_handler); /* handles signal from other client */

    while (1) {
        // wait for a signal for 60 seconds
        alarm(60);
        pause();
    }
    return 0;
}