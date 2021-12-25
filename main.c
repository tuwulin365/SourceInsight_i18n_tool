#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boyer_moore.h"

typedef struct
{
    int iCurrentLine;
    unsigned char ucEn[4096];
    int iEnLen;
    unsigned char ucNew[4096];
    int iNewLen;
}StringPair;

unsigned char* GetStringLine(unsigned char *pBuf, int iInLen, unsigned char *pOut, int *iOutLen, int *iCurrentLine)
{
    int i;

    for (i = 0; i < iInLen; i++)
    {
        if ((pBuf[i] == 0x0D) && (pBuf[i+1] == 0x0A))
        {
            break;
        }
    }

    if (i < iInLen)
    {
        *iCurrentLine += 1;
        if (0 == memcmp(pBuf, "//", 2))
        {
            return GetStringLine(pBuf+i+2, iInLen-i-2, pOut, iOutLen, iCurrentLine);
        }
        else
        {
            memcpy(pOut, pBuf, i);
            *iOutLen = i;
            return pBuf+i+2;
        }
    }

    return NULL;
}

unsigned char* GetStringPair(unsigned char *pBuf, int iInLen, StringPair *pstString)
{
    unsigned char *pTmp;
    unsigned char uczBuf[1024];
    int i;

    pTmp = GetStringLine(pBuf, iInLen, pstString->ucEn, &pstString->iEnLen, &pstString->iCurrentLine);
    if (pTmp)
    {
        pTmp = GetStringLine(pTmp, iInLen-(pTmp-pBuf), pstString->ucNew, &pstString->iNewLen, &pstString->iCurrentLine);
        if (pTmp)
        {
            pTmp = GetStringLine(pTmp, iInLen-(pTmp-pBuf), uczBuf, &i, &pstString->iCurrentLine);
        }
    }

    return pTmp ? pTmp : NULL;
}

unsigned char* FindString(unsigned char *pBuf, int iInLen, unsigned char *pStr, int iStrLen)
{
#if 0
    unsigned char *pTmp;
    int iLen;

    for (iLen = iInLen; iLen; iLen--,pBuf++)
    {
        pTmp = memchr(pBuf, pStr[0], iLen);
        if (pTmp)
        {
            int i;
            for (i= 0; i < iStrLen; i++)
            {
                if (pTmp[i] != pStr[i])
                {
                    break;
                }
            }
            if ((i == iStrLen) && (pTmp[-1] == 0) && (pTmp[iStrLen] == 0))
            {
                return pTmp;
            }
        }
    }

    return NULL;
#else
    unsigned char *pTmp;
    unsigned char *pStart = pBuf;
    int iStart = iInLen;

    if (!iStrLen || !iInLen)
    {
        return NULL;
    }

    while (1)
    {
        pTmp = BoyerMoore(pStr, iStrLen, pStart, iStart);
        if (!pTmp)
        {
            break;
        }

        if ((pTmp[-1] == 0) && (pTmp[iStrLen] == 0))
        {
            break;
        }
        else
        {
            pStart = pTmp + iStrLen;
            iStart = iInLen - ((unsigned int)pStart - (unsigned int)pBuf);
        }
    }

    return pTmp;
#endif
}

int IsStringFit(unsigned char *pBuf, StringPair *pSt, int *iMaxChar)
{
    int i;

    if (pSt->iEnLen >= pSt->iNewLen)
    {
        return 1;
    }

    i = pSt->iEnLen;
    *iMaxChar = i;
    while (1)
    {
        if ((pBuf[i] == 0) && (pBuf[i+1] == 0))
        {
            i++;
            *iMaxChar = i;
            if (i >= pSt->iNewLen)
            {
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
}

int main(int iCnt, char *pParam[])
{
    FILE *pfLngFile;
    FILE *pfExe;
    unsigned char *pucLng;
    unsigned char *pucExe;
    StringPair stString;
    int i,j;
    unsigned char *pTmpIn;
    unsigned char *pTmpOut;

    unsigned char *pExeTmpIn;
    unsigned char *pExeTmpOut;

    char czNewFileName[128];
    char czOldFileName[128];

    printf("sourceinsight4 i18n tool V1.00\ntuwulin365@126.com  2020-04-07\n");
    printf("usage: i18n_tool.exe si.exe chs.lng\n\n");

    if (iCnt != 3)
    {
        printf("param err.\n");
        return -1;
    }

    pfLngFile = fopen(pParam[2], "rb");
    if (!pfLngFile)
    {
        printf("open lng file error: %s.\n", pParam[2]);
        return -1;
    }

    pfExe = fopen(pParam[1], "rb");
    if (!pfExe)
    {
        printf("open org exe file error: %s.\n", pParam[1]);
        return -1;
    }

    pucLng = malloc(1*1024*1024);
    pucExe = malloc(5*1024*1024);
    if (!pucExe || !pucLng)
    {
        printf("malloc error.\n");
        return -1;
    }

    i = fread(pucLng, 1, 1*1024*1024, pfLngFile);
    j = fread(pucExe, 1, 5*1024*1024, pfExe);
    fclose(pfLngFile);
    fclose(pfExe);

    memset(&stString, 0, sizeof(StringPair));
    pTmpIn = pucLng;
    pTmpOut = pTmpIn;
    while (pTmpOut)
    {
        int iTmp;

        iTmp = stString.iCurrentLine;
        memset(&stString, 0, sizeof(StringPair));
        stString.iCurrentLine = iTmp;

        pTmpOut = GetStringPair(pTmpIn, i, &stString);
        if (pTmpOut)
        {
            i = i - (pTmpOut-pTmpIn);
            pTmpIn = pTmpOut;
            //printf("%s %d\n", stString.ucEn, stString.iEnLen);
            //printf("%s %d\n", stString.ucNew, stString.iNewLen);


            pExeTmpOut = FindString(pucExe, j, stString.ucEn, stString.iEnLen);
            if (pExeTmpOut)
            {
                int iMaxChar = 0;
                if (IsStringFit(pExeTmpOut, &stString, &iMaxChar))
                {
                    //printf("%s\n", pExeTmpOut);
                    memset(pExeTmpOut, 0, stString.iEnLen);
                    memcpy(pExeTmpOut, stString.ucNew, stString.iNewLen);
                }
                else
                {
                    printf("string not fit : %d : %s\n", stString.iCurrentLine-2, stString.ucEn);
                    printf("old max char : %d\n", iMaxChar);
                    printf("new char : %d\n\n", stString.iNewLen);
                }
            }
            else
            {
                printf("string not find : %d : %s\n\n", stString.iCurrentLine-2, stString.ucEn);
            }
        }
    }


    strcpy(czOldFileName, pParam[1]);
    czOldFileName[strlen(czOldFileName)-4] = 0;

    sprintf(czNewFileName, "%s_new.exe", czOldFileName);
    pfExe = fopen(czNewFileName, "wb");
    if (!pfExe)
    {
        printf("new file error.\n");
        return -1;
    }

    fwrite(pucExe, 1, j, pfExe);
    fclose(pfExe);

    printf("DONE !\n");
    return 0;
}
