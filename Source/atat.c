/**
  ******************************************************************************
  * @file    atat.c
  *
  * @brief   AT-AT core file.
  *
  * @author  esynr3z
  * @date    13.09.2016
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "atat.h"

/* Private defines -----------------------------------------------------------*/

/* Service answer strings */
#define ATAT_ERR_NONE_STR               "OK\r\n"
#define ATAT_ERR_BUFFER_OVEFLOW_STR     "ERROR=BUFFER_OVEFLOW\r\n"
#define ATAT_ERR_NO_HANDLER_STR         "ERROR=NO_HANDLER\r\n"
#define ATAT_ERR_UNKNOWN_HEADER_STR     "ERROR=UNKNOWN_HEADER\r\n"
#define ATAT_ERR_UNKNOWN_COMMAND_STR    "ERROR=UNKNOWN_COMMAND\r\n"
#define ATAT_ERR_UNKNOWN_MODIFIER_STR   "ERROR=UNKNOWN_MODIFIER\r\n"
#define ATAT_ERR_HANDLER_FAILED_STR     "ERROR=HANDLER_FAILED\r\n"

/* Misc strings */
#define ATAT_HEADER_STR     "AT"
#define ATAT_HEADER_EMPTY   0
#define ATAT_CMD_EMPTY      0
#define ATAT_MOD_EMPTY      0
#define ATAT_MOD_GET_STR    "?"
#define ATAT_MOD_SET_STR    "="

/* Private types -------------------------------------------------------------*/

/* Command modifier ID */
typedef enum {
    ATAT_CmdModID_Exec,
    ATAT_CmdModID_Set,
    ATAT_CmdModID_Get
}ATAT_CmdModID_TypeDef;

/* Possible errors */
typedef enum {
    ATAT_Err_None,
    ATAT_Err_BufferOverflow,
    ATAT_Err_UnknownHeader,
    ATAT_Err_UnknownCommand,
    ATAT_Err_UnknownModifier,
    ATAT_Err_NoHandler,
    ATAT_Err_HandlerFailed
}ATAT_Err_TypeDef;

/* Frame structure with raw strings to parse */
typedef struct {
    char Header[ATAT_FRAME_HEADER_SIZE];
    char Command[ATAT_FRAME_COMMAND_SIZE];
    char Modifier[ATAT_FRAME_MODIFIER_SIZE];
    char RxData[ATAT_FRAME_DATA_SIZE];
    ATAT_CmdID_TypeDef CommandID;
    ATAT_CmdModID_TypeDef ModifierID;
    char TxData[ATAT_FRAME_DATA_SIZE];
}ATAT_Frame_TypeDef;

/* Command structure for creating command list */
typedef struct {
    char CmdStr[ATAT_FRAME_COMMAND_SIZE];
    ATAT_CmdID_TypeDef ATAT_CmdID;
    ATAT_HandlerStat_TypeDef (*UserExecHandler)(void);
    ATAT_HandlerStat_TypeDef (*UserSetHandler)(char *Data);
    ATAT_HandlerStat_TypeDef (*UserGetHandler)(char *Data);
}ATAT_Command_TypeDef;

/* Private function prototypes -----------------------------------------------*/
ATAT_Err_TypeDef ATAT_ParseHeader();
ATAT_Err_TypeDef ATAT_ParseModifier();
ATAT_Err_TypeDef ATAT_ParseCommand();
void ATAT_ParseFrame();
void ATAT_FrameFlush();
ATAT_Err_TypeDef ATAT_InvokeHandler();
ATAT_HandlerStat_TypeDef ATAT_Test_DefGetHandler(char *Data);
ATAT_HandlerStat_TypeDef ATAT_Test_DefSetHandler(char *Data);

/* Private variables ---------------------------------------------------------*/

/* Private lib core structure where all inner data located */
struct {
    char RxBuff[ATAT_BUFF_SIZE];
    uint32_t RxBuffIndex;
    ATAT_Frame_TypeDef Frame;
    ATAT_Command_TypeDef CmdList[ATAT_CmdID_TOTAL];
}ATAT_Core;

/* String for TEST command */
char TestStr[64] = "test_value";


/* Private functions ---------------------------------------------------------*/

/*
 * Lib init function.
 *
 * Reset all needed inner data and init all commands.
 */
void ATAT_Init()
{
    /* Start of buffer */
    ATAT_Core.RxBuffIndex = 0;

    /* COMMAND: TEST */
    sprintf(ATAT_Core.CmdList[ATAT_CmdID_Test].CmdStr, ATAT_CMD_STR_TEST);
    ATAT_Core.CmdList[ATAT_CmdID_Test].ATAT_CmdID = ATAT_CmdID_Test;
    ATAT_Core.CmdList[ATAT_CmdID_Test].UserExecHandler = NULL;
    ATAT_Core.CmdList[ATAT_CmdID_Test].UserGetHandler = &ATAT_Test_DefGetHandler;
    ATAT_Core.CmdList[ATAT_CmdID_Test].UserSetHandler = &ATAT_Test_DefSetHandler;

    /* NOTE: you can add init for new command here ^^^ */
}

/*
 * Error handler.
 *
 * Print information about the errors.
 */
void ATAT_Error(ATAT_Err_TypeDef ATAT_Err)
{
    switch(ATAT_Err)
    {
        case ATAT_Err_None:
            printf(ATAT_ERR_NONE_STR);
            break;
        case ATAT_Err_UnknownCommand:
            printf(ATAT_ERR_UNKNOWN_COMMAND_STR);
            break;
        case ATAT_Err_UnknownModifier:
            printf(ATAT_ERR_UNKNOWN_MODIFIER_STR);
            break;
        case ATAT_Err_UnknownHeader:
            printf(ATAT_ERR_UNKNOWN_HEADER_STR);
            break;
        case ATAT_Err_NoHandler:
            printf(ATAT_ERR_NO_HANDLER_STR);
            break;
        case ATAT_Err_HandlerFailed:
            printf(ATAT_ERR_HANDLER_FAILED_STR);
            break;
        case ATAT_Err_BufferOverflow:
            printf(ATAT_ERR_BUFFER_OVEFLOW_STR);
            break;
    }
}

/*
 * Frame flush function.
 *
 * Clear info about previous frame for parsing current correctly.
 */
void ATAT_FrameFlush()
{
    ATAT_Core.Frame.Header[0] = ATAT_HEADER_EMPTY;
    ATAT_Core.Frame.Command[0] = ATAT_CMD_EMPTY;
    ATAT_Core.Frame.Modifier[0] = ATAT_MOD_EMPTY;
}


/*
 * Recieve symbol.
 *
 * Place this function in UART RX interrupt handler, for example.
 * Get frame byte-by-byte and store it in buffer.
 * If frame was completed, start parsing.
 */
void ATAT_GetSymbol(char Symbol)
{
    uint32_t BuffIndex;

    BuffIndex = ATAT_Core.RxBuffIndex;

    /* One element always for '\0' */
    if (BuffIndex == (ATAT_BUFF_SIZE-1))
    {
        ATAT_Error(ATAT_Err_BufferOverflow);
        BuffIndex = 0;
    }
    else
    {
        ATAT_Core.RxBuff[BuffIndex] = Symbol;

        // CRLF frame end check
        if ((ATAT_Core.RxBuff[BuffIndex] == '\n') &&
            (ATAT_Core.RxBuff[BuffIndex-1] == '\r'))
        {
            ATAT_Core.RxBuff[BuffIndex+1] = '\0';
            ATAT_ParseFrame();
            BuffIndex = 0;
        }
        else
        {
            BuffIndex++;
        }
    }

    ATAT_Core.RxBuffIndex = BuffIndex;
}

/*
 * Frame parsing function.
 *
 * Parse frame buffer to fill ATAT_Core.Frame structure.
 * Then parse header and commands.
 */
void ATAT_ParseFrame()
{
    ATAT_Err_TypeDef Status;

    Status = ATAT_Err_None;

    ATAT_FrameFlush();

    sscanf(ATAT_Core.RxBuff, "%2s%[^(\r|\n|?|=)]%1s%s\r\n",
           ATAT_Core.Frame.Header,
           ATAT_Core.Frame.Command,
           ATAT_Core.Frame.Modifier,
           ATAT_Core.Frame.RxData);
    /*
    printf("RX Buff: %s", ATAT_Core.RxBuff);
    printf("Header: %s\n", ATAT_Core.Frame.Header);
    printf("Command: %s\n", ATAT_Core.Frame.Command);
    printf("Modifier: %s\n", ATAT_Core.Frame.Modifier);
    printf("RxData: %s\n", ATAT_Core.Frame.RxData);
    */
    Status = ATAT_ParseHeader();
    if (Status != ATAT_Err_None)
    {
        ATAT_Error(Status);
        return;
    }

    Status = ATAT_ParseModifier();
    if (Status != ATAT_Err_None)
    {
        ATAT_Error(Status);
        return;
    }

    Status = ATAT_ParseCommand();
    if (Status != ATAT_Err_None)
    {
        ATAT_Error(Status);
        return;
    }

    /* If we recieved correct header with empty command,
     * we dont need to invoke handler - master just try to ping us.
     */
    if (ATAT_Core.Frame.Command[0] != ATAT_CMD_EMPTY)
    {
        Status = ATAT_InvokeHandler();
        if (Status != ATAT_Err_None)
        {
            ATAT_Error(Status);
            return;
        }
    }

    ATAT_Error(Status);
}

/*
 * Header parsing function.
 */
ATAT_Err_TypeDef ATAT_ParseHeader()
{
    ATAT_Err_TypeDef Status;

    Status = ATAT_Err_None;

    if (strcmp(ATAT_Core.Frame.Header, ATAT_HEADER_STR) != 0)
    {
        Status = ATAT_Err_UnknownHeader;
    }

    return Status;
}

/*
 * Modifier parsing function.
 */
ATAT_Err_TypeDef ATAT_ParseModifier()
{
    ATAT_Err_TypeDef Status;

    Status = ATAT_Err_None;

    if (ATAT_Core.Frame.Modifier[0] == ATAT_MOD_EMPTY)
    {
        ATAT_Core.Frame.ModifierID = ATAT_CmdModID_Exec;
    }
    else if (strcmp(ATAT_Core.Frame.Modifier, ATAT_MOD_GET_STR) == 0)
    {
        ATAT_Core.Frame.ModifierID = ATAT_CmdModID_Get;
    }
    else if (strcmp(ATAT_Core.Frame.Modifier, ATAT_MOD_SET_STR) == 0)
    {
        ATAT_Core.Frame.ModifierID = ATAT_CmdModID_Set;
    }
    else
    {
        Status = ATAT_Err_UnknownModifier;
    }

    return Status;
}

/*
 * Command parsing function.
 */
ATAT_Err_TypeDef ATAT_ParseCommand()
{
    ATAT_Err_TypeDef Status;

    Status = ATAT_Err_None;

    /* Rigth header with a empty command is a ping */
    if (ATAT_Core.Frame.Command[0] != ATAT_CMD_EMPTY)
    {
        for(uint32_t i=0; i<(uint32_t)ATAT_CmdID_TOTAL; i++)
        {
            /* Search recieved command in command list */
            if (strcmp(ATAT_Core.CmdList[i].CmdStr, ATAT_Core.Frame.Command) == 0)
            {
                ATAT_Core.Frame.CommandID = (ATAT_CmdID_TypeDef)i;
                Status = ATAT_Err_None;
            }
            else
            {
                Status = ATAT_Err_UnknownCommand;
            }
        }
    }

    return Status;
}

/*
 * Call user defined handlers for parsed command.
 */
ATAT_Err_TypeDef ATAT_InvokeHandler()
{
    ATAT_Err_TypeDef Status;
    ATAT_HandlerStat_TypeDef HandlerStat;
    ATAT_CmdID_TypeDef CmdID;
    ATAT_CmdModID_TypeDef ModID;
    char GetData[ATAT_BUFF_SIZE];

    Status = ATAT_Err_None;

    CmdID = ATAT_Core.Frame.CommandID;
    ModID = ATAT_Core.Frame.ModifierID;

    if (ModID == ATAT_CmdModID_Exec)
    {
        if (ATAT_Core.CmdList[CmdID].UserExecHandler == NULL)
        {
            Status = ATAT_Err_NoHandler;
        }
        else
        {
            HandlerStat = ATAT_Core.CmdList[CmdID].UserExecHandler();
            if (HandlerStat != ATAT_HandlerStat_OK)
            {
               Status = ATAT_Err_HandlerFailed;
            }
        }
    }
    else if (ModID == ATAT_CmdModID_Set)
    {
        if (ATAT_Core.CmdList[CmdID].UserSetHandler == NULL)
        {
            Status = ATAT_Err_NoHandler;
        }
        else
        {
            HandlerStat = ATAT_Core.CmdList[CmdID].UserSetHandler(ATAT_Core.Frame.RxData);
            if (HandlerStat != ATAT_HandlerStat_OK)
            {
               Status = ATAT_Err_HandlerFailed;
            }
        }
    }
    else if (ModID == ATAT_CmdModID_Get)
    {
        if (ATAT_Core.CmdList[CmdID].UserGetHandler == NULL)
        {
            Status = ATAT_Err_NoHandler;
        }
        else
        {
            HandlerStat = ATAT_Core.CmdList[CmdID].UserGetHandler(GetData);
            if (HandlerStat != ATAT_HandlerStat_OK)
            {
               Status = ATAT_Err_HandlerFailed;
            }
            else
            {
                printf("%s=%s\r\n", ATAT_Core.CmdList[CmdID].CmdStr, GetData);
            }
        }
    }

    return Status;
}

/*
 * Set user handler for CommandID ExecHandler
 */
void ATAT_ExecHandlerInit(ATAT_CmdID_TypeDef CommandID, ATAT_HandlerStat_TypeDef (*UserExecHandler)(void))
{
    ATAT_Core.CmdList[CommandID].UserExecHandler = UserExecHandler;
}

/*
 * Set user handler for CommandID SetHandler
 */
void ATAT_SetHandlerInit(ATAT_CmdID_TypeDef CommandID, ATAT_HandlerStat_TypeDef (*UserSetHandler)(char *Data))
{
    ATAT_Core.CmdList[CommandID].UserSetHandler = UserSetHandler;
}

/*
 * Set user handler for CommandID GetHandler
 */
void ATAT_GetHandlerInit(ATAT_CmdID_TypeDef CommandID, ATAT_HandlerStat_TypeDef (*UserGetHandler)(char *Data))
{
    ATAT_Core.CmdList[CommandID].UserGetHandler = UserGetHandler;
}

/*
 * Default GET handler for TEST command.
 */
ATAT_HandlerStat_TypeDef ATAT_Test_DefGetHandler(char* Data)
{
    ATAT_HandlerStat_TypeDef HandlerStat;

    HandlerStat = ATAT_HandlerStat_OK;
    strcpy(Data, TestStr);
    return HandlerStat;
}

/*
 * Default SET handler for TEST command.
 */
ATAT_HandlerStat_TypeDef ATAT_Test_DefSetHandler(char *Data)
{
    ATAT_HandlerStat_TypeDef HandlerStat;

    HandlerStat = ATAT_HandlerStat_OK;

    sprintf(TestStr, "%s", Data);

    return HandlerStat;
}

/************************** END OF FILE ***************************************/
