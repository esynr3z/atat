/**
  ******************************************************************************
  * @file    main.c
  *
  * @brief   Test for AT-AT lib
  *
  * @author  esynr3z
  * @date    13.09.2016
  *
  ******************************************************************************
  */

#include "atat.h"

ATAT_HandlerStat_TypeDef test_exec_handler()
{
    printf("Function test_exec_handler() was called!\n");

    return ATAT_HandlerStat_OK;
}

int main(void)
{
    ATAT_Init();
    ATAT_ExecHandlerInit(ATAT_CmdID_Test, &test_exec_handler);

    char frame0[] = "AT\r\n";
    char frame1[] = "AT+TEST!\r\n";
    char frame2[] = "AT+TEST?\r\n";
    char frame3[] = "AT+TEST=new_test_value\r\n";
    char frame4[] = "AT+TEST\r\n";
    char frame5[] = "AT+TEST?\r\n";

    /* Simulate byte-by-byte transfers from master.
       For example, in UART RX interrupt handler. */

    printf(">%s", frame0);
    for(uint32_t i=0; i<sizeof(frame0)-1; i++)
    {
        ATAT_GetSymbol(frame0[i]);
    }

    printf(">%s", frame1);
    for(uint32_t i=0; i<sizeof(frame1)-1; i++)
    {
        ATAT_GetSymbol(frame1[i]);
    }

    printf(">%s", frame2);
    for(uint32_t i=0; i<sizeof(frame2)-1; i++)
    {
        ATAT_GetSymbol(frame2[i]);
    }

    printf(">%s", frame3);
    for(uint32_t i=0; i<sizeof(frame3)-1; i++)
    {
        ATAT_GetSymbol(frame3[i]);
    }

    printf(">%s", frame4);
    for(uint32_t i=0; i<sizeof(frame4)-1; i++)
    {
        ATAT_GetSymbol(frame4[i]);
    }

    printf(">%s", frame5);
    for(uint32_t i=0; i<sizeof(frame5)-1; i++)
    {
        ATAT_GetSymbol(frame5[i]);
    }

    return 0;
}
