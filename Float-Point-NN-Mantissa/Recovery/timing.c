#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "highestcorr.h"
#define FN  "data5000.bin"
#define YN  "this.npy"

unsigned int   int32LE(unsigned char val[4]) {
    return val[0] | (val[1] << 8) | (val[2] << 16) | (val[3] << 24);
}

cr  inCPA(int bitloc) {
    cr      local, global;
    if(bitloc < 0 || bitloc > 3) { global.maxcorr = 0; global.maxloc = 0; global.maxwt = 0; return global; }

    FILE    *RFP, *YFP, *WFP;
    float  *corr;
    float   *cutX, *cutY;
    float   **data;
    float   *inY;
    int     trNum       = 5000;
    int     trLen       = 24000;
    int     wt, loc;
    unsigned char in8[4];
    unsigned int  in32;
    
    
    if((RFP = fopen(FN, "rt")) == NULL) puts("MAIN :: TRACE FILE IS NOT DETECTED");
    if((YFP = fopen(YN, "rt")) == NULL) puts("MAIN :: MODEL FILE IS NOT DETECTED");

    data = (float**)calloc(sizeof(float*), trNum);
    for(int i = 0 ; i < trNum ; i++)
        data[i] = (float*)calloc(sizeof(float), trLen);

    for(int i = 0 ; i < trNum ; i++)
        fread(data[i], sizeof(float), trLen, RFP);
    fclose(RFP);
    
    inY  = (float*)calloc(sizeof(float), trNum * 3);
    cutX = (float*)calloc(sizeof(float), trNum);
    cutY = (float*)calloc(sizeof(float), trNum);
    corr = (float*)calloc(sizeof(double), trLen);

    init(&local, 0);
    for(int i = 0 ; i < trNum ; i++) {
        for(int j = 0 ; j < bitloc + 1 ; j++)
            fread(&in8, 1, sizeof(unsigned int), YFP);
        in32 = int32LE(in8);    
        for(int k = 0 ; k < 32 ; k++)   
            cutY[i] += (in32 >> k) & 0b1;
        for(int j = 0 ; j < 3 - bitloc ; j++)
            fread(&in8, 1, sizeof(unsigned int), YFP);
    }
    fclose(YFP);

    for(loc = 0 ; loc < trLen ; loc++) {
        for(int i = 0 ; i < trNum ; i++)
            cutX[i] = data[i][loc];

        corr[loc] = correlation(cutX, cutY, trNum);
        if(fabs(corr[loc]) > local.maxcorr) {
            local.maxcorr = fabs(corr[loc]);
            local.maxloc  = loc;
            local.maxwt   = wt;
        }
    }
    if(bitloc == 0)        WFP = fopen("corr.bin", "w+b");
    else if(bitloc == 1)   WFP = fopen("corr.bin", "a+b");
    else if(bitloc == 2)   WFP = fopen("corr.bin", "a+b");

    fwrite(corr, sizeof(float), trLen, WFP);
    fclose(WFP);
    //printf("[%lf], [%d]\n", local.maxcorr, local.maxloc);

    for(int i = 0 ; i < trNum ; i++)
        free(data[i]);
    free(data);
    free(corr);
    free(cutX);
    free(cutY);
    free(inY);

    return local;
}

int main() {
    puts("\t:: MMTB Experiment ::\t");
    cr    in[3];

    for(int i = 0 ; i < 3 ; i++) {
        in[i] = inCPA(i);
        //if(in[i].maxcorr < 0.8) { printf("MAIN :: CORR PEAK (-> %d) IS NOT UPPER THAN 0.8 (%.2f%%)\n", i, in[i].maxcorr * 100); }
        printf("INPUT LOCATION(%d)\t[%d] (%.2f%%)\n", i, in[i].maxloc, in[i].maxcorr * 100);
    }
}