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
    int result = 0;
    printfTitle("RyanJson 示例程序");
    RyanJsonExample();

    printfTitle("RyanJson 接口测试程序");
    result = RyanJsonTest();
    if (0 != result)
    {
        printf("%s:%d RyanJsonTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    printfTitle("RyanJson / cJSON / yyjson RFC8259标准测试");
    result = RFC8259JsonTest();
    if (0 != result)
    {
        printf("%s:%d RFC8259JsonTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    printfTitle("RyanJson / cJSON / yyjson 内存对比程序");
    result = RyanJsonMemoryFootprintTest();

    displayMem();
    return 0;
}