/* Assembler code fragment */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

struct Label
{
    char name[6] = "";
    int lineNum = -1;
};
Label labels[MAXLINELENGTH];

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int findLineOfLabel(Label[],char[]);
void dec2Bi(char *,int,int,int);
int bi2Dec(char *);

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }
    //=========================================================================
    //cal symbolic  address
    int lineCount = 0;
    int labelCount = 0;
    while(1){
        if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
            break;
        }

        //check is have the duplicate label name
        int  n = labelCount;
        while(n>=0){
            if(!strcmp(label,labels[n].name) && strcmp(label,"")){
                printf("The label is already exist.");
                exit(1);
            }
            n--;
        }

        if(strcmp(label,"")){
            strcpy(labels[labelCount].name,label);
            labels[labelCount].lineNum = lineCount;
            labelCount++;
        }
        lineCount++;
    }

    //start translate to machine code
    rewind(inFilePtr);
    int currentLine = 0;
    while(1){
        int type = -1;  //R=0, I=1, J=2, O=3
        currentLine++;
        char mCode[33] = "0000000";
        if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
            break;
        }else{
            //opcode
            if (!strcmp(opcode, "add")) {
                strcat(mCode,"000");
                type = 0;
            }else if (!strcmp(opcode, "nand")) {
                strcat(mCode,"001");
                type = 0;
            }else if (!strcmp(opcode, "lw")) {
                strcat(mCode,"010");
                type = 1;
            }else if (!strcmp(opcode, "sw")) {
                strcat(mCode,"011");
                type = 1;
            }else if (!strcmp(opcode, "beq")) {
                strcat(mCode,"100");
                type = 1;
            }else if (!strcmp(opcode, "jalr")) {
                strcat(mCode,"101");
                type = 2;
            }else if (!strcmp(opcode, "halt")) {
                strcat(mCode,"110");
                strcat(mCode,"0000000000000000000000");
                type = 3;
            }else if (!strcmp(opcode, "noop")) {
                strcat(mCode,"111");
                strcat(mCode,"0000000000000000000000");
                type = 3;
            }else if (!strcmp(opcode, ".fill")) {
                if(isNumber(arg0)){
                    strcpy(mCode,arg0);
                }else{
                    int num = findLineOfLabel(labels,arg0);
                    char num_char[33];
                    sprintf(num_char,"%d",num);
                    strcpy(mCode,num_char);
                }
                fprintf(outFilePtr,"%s\n",mCode);
                continue;
            }

            //arg0
            if(type<3){
                dec2Bi(arg0,3,currentLine,-1);
                strcat(mCode,arg0);
            }

            //arg1
            if(type<3){
                dec2Bi(arg1,3,currentLine,-1);
                strcat(mCode,arg1);
            }

            //arg2
            if(type<=2){
                if(type==0){
                    strcat(mCode,"0000000000000");
                    dec2Bi(arg2,3,currentLine,-1);
                    strcat(mCode,arg2);
                }else if(type==2){
                    strcat(mCode,"0000000000000000");
                }else if(type==1){      //do 16bit 2's complement
                    if (!strcmp(opcode, "beq")){
                        dec2Bi(arg2,16,currentLine,1);
                    }else{
                        dec2Bi(arg2,16,currentLine,0);
                    }
                    strcat(mCode,arg2);
                }
            }
            int dec_value = bi2Dec(mCode);
            fprintf(outFilePtr,"%d\n",dec_value);
        }
    }

    fclose(inFilePtr);
    fclose(outFilePtr);

    //=========================================================================
    // /* here is an example for how to use readAndParse to read a line from
    //     inFilePtr */
    // if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
    //     /* reached end of file */
    // }

    // /* this is how to rewind the file ptr so that you start reading from the
    //     beginning of the file */
    // rewind(inFilePtr);

    // /* after doing a readAndParse, you may want to do the following to test the
    //     opcode */
    // if (!strcmp(opcode, "add")) {
    //     /* do whatever you need to do for opcode "add" */
    // }

    return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

//=========================================================================
int findLineOfLabel(Label labels[],char nameL[]){
    int n=0;
    while(1){
        if(!strcmp(labels[n].name,"")){
            printf("The label is undefine.\n");
            exit(1);
            break;
        }
        if(!strcmp(labels[n].name, nameL)){
            return labels[n].lineNum;
        }
        n++;
    }
    return -1;
}

//inst -1 = not I type, 0=lw,sw, 1=beq
void dec2Bi(char * numC,int nbit,int lineCount,int inst){
    int n;
    int sign=0;

    //cal offsetField
    if(isNumber(numC)){
        sscanf(numC, "%d", &n);
    }else{
        n = findLineOfLabel(labels,numC);
        if(inst==1){
            n = n - lineCount;
        }
    }

    //check if offsetField have more than 16 bits
    if((n > 32767 || n < -32768) && (inst==0 || inst==1)){
        printf("OffsetField have more than 16 bits.");
        exit(1);
    }
    //check if other argument have more than 3 bits
    if((n > 7 || n < 0) && (inst==-1)){
        printf("Argument is not the right number.");
        exit(1);
    }

    if(n<0){
        sign=1;
        n *= -1;
        n--;
    } 

    strcpy(numC,"");
    for (int i = nbit-1; i >= 0; i--) {
        int k = n >> i;
        if (k & 1)
            if(sign==0) strcat(numC,"1");
            else strcat(numC,"0");
        else{
            if(sign==0) strcat(numC,"0");
            else strcat(numC,"1");
        }
    }
}

int bi2Dec(char* biCode) {
    int dec_value = 0;
    int base = 1;
    int len = strlen(biCode);

    for (int i = len - 1; i >= 0; i--) {
        if (biCode[i] == '1')
            dec_value += base;
        base = base * 2;
    }
    return dec_value;
}