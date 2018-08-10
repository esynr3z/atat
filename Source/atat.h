/**
  ******************************************************************************
  * @file    atat.h
  *
  * @brief   AT-AT include file.
  *
  *          HOW TO USE IT?
  *
  *          - include this to your app main file
  *          - call ATAT_Init() function to init lib
  *
  * @author  esynr3z
  * @date    13.09.2016
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ATAT_H
#define __ATAT_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Exported defines ----------------------------------------------------------*/

/* Frame and bufer size configuration
 * (remember that one symbol is always for '\0') */
#define ATAT_FRAME_HEADER_SIZE      3
#define ATAT_FRAME_COMMAND_SIZE     16
#define ATAT_FRAME_MODIFIER_SIZE    2
#define ATAT_FRAME_DATA_SIZE        100
#define ATAT_BUFF_SIZE              128

/* Command strings.
 * Every command should start from "+" and have not more than 15 symbols next */
#define ATAT_CMD_STR_TEST   "+TEST"

/* NOTE: you can add new command string here ^^^ */

/* Exported types ------------------------------------------------------------*/

/* Handler status */
typedef enum {
    ATAT_HandlerStat_OK,
    ATAT_HandlerStat_ERR,
}ATAT_HandlerStat_TypeDef;

/* Command ID.
 * *_TOTAL must be the last element ALWAYS.  */
typedef enum {
    ATAT_CmdID_Test,

    /* NOTE: you can add new command ID here ^^^ */
    ATAT_CmdID_TOTAL
}ATAT_CmdID_TypeDef;

/* Exported functions prototypes ---------------------------------------------*/
void ATAT_Init();
void ATAT_GetSymbol(char Symbol);
void ATAT_ExecHandlerInit(ATAT_CmdID_TypeDef CommandID, ATAT_HandlerStat_TypeDef (*UserExecHandler)(void));
void ATAT_SetHandlerInit(ATAT_CmdID_TypeDef CommandID, ATAT_HandlerStat_TypeDef (*UserSetHandler)(char *Data));
void ATAT_GetHandlerInit(ATAT_CmdID_TypeDef CommandID, ATAT_HandlerStat_TypeDef (*UserGetHandler)(char *Data));

#endif /*__ATAT_H*/

/************************** END OF FILE ***************************************/
