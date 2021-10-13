/* instruction-level simulator */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
void dec2Bi(char *);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    //=========================================================================
    //translate decimal instruction to binary
    char memBin[state.numMemory][33];
    char temp[MAXLINELENGTH];
    for(int i=0;i<state.numMemory;i++){
        sprintf(temp,"%d",state.mem[i]);
        dec2Bi(temp);
        strcpy(memBin[i],temp);
    }

    //set reg 0-7 and program counter to zero
    state.pc = 0;
    for(int i=0;i<8;i++){
        state.reg[i] = 0;
    }

    //loop read the instruction until find HALT instruction
    int numExeInst = 0;
    // while(1){

    // }

    //print final state before exit program
    /*printf("total of %d instructions executed\n",numExeInst);
    printf("final state of machine:\n");
    printState(&state);*/

    fclose(filePtr);
    //=========================================================================

    return(0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
	for (i=0; i<statePtr->numMemory; i++) {
	    printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}
    printf("end state\n");
}

//=========================================================================
void dec2Bi(char *num){
    int n = 0;
    sscanf(num, "%d", &n);

    strcpy(num,"");
    for (int i = 32-1; i >= 0; i--) {
        int k = n >> i;
        if (k & 1)
            strcat(num,"1");
        else{
            strcat(num,"0");
        }
    }
}