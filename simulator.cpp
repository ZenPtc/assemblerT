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
int bi2Dec(char [],int);
void ADD(int,int,int,int *);
void NAND(int,int,int,int *);
void LW(int,int,int,int *,int *);
void SW(int,int,int,int *,int *);
void BEQ(int,int,int,int *,int *);
void JALR(int,int,int *,int *);

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
    char opcode[4],regA[4],regB[4],offset[17],destReg[4];
    int numRegA,numRegB,numOffset,numDestReg;
    while(1){
        //printState before execute
        printState(&state);
        numExeInst++;

        //read opcode
        memcpy(opcode,&memBin[state.pc][7],3);
        if(!strcmp(opcode, "110")){         //halt
            printf("machine halted\n");
            state.pc++;
            break;
        }else if(!strcmp(opcode, "111")){   //noop
            continue;
        }

        //read regA
        memcpy(regA,&memBin[state.pc][10],3);
        numRegA = bi2Dec(regA,0);

        //read regB
        memcpy(regB,&memBin[state.pc][13],3);
        numRegB = bi2Dec(regB,0);

        //read other field
        memcpy(offset,&memBin[state.pc][16],16);
        memcpy(destReg,&memBin[state.pc][29],3);
        numOffset  = bi2Dec(offset,1);      //use 2's complement
        numDestReg = bi2Dec(destReg,0);

        //do instruction
        if(!strcmp(opcode, "000")){             //add
            ADD(numRegA,numRegB,numDestReg,state.reg);
        }else if(!strcmp(opcode, "001")){       //nand
            NAND(numRegA,numRegB,numDestReg,state.reg);
        }else if(!strcmp(opcode, "010")){       //lw
            LW(numRegA,numRegB,numOffset,state.reg,state.mem);
        }else if(!strcmp(opcode, "011")){       //sw
            SW(numRegA,numRegB,numOffset,state.reg,state.mem);
        }else if(!strcmp(opcode, "100")){       //beq
            BEQ(numRegA,numRegB,numOffset,state.reg,&state.pc);
        }else if(!strcmp(opcode, "101")){       //jalr
            JALR(numRegA,numRegB,state.reg,&state.pc);
            continue;
        }

        state.pc++;
    }

    //print final state before exit program
    printf("total of %d instructions executed\n",numExeInst);
    printf("final state of machine:\n");
    printState(&state);

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

//twoComp -> 0 is not use ,1 is use 2's complement
int bi2Dec(char biCode[],int twoComp){
    int dec_value = 0;
    int base = 1;
    int len = strlen(biCode);
    int neg = 0;  //if 1 it's means negative number
    char firstBit[2];
    strcpy(firstBit+1,"\0");
    memcpy(firstBit,&biCode[0],1);

    if(!strcmp(firstBit,"1") && twoComp==1){
        neg = 1;
    }

    for (int i = len - 1; i >= 0; i--) {
        if(twoComp==1 && neg==1){
            if(biCode[i] == '0'){
                dec_value += base;
            }
        }else{
            if(biCode[i] == '1'){
                dec_value += base;
            }
        }
        base = base * 2;
    }

    if(neg==1){
        dec_value += 1;
        dec_value *= -1;
    }

    return dec_value;
}

void ADD(int regA,int regB,int destReg,int *regArr){
    int valueA,valueB;

    valueA = *(regArr+regA);
    valueB = *(regArr+regB);

    *(regArr+destReg) = valueA + valueB;
}

void NAND(int regA,int regB,int destReg,int *regArr){
    int valueA,valueB;

    valueA = *(regArr+regA);
    valueB = *(regArr+regB);

    *(regArr+destReg) = ~(valueA & valueB);
}

void LW(int regA,int regB,int offset,int *regArr,int *memArr){
    int valueA,memAddr,valueM;

    valueA = *(regArr+regA);
    memAddr = valueA + offset;
    valueM = *(memArr+memAddr);

    *(regArr+regB) = valueM;
}

void SW(int regA,int regB,int offset,int *regArr,int *memArr){
    int valueA,valueB,memAddr;

    valueA = *(regArr+regA);
    valueB = *(regArr+regB);
    memAddr = valueA + offset;

    *(memArr+memAddr) = valueB;
}

void BEQ(int regA,int regB,int offset,int *regArr,int *pc){
    int valueA,valueB;

    valueA = *(regArr+regA);
    valueB = *(regArr+regB);

    if(valueA == valueB){
        *pc = (*pc)+offset;
    }
}

void JALR(int regA,int regB,int *regArr,int *pc){
    int valueA;
    valueA = *(regArr+regA);

    *(regArr+regB) = (*pc)+1;
    *pc = valueA;
}