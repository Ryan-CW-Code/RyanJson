#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "RyanJson.h"
#include "valloc.h"

void printfTitle(char *title)
{
    printf("\r\n");
    printf("\r\n");
    printf("*****************************************************************************\r\n");
    printf("*************************** %s **************************\r\n", title);
    printf("*****************************************************************************\r\n");
}

extern int RyanJsonExample();
extern int RyanJsonTest();
extern int RyanJsonMemoryFootprintTest();
extern int RFC8259JsonTest();

int main(void)
{
    printfTitle("RyanJson 示例程序");
    RyanJsonExample();

    printfTitle("RyanJson 接口测试程序");
    RyanJsonTest();

    printfTitle("RyanJson / cJSON / yyjson 内存对比程序");
    RyanJsonMemoryFootprintTest();

    printfTitle("RyanJson / cJSON / yyjson RFC8259标准测试");
    RFC8259JsonTest();

    displayMem();
    return 0;
}