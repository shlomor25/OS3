/**
 * Name: Shlomo Rabinovich
 * ID: 308432517
 */
#define _POSIX_SOURCE
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define CONF_SIZE 150
#define PATH_SIZE 150
#define TRUE 1
#define FALSE 0
#define ERROR -1


//function declaration
void PathCreator(char *buffer, char *path, char *dir);

void FindCFile(char *cFilePath, char *dirPath);

int IsCFile(char *name);

int IsLegalDir(char *path, char *name);

void WriteLine(char *result, char *name, int grade, int fd);

int compileFailed(char *cFilePath);

int run(char *inPath);

int compare(char *path, char *outPath);

void error(){
    write(2, "ERROR IN SYSTEM CALL", sizeof("ERROR IN SYSTEM CALL"));
    exit(-1);
}

/**
 * main func
 * @param argc
 * @param argv configuration file
 * @return
 */
int main(int argc, char *argv[]) {
    int fdConf, sizeConf, fdResult, grade;
    char buffer[CONF_SIZE];
    char *dirPath, *inPath, *outPath;
    char subDirPath[PATH_SIZE], cFilePath[PATH_SIZE], result[CONF_SIZE];
    const char delim[2] = {'\n', '\r'};
    DIR *dir;
    struct dirent *dirent;

    // open conf.txt
    fdConf = open(argv[1], O_RDONLY);
    if (fdConf < 0) error();

    sizeConf = (int)read(fdConf, buffer, CONF_SIZE);
    if (sizeConf < 0) error();

    //split configuration to 3 paths
    dirPath = strtok(buffer, delim);
    inPath = strtok(NULL, delim);
    outPath = strtok(NULL, delim);

    //open folder of student folders
    if ((dir = opendir(dirPath)) == NULL) error();

    //create result.csv
    if ((fdResult = open("result.csv",O_CREAT | O_RDWR | O_TRUNC, 0666)) < 0) error();

    //loop on all directories
    while ((dirent = readdir(dir)) != NULL) {
        memset(subDirPath, 0, PATH_SIZE);
        memset(cFilePath, 0, PATH_SIZE);
        memset(result,0,CONF_SIZE);
        //create directory path
        PathCreator(subDirPath, dirPath, dirent->d_name);
        if (IsLegalDir(subDirPath, dirent->d_name)) {
            // find executable file
            FindCFile(cFilePath, subDirPath);
            if (strstr(cFilePath, "/")) {
                //try to compile, if compile failed, grade is '20'
                if (compileFailed(cFilePath)) {
                    grade = 20;
                } else {
                    //for timeout - grade is '40'
                    if (run(inPath)){
                        grade = 40;
                        //compare output to correct output
                    } else {
                        grade = compare(cFilePath, outPath);
                    }
                }
                //in case there is no c file - grade is '0'
            } else {
                grade = 0;
            }
            //write to result.csv
            WriteLine(result, dirent->d_name, grade, fdResult);
        }
    }
    //close directoy
    if (closedir(dir) < 0) error();
    // close files
    if (close(fdConf) < 0 || close(fdResult) < 0) error();
    // remove temp files
    if (unlink("student.out") < 0 || unlink("studentOutput"))  error();
    return 0;
}


/**
 * compare output files
 * @param path first file
 * @param outPath second file
 * @return grade
 */
int compare(char *path, char *outPath) {
    //variable declaration
    pid_t pid;
    int status, compare = -1;
    //initialize buffer
    memset(path,0,PATH_SIZE);

    //run ex31.c
    if((pid = fork()) == 0){
        execlp("./comp.out", "comp.out", "studentOutput", outPath, NULL);
    }
    //validation
    if (pid < 0) error();
    if ((waitpid(pid,&status,0)) < 0) error();
    //check return value from comp.out
    if (WIFEXITED(status)){
        compare = WEXITSTATUS(status);
    }
    //write result to buffer
    switch (compare){
        case 1:
            strcpy(path,"GREAT_JOB");
            return 100;
        case 2:
            strcpy(path,"SIMILLAR_OUTPUT");
            return 80;
        case 3:
            strcpy(path, "BAD_OUTPUT");
            return 60;
        default:
            break;
    }
    // error
    return ERROR;
}


