// zg_bugfix.c
//
#include "g_local.h"

// osp: 0x00035cc6
// complete & verified
void trap_SendServerCommand(int clientNum, const char *text)
{
    if (strlen(text) > 1020)
    {
        G_LogPrintf("trap_SendServerCommand( %d, ... ) length exceeds 1020.\n", clientNum);
        return;
    }

    real_SendServerCommand(clientNum, text);
}
