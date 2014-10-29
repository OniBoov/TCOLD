#include "ScriptPCH.h"

/* This is where custom scripts' loading functions should be declared. */
void AddSC_custom_scripts();
void AddSC_TW_boss_thorim();
void AddSC_custom_commandscript();

/* This is where custom scripts should be added. */
void AddSC_custom()
{
    AddSC_custom_commandscript();
    AddSC_custom_scripts();
    AddSC_TW_boss_thorim();
}
