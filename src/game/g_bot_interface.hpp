#ifndef	__G_BOT_INTERFACE_H
#define	__G_BOT_INTERFACE_H

//#define DREVIL_BOT_SUPPORT
#ifdef DREVIL_BOT_SUPPORT

#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"
#include "ETF_Config.h"

int Bot_Interface_Init(void);
int Bot_Interface_Shutdown(void);
void Bot_Interface_Update();
int Bot_Interface_ConsoleCommand();
void Bot_Interface_SendEvent(int _eid, int _dest, int _source, float _delay, BotUserData *_data);
void Bot_Interface_SendGlobalEvent(int _eid, int _source, float _delay, BotUserData * _data);
void Bot_Interface_LogOutput(const char *_txt);

void Bot_SendSoundEvent(int _client, int _sndtype, GameEntity _source);
void Bot_SendTrigger(const TriggerInfo *_triggerInfo);

#endif
#endif