/************************************************************************
 * Function name: Run
 * The input: input file path and buffer
 * The output: no output
 * The function operation: runs student's program and checks for timeout
*************************************************************************/
int run(char *inPath) {
    pid_t pid;
    int timeout, status, fdIn, fdOut, timeOutFlag = 0;

    fdOut = open("studentOutput", O_CREAT | O_RDWR | O_TRUNC, 0666);
    fdIn = open(inPath, O_RDONLY);
    if (fdOut < 0 || fdIn < 0)  error();
    pid = fork();
    if (pid < 0) error();
    if (pid == 0){
        if((dup2(fdIn,0)) < 0) error();
        if((dup2(fdOut, 1)) < 0) error();
        //run student's file.out
        execlp("./student.out", "student.out", NULL);
    }
    //timeout
    timeout = 5;
    while (timeout != 0){
        timeout--;
        sleep(1);
        //if process returned
        if ((waitpid(pid, &status,WNOHANG)) != 0){
            break;
        }
    }
    // stop process after 5 sec
    if (timeout == 0){
        kill(pid,SIGSTOP);
        timeOutFlag = 1;
    }
    if ((close(fdOut) < 0) || close(fdIn) < 0) error();
    return timeOutFlag;

}


/**
 * compile Failde
 * @param cFilePath
 * @return 1 for comilation error
 */
int compileFailed(char *cFilePath) {
    int status, compileFlag = 0;
    pid_t pid = fork();
    if (pid < 0) error();
    //compile student's c file
    if (pid == 0){
        execlp("gcc", "gcc", cFilePath, "-o", "student.out", NULL);
    }
    //check the status
    if ((wait(&status)) < 0) error();
    if (WIFEXITED(status)){
        compileFlag = WEXITSTATUS(status);
    }
    return compileFlag;
}


/**
 * write to result.csv
 * @param result file
 * @param name dir name
 * @param grade
 * @param fd write param
 */
void WriteLine(char *result, char *name, int grade, int fd) {
    // get grade
    strcat(result, name);
    switch (grade){
        case 100:
            strcat(result, ",100,GREAT_JOB\n");
            break;
        case 80:
            strcat(result, ",80,SIMILAR_OUTPUT\n");
            break;
        case 60:
            strcat(result, ",60,BAD_OUTPUT\n");
            break;
        case 40:
            strcat(result, ",40,TIMEOUT\n");
            break;
        case 20:
            strcat(result, ",20,COMPILATION_ERROR\n");
            break;
        case 0:
            strcat(result, ",0,NO_C_FILE\n");
            break;
        default:
            break;

    }

    if (write(fd,result,strlen(result)) < 0) error();
}


/**
 * find c file in directory
 * @param cFilePath
 * @param dirPath
 */
void FindCFile(char *cFilePath, char *dirPath) {
    int count = 0;
    DIR *dir;
    struct dirent *dirent;
    char newPath[PATH_SIZE], subDirPath[PATH_SIZE];
    //open directory
    if ((dir = opendir(dirPath)) == NULL)  error();
    //loop on all files and directories
    while ((dirent = readdir(dir)) != NULL) {
        //create new path
        PathCreator(newPath, dirPath, dirent->d_name);
        //for directory - continue with new path
        if (IsLegalDir(newPath, dirent->d_name)) {
            strcpy(subDirPath, newPath);
            count++;
            // for c file, save its path
        } else {
            if (IsCFile(dirent->d_name)) {
                strcpy(cFilePath, newPath);
                return;
            }
        }
        memset(newPath,0,strlen(newPath));
    }
    if (closedir(dir) < 0) error();

    if (count != 1) {
        return;
    }
    //if found one directory and no c file, send recurrsively
    return FindCFile(cFilePath, subDirPath);
}


/**
 * Check for legal directory
 * @param path
 * @param name dir name
 * @return true for dir and not . or ..
 */
int IsLegalDir(char *path, char *name) {
    struct stat statBuf;
    int statSuccess = stat(path, &statBuf);
    if (statSuccess == ERROR) error();
    // true for directory and not "." or ".."
    if (S_ISDIR(statBuf.st_mode) && strcmp(name, ".") && strcmp(name, "..")) {
        return TRUE;
    }
    return FALSE;
}


/**
 * Check if extension is ".c"
 * @param name file name
 * @return 1 for true
 */
int IsCFile(char *name) {
    char *extension;
    extension = strrchr(name, '.');
    //check if there is '.' in the name of the file
    if (extension != NULL && !strcmp(extension, ".c")) {
        return TRUE;

    }
    return FALSE;
}

/**
 * create current path
 * @param buffer
 * @param path
 * @param dir name
 */
void PathCreator(char *buffer, char *path, char *dir) {
    //append dir name to path
    strcpy(buffer, path);
    strcat(buffer, "/");
    strcat(buffer, dir);
}