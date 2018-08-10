## What is it?

It's a C library called "ATAT" (yes, almost like that huge [armored walker](http://starwars.wikia.com/wiki/All_Terrain_Armored_Transport) from the Star Wars =)) which implements AT-like commands processing. This lib doesn't provide a list of ready commands but gives you a tool to create this list easily yourself, special for your project and needs. ATAT can be used in many many embed devices, which need to be configurated and controlled by simple and clear protocol through widely used UART-USB and UART-Bluetooth converters.
If you don't know what is "AT command" - [Wiki](https://en.wikipedia.org/wiki/Hayes_command_set#GSM).

##  How to use?

* Include ```atat.h``` to your app ```main.c``` file or another where you want to use lib
* Call ```ATAT_Init()``` function to init lib
* You can use ```ATAT_*HandlerInit()``` to define your own handlers for commands
* Place ```ATAT_GetSymbol()``` function, for example, in interrupt handler from UART Rx where it can recieve frames byte-by-byte and process them.

Example of code (pseudocode):
```
#include "atat.h"

void test_exec_handler()
{
    printf("Function test_exec_handler() was called!\n");
}

int main(void)
{
    ATAT_Init();
    ATAT_ExecHandlerInit(ATAT_CmdID_Test, &test_exec_handler);
    UART_Init();

    while(1);
    return 0;
}

UART_IRQHandler()
{
    ATAT_GetSymbol(UART_ReceiveByte());
    UART_ClearFlags();
}
```

## Protocol

All frames from host should start from header ```AT``` and should end by CRLF ```\r\n```.
Also answers from device will be ended the same.

The simplest command is ```AT```. It performs an action like "ping", and answer will be:
```
AT
OK
```

Commands should start from ```+``` and should be placed right after ```AT``` header:
```
AT+TEST
```

Every command have modifier after it. It will define which handler will be called on device: 

* Execution modifier (empty modifier) - call handler which performs some actions
```
AT+TEST
OK
```

* Get modifier ```?``` - call handler which request some values
```
AT+TEST?
+TEST=test_value
OK
```

* Set modifier ```=``` - call handler which write some values
```
AT+TEST=new_value
OK
```

Device will answer with this statuses, if an error occured:

* ```ERROR=BUFFER_OVEFLOW``` - if receive buffer oferflow happened

* ```ERROR=NO_HANDLER``` - if no handler defined for this combination of command and modifier

* ```ERROR=UNKNOWN_HEADER``` - corrupted frame, header not recognized

* ```ERROR=UNKNOWN_COMMAND``` - corrupted or not recognized command

* ```ERROR=UNKNOWN_MODIFIER``` -  corrupted or not recognized modifier

## How to implement your own command

* Add ID of your command to enum ```ATAT_CmdID_TypeDef``` in ```atat.h```

* Add a string with command mnemonic to ```Command strings``` section in ```atat.c```. Remember, that every command should start from ```+``` and have not more than 15 symbols next (by default).

* Place init code for your command in ```ATAT_Init()``` function (```atat.c``` file)

* Define command hanlers (exec, set or get) in user code by ```ATAT_*HandlerInit()``` function