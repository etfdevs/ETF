// Copyright (C) 1999-2000 Id Software, Inc.
//
//===========================================================================
//
// Name:			chars.h
// Function:		bot characteristics
// Programmer:		Mr Elusive (MrElusive@idsoftware.com)
// Last update:		1999-09-08
// Tab Size:		4 (real tabs)
//===========================================================================


//========================================================
//========================================================
//name
#define CHARACTERISTIC_NAME							0	//string
//gender of the bot
#define CHARACTERISTIC_GENDER						1	//string ("male", "female", "it")
//attack skill
// >  0.0 && <  0.2 = don't move
// >  0.3 && <  1.0 = aim at enemy during retreat
// >  0.0 && <  0.4 = only move forward/backward
// >= 0.4 && <  1.0 = circle strafing
// >  0.7 && <  1.0 = random strafe direction change
#define CHARACTERISTIC_ATTACK_SKILL					2	//float [0, 1]
//weapon weight file
#define CHARACTERISTIC_WEAPONWEIGHTS				3	//string
//view angle difference to angle change factor
#define CHARACTERISTIC_VIEW_FACTOR					4	//float <0, 1]
//maximum view angle change
#define CHARACTERISTIC_VIEW_MAXCHANGE				5	//float [1, 360]
//reaction time in seconds
#define CHARACTERISTIC_REACTIONTIME					6	//float [0, 5]

// FALCON : START : Modified to add Q3F weapons
//accuracy when aiming
#define CHARACTERISTIC_AIM_ACCURACY						7	//float [0, 1]
//weapon specific aim accuracy
#define CHARACTERISTIC_AIM_ACCURACY_SHOTGUN				8	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_SUPERSHOTGUN		9	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_NAILGUN				10	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_SUPERNAILGUN		11	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_GRENADELAUNCHER		12	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_ROCKETLAUNCHER		13	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_SNIPERRIFLE			14	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_RAILGUN				15	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_FLAMETHROWER		16	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_MINIGUN				17	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_ASSAULTRIFLE		18	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_DARTGUN				19	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_PIPELAUNCHER		20	//float [0, 1]
#define CHARACTERISTIC_AIM_ACCURACY_NAPALMCANNON		21	//float [0, 1]
//skill when aiming
// >  0.0 && <  0.9 = aim is affected by enemy movement
// >  0.4 && <= 0.8 = enemy linear leading
// >  0.8 && <= 1.0 = enemy exact movement leading
// >  0.5 && <= 1.0 = prediction shots when enemy is not visible
// >  0.6 && <= 1.0 = splash damage by shooting nearby geometry
#define CHARACTERISTIC_AIM_SKILL						22	//float [0, 1]
//weapon specific aim skill
#define CHARACTERISTIC_AIM_SKILL_SHOTGUN				23	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_SUPERSHOTGUN			24	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_NAILGUN				25	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_SUPERNAILGUN			26	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_GRENADELAUNCHER		27	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_ROCKETLAUNCHER			28	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_SNIPERRIFLE			29	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_RAILGUN				30	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_FLAMETHROWER			31	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_MINIGUN				32	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_ASSAULTRIFLE			33	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_DARTGUN				34	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_PIPELAUNCHER			35	//float [0, 1]
#define CHARACTERISTIC_AIM_SKILL_NAPALMCANNON			36	//float [0, 1]
// FALCON : END

//========================================================
//chat
//========================================================
//file with chats
#define CHARACTERISTIC_CHAT_FILE						37	//string
//name of the chat character
#define CHARACTERISTIC_CHAT_NAME						38	//string
//characters per minute type speed
#define CHARACTERISTIC_CHAT_CPM							39	//integer [1, 4000]
//tendency to insult/praise
#define CHARACTERISTIC_CHAT_INSULT						40	//float [0, 1]
//tendency to chat misc
#define CHARACTERISTIC_CHAT_MISC						41	//float [0, 1]
//tendency to chat at start or end of level
#define CHARACTERISTIC_CHAT_STARTENDLEVEL				42	//float [0, 1]
//tendency to chat entering or exiting the game
#define CHARACTERISTIC_CHAT_ENTEREXITGAME				43	//float [0, 1]
//tendency to chat when killed someone
#define CHARACTERISTIC_CHAT_KILL						44	//float [0, 1]
//tendency to chat when died
#define CHARACTERISTIC_CHAT_DEATH						45	//float [0, 1]
//tendency to chat when enemy suicides
#define CHARACTERISTIC_CHAT_ENEMYSUICIDE				46	//float [0, 1]
//tendency to chat when hit while talking
#define CHARACTERISTIC_CHAT_HITTALKING					47	//float [0, 1]
//tendency to chat when bot was hit but didn't dye
#define CHARACTERISTIC_CHAT_HITNODEATH					48	//float [0, 1]
//tendency to chat when bot hit the enemy but enemy didn't dye
#define CHARACTERISTIC_CHAT_HITNOKILL					49	//float [0, 1]
//tendency to randomly chat
#define CHARACTERISTIC_CHAT_RANDOM						50	//float [0, 1]
//tendency to reply
#define CHARACTERISTIC_CHAT_REPLY						51	//float [0, 1]
//========================================================
//movement
//========================================================
//tendency to crouch
#define CHARACTERISTIC_CROUCHER							52	//float [0, 1]
//tendency to jump
#define CHARACTERISTIC_JUMPER							53	//float [0, 1]
//tendency to walk
#define CHARACTERISTIC_WALKER							54	//float [0, 1]
//tendency to jump using a weapon
#define CHARACTERISTIC_WEAPONJUMPING					55	//float [0, 1]
//tendency to use the grapple hook when available
#define CHARACTERISTIC_GRAPPLE_USER						56	//float [0, 1]	//use this!!
//========================================================
//goal
//========================================================
//item weight file
#define CHARACTERISTIC_ITEMWEIGHTS						57	//string
//the aggression of the bot
#define CHARACTERISTIC_AGGRESSION						58	//float [0, 1]
//the self preservation of the bot (rockets near walls etc.)
#define CHARACTERISTIC_SELFPRESERVATION					59	//float [0, 1]
//how likely the bot is to take revenge
#define CHARACTERISTIC_VENGEFULNESS						60	//float [0, 1]	//use this!!
//tendency to camp
#define CHARACTERISTIC_CAMPER							61	//float [0, 1]
//========================================================
//========================================================
//tendency to get easy frags
#define CHARACTERISTIC_EASY_FRAGGER						62	//float [0, 1]
//how alert the bot is (view distance)
#define CHARACTERISTIC_ALERTNESS						63	//float [0, 1]
//how much the bot fires it's weapon
#define CHARACTERISTIC_FIRETHROTTLE						64	//float [0, 1]

