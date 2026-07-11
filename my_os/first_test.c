/*
 * first_test.c — 你進入 NachOS 的第一支使用者程式
 *
 * 目的：確認「交叉編譯 → coff2noff → 在 NachOS 上跑」整條流程是通的，
 *       同時預告 MP1 你要親手實作的那幾個系統呼叫。
 *
 * 怎麼用：
 *   1. 把這支檔案複製到你乾淨底的 code/test/ 底下。
 *   2. 編輯 test/Makefile，仿照 halt / add 的規則，加上 first_test。
 *   3. cd test && make        # 產出 NachOS 可執行檔 first_test
 *   4. cd ../build.linux && ./nachos -e ../test/first_test   (執行參數是 -e，不是 -x)
 *
 * 一開始 PrintInt / Halt 這種「已內建」的 syscall 會動；
 * 而 Open/Write/Read/Close 這幾個要等你在 MP1 的 exception.cc 實作後才會動。
 */

#include "syscall.h"   /* NachOS 提供的使用者端 syscall 介面 */

int
main(void)
{
    /* --- 這段現在就能跑（syscall 已內建）--- */
    int i;
    int sum = 0;
    for (i = 1; i <= 10; i++)
        sum += i;
    PrintInt(sum);          /* 應印出 55，證明程式有被載入並執行 */

    /* --- 下面這段是 MP1 的目標：實作完 file syscall 後才會成功 --- */
    /* 提示：先在 exception.cc 的 ExceptionHandler 裡接好 SC_Create/SC_Open/
     *       SC_Write/SC_Read/SC_Close，再回來把下面註解打開測試。          */
    /*
    OpenFileId fd;
    char msg[] = "hello nachos\n";
    char buf[16];

    Create("test_output.txt");
    fd = Open("test_output.txt");
    Write(msg, 13, fd);          // 寫入檔案
    Close(fd);

    fd = Open("test_output.txt");
    Read(buf, 13, fd);           // 讀回來
    Close(fd);
    */

    Halt();                 /* 乾淨地結束 NachOS */
    return 0;
}
