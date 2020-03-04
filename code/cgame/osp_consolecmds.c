// osp_consolecmds.c
//
#include "cg_local.h"

// osp: 0x000259b5
void OSP_InitConsoleCommands(void)
{
    trap_AddCommand("?");
    trap_AddCommand("commands");
    trap_AddCommand("motd");
    trap_AddCommand("ready");
    trap_AddCommand("notready");
    trap_AddCommand("players");
    trap_AddCommand("statsall");
    trap_AddCommand("tokens");
    trap_AddCommand("matchinfo");
    trap_AddCommand("settings");
    trap_AddCommand("ref");
    trap_AddCommand("adm");
    trap_AddCommand("tcmd");
    trap_AddCommand("say_teamnl");
    trap_AddCommand("showex");
    trap_AddCommand("sub");
}

// osp: 0x00025a08
void OSP_StartRecording_f(void)
{
    OSP_Record(qtrue);
}

// osp: 0x00025a10
void OSP_StopRecording_f(void)
{
    OSP_Record(qfalse);
}
