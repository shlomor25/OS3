/*
 * Name: Shlomo Rabinovich
 * ID: 308432517
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>


/* Declarations */
int isCapital(char c);
int isSmall(char c);
int similar(char c, char d);
void error();


/**
 * Main
 * @param argc
 * @param argv
 * @return 1 for equal, 2 for similar, ow 3
 */
int main(int argc, char* argv[]) {
    int returnVal = 1;
    int first, second;
    ssize_t sizeC=1, sizeD=1;
    char c, d;

    if (argc != 3){
        fprintf(stderr, "input error\n");
    }

    first = open(argv[1],O_RDONLY);
    second = open(argv[2],O_RDONLY);
    if (first < 0 || second < 0) error();
    while (sizeC == 1 && sizeD == 1){
        //read char from 2 files
        sizeC = read(first,&c,1);
        sizeD = read(second,&d,1);
        if (sizeD < 0 || sizeC < 0) error();
        //if not equal
        if (c != d){
            returnVal = 2;
            //ignore whitespace
            while ((c == ' ' || c == '\n') && sizeC == 1){
                sizeC = read(first,&c,1);
                if (sizeC < 0) error();
            }
            while ((d == ' ' || d == '\n') && sizeD == 1) {
                sizeD = read(second,&d,1);
                if (sizeD < 0) error();

            }
            if (similar(c, d) == 1){
                continue;
            } else return 3;

        }
    }
    if (close(first) < 0 || close(second) < 0) error();

    return returnVal;
}

void error(){
    write(2, "ERROR IN SYSTEM CALL", sizeof("ERROR IN SYSTEM CALL"));
    exit(-1);
}

/**
 * is capital
 * @param c letter
 * @return true for capital letter
 */
int isCapital(char c){
    if (c >= 'A' && c <= 'Z') {
        return 1;
    } else {
        return 0;
    }
}

/**
 * is Small
 * @param c letter
 * @return true for small letter
 */
int isSmall(char c) {
    if (c >= 'a' && c <= 'z') {
        return 1;
    } else {
        return 0;
    }
}

/**
 * check for similar
 * @param c char
 * @param d char
 * @return 1 for similar, otherwise 0, -1 for error
 */
int similar(char c, char d) {
    if (isCapital(c) && isCapital(d)) {
        return (c == d);
    }
    if (isSmall(c) && isSmall(d)) {
        return (c == d);
    }

    if (isCapital(c) && isSmall(d)){
        c += ('a' - 'A');
        return (c == d);

    }
    if (isCapital(d) && isSmall(c)){
        d += ('a' - 'A');
        return (c == d);
    }
    //error
    return -1;
}
