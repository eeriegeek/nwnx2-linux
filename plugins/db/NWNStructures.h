#if !defined(NWNStructures_h_)
#define NWNStructures_h_

typedef unsigned long dword;
typedef unsigned short int word;
typedef unsigned char byte;

#include "CExoString.h"

struct Vector
{
	float X;
	float Y;
	float Z;
};

struct Location
{
	dword AreaID;
	Vector vect;
	float Facing;
};

struct CGameObject;
struct CExoArrayList;
struct CResRef;
struct CRes;
struct CResStruct;
struct CResGFF;
struct CNWSModule;
struct CNWSArea;
struct CExoLinkedList;
struct CExoLocString;
struct CNWSObject;
struct CNWSCreature;
struct CNWSPlaceable;
struct CNWSStore;
struct CNWSTrigger;
struct CGameObject
{
  /* 0x0/0 */ unsigned long field_0;
  /* 0x4/4 */ unsigned long ObjectID;
  /* 0x8/8 */ char ObjectType;
  /* 0x9/9 */ char field_9;
  /* 0xA/10 */ char field_A;
  /* 0xB/11 */ char field_B;
  /* 0xC/12 */ void *Methods;
};
struct CExoArrayList
{
  /* 0x0/0 */ void *Array;
  /* 0x4/4 */ unsigned long Count;
};
struct CResRef
{
  /* 0x0/0 */ char ResRef[16];
};
struct CRes
{
  /* 0x0/0 */ unsigned short m_nDemands;
  /* 0x2/2 */ unsigned short m_nRequests;
  /* 0x4/4 */ unsigned long m_nID;
  /* 0x8/8 */ unsigned long field_8;
  /* 0xC/12 */ void *pResData;
  /* 0x10/16 */ void *ResName;
  /* 0x14/20 */ unsigned long m_nSize;
  /* 0x18/24 */ unsigned long m_status;
  /* 0x1C/28 */ unsigned long field_1C;
  /* 0x20/32 */ unsigned long field_20;
  /* 0x24/36 */ void *m_pKeyEntry;
  /* 0x28/40 */ void *vtbl;
};
struct CResStruct
{
  /* 0x0/0 */ unsigned long m_nStructIndex;
};
struct CResGFF
{
  /* 0x0/0 */ CRes Res;
  /* 0x2C/44 */ unsigned long field_2C;
  /* 0x30/48 */ unsigned long field_30;
  /* 0x34/52 */ unsigned long field_34;
  /* 0x38/56 */ unsigned long field_38;
  /* 0x3C/60 */ unsigned long field_3C;
  /* 0x40/64 */ unsigned long field_40;
  /* 0x44/68 */ unsigned long m_pFileHeader;
  /* 0x48/72 */ unsigned long field_48;
  /* 0x4C/76 */ unsigned long field_4C;
  /* 0x50/80 */ unsigned long field_50;
  /* 0x54/84 */ unsigned long field_54;
  /* 0x58/88 */ unsigned long field_58;
  /* 0x5C/92 */ unsigned long field_5C;
  /* 0x60/96 */ unsigned long field_60;
  /* 0x64/100 */ unsigned long field_64;
  /* 0x68/104 */ unsigned long field_68;
  /* 0x6C/108 */ unsigned long field_6C;
  /* 0x70/112 */ unsigned long field_70;
  /* 0x74/116 */ unsigned long field_74;
  /* 0x78/120 */ unsigned long field_78;
  /* 0x7C/124 */ unsigned long field_7C;
  /* 0x80/128 */ unsigned long field_80;
  /* 0x84/132 */ char m_pLabelBuffer[17];
  /* 0x95/149 */ char m_pFileType[4];
  /* 0x9A/154 */ char rsvd1;
  /* 0x9A/154 */ unsigned long m_bLoaded;
  /* 0x9E/158 */ unsigned short field_9E;
  /* 0xA0/160 */ unsigned short field_A0;
  /* 0xA8/168 */ char rsvd2[6];
  /* 0xA8/168 */ unsigned long field_A8;
};
struct CExoLinkedList
{
  /* 0x0/0 */ void *Header;
  /* 0x4/4 */ unsigned long Count;
};
struct CExoLocString
{
  /* 0x0/0 */ CExoLinkedList List;
};
struct CNWSModule
{
  /* 0x0/0 */ CGameObject GameObject;
  /* 0x10/16 */ unsigned long ModuleExpansionList;
  /* 0x14/20 */ unsigned long field_30;
  /* 0x18/24 */ unsigned long AreasResrefList;
  /* 0x1C/28 */ CExoArrayList Areas;
  /* 0x4C/76 */ char rsvd1[40];
  /* 0x4C/76 */ unsigned long field_68;
  /* 0xD4/212 */ char rsvd2[132];
  /* 0xD4/212 */ CExoString field_F0;
  /* 0xDC/220 */ CExoString field_F8;
  /* 0xE4/228 */ CExoString OnPlayerDeath;
  /* 0xEC/236 */ CExoString OnPlayerDying;
  /* 0xF4/244 */ CExoString field_110;
  /* 0x1CC/460 */ char rsvd3[208];
  /* 0x1CC/460 */ unsigned long field_1E8;
  /* 0x1D0/464 */ unsigned long field_1EC;
  /* 0x1D4/468 */ unsigned long field_1F0;
  /* 0x1D8/472 */ unsigned long field_1F4;
  /* 0x1DC/476 */ unsigned long field_1F8;
  /* 0x1E0/480 */ unsigned long field_1FC;
  /* 0x1E4/484 */ unsigned long field_200;
};
struct CNWSArea
{
  /* 0x0/0 */ unsigned long field_0;
  /* 0x4/4 */ unsigned long field_4;
  /* 0x8/8 */ unsigned long field_8;
  /* 0xC/12 */ unsigned long field_C;
  /* 0x10/16 */ unsigned long field_10;
  /* 0x14/20 */ unsigned long field_14;
  /* 0x18/24 */ unsigned long field_18;
  /* 0x1C/28 */ unsigned long field_1C;
  /* 0x20/32 */ unsigned long field_20;
  /* 0x24/36 */ unsigned long field_24;
  /* 0x28/40 */ unsigned long field_28;
  /* 0x2C/44 */ unsigned long field_2C;
  /* 0x30/48 */ unsigned long field_30;
  /* 0x80/128 */ char rsvd1[76];
  /* 0x80/128 */ unsigned long field_80;
  /* 0x84/132 */ unsigned long field_84;
  /* 0x88/136 */ unsigned long field_88;
  /* 0x8C/140 */ unsigned long field_8C;
  /* 0x90/144 */ unsigned long field_90;
  /* 0x94/148 */ unsigned long field_94;
  /* 0x98/152 */ unsigned long field_98;
  /* 0x9C/156 */ unsigned long field_9C;
  /* 0xA0/160 */ unsigned long field_A0;
  /* 0xA4/164 */ unsigned long CNWSArea;
  /* 0xA8/168 */ unsigned long field_A8;
  /* 0xAC/172 */ unsigned long field_AC;
  /* 0xB0/176 */ unsigned long field_B0;
  /* 0xB4/180 */ unsigned long field_B4;
  /* 0xB8/184 */ unsigned long field_B8;
  /* 0xBC/188 */ unsigned long field_BC;
  /* 0xC0/192 */ unsigned long CResARE;
  /* 0xC4/196 */ CGameObject GameObject;
  /* 0xD4/212 */ unsigned long NumPlayers;
  /* 0xFC/252 */ char rsvd2[36];
  /* 0xFC/252 */ CExoLocString Name;
  /* 0x104/260 */ unsigned long Tag;
  /* 0x120/288 */ char rsvd3[24];
  /* 0x120/288 */ unsigned long field_120;
  /* 0x154/340 */ char rsvd4[48];
  /* 0x154/340 */ CExoArrayList ObjectList;
  /* 0x160/352 */ char rsvd5[4];
  /* 0x160/352 */ unsigned long CurrentObjectIndex;
  /* 0x1D4/468 */ char rsvd6[112];
  /* 0x1D4/468 */ unsigned long field_1D4;
  /* 0x1D8/472 */ unsigned long field_1D8;
  /* 0x1DC/476 */ unsigned long field_1DC;
  /* 0x1E0/480 */ unsigned long field_1E0;
  /* 0x1F4/500 */ char rsvd7[16];
  /* 0x1F4/500 */ unsigned long field_D4;
  /* 0x1F8/504 */ unsigned long field_1F8;
  /* 0x1FC/508 */ unsigned long field_1FC;
  /* 0x200/512 */ unsigned long field_200;
  /* 0x204/516 */ unsigned long field_204;
  /* 0x208/520 */ unsigned long field_208;
  /* 0x20C/524 */ unsigned long field_20C;
};
struct CNWSObject
{
  /* 0x0/0 */ unsigned long field_0;
  /* 0x4/4 */ unsigned long ObjectID;
  /* 0x8/8 */ char ObjectType;
  /* 0x9/9 */ char field_9;
  /* 0xA/10 */ char field_A;
  /* 0xB/11 */ char field_B;
  /* 0xC/12 */ void *Methods;
  /* 0x10/16 */ CExoLocString LocString;
  /* 0x18/24 */ unsigned short field_18;
  /* 0x1A/26 */ unsigned short field_1A;
  /* 0x1C/28 */ void *vtbl2;
  /* 0x24/36 */ char rsvd1[4];
  /* 0x24/36 */ CExoString ResRef;
  /* 0x2C/44 */ CResRef field_2C;
  /* 0x40/64 */ char rsvd2[4];
  /* 0x40/64 */ unsigned long Dialog;
  /* 0x58/88 */ char rsvd3[20];
  /* 0x58/88 */ unsigned long ConversationWith;
  /* 0x5C/92 */ unsigned long InConversation;
  /* 0x60/96 */ unsigned long AILevel;
  /* 0x70/112 */ char rsvd4[12];
  /* 0x70/112 */ unsigned long field_70;
  /* 0x78/120 */ char rsvd5[4];
  /* 0x78/120 */ unsigned long AreaID;
  /* 0x7C/124 */ unsigned long X;
  /* 0x80/128 */ unsigned long Y;
  /* 0x84/132 */ unsigned long Z;
  /* 0x8C/140 */ char rsvd6[4];
  /* 0x8C/140 */ unsigned long field_8C;
  /* 0xB8/184 */ char rsvd7[40];
  /* 0xB8/184 */ unsigned long field_B8;
  /* 0xBC/188 */ unsigned short field_BC;
  /* 0xC4/196 */ char rsvd8[6];
  /* 0xC4/196 */ unsigned long CanModifyActionQueue;
  /* 0xC8/200 */ unsigned long IsDestroyable;
  /* 0xCC/204 */ unsigned long IsRaiseable;
  /* 0xD0/208 */ unsigned long DeadSelectable;
  /* 0xD4/212 */ unsigned long Invulnerable;
  /* 0xEC/236 */ char rsvd9[20];
  /* 0xEC/236 */ void *Effects;
  /* 0xF8/248 */ char rsvd10[8];
  /* 0xF8/248 */ unsigned long PerceptionList;
  /* 0x1C0/448 */ char rsvd11[196];
  /* 0x1C0/448 */ unsigned long field_1C0;
};
struct CNWSCreature
{
  /* 0x0/0 */ CNWSObject Object;
  /* 0x1C4/452 */ unsigned long field_1C4;
  /* 0x1F8/504 */ char rsvd1[48];
  /* 0x1F8/504 */ CExoString HeartbeatScript;
  /* 0x200/512 */ CExoString PerceptionScript;
  /* 0x208/520 */ CExoString SpellCastAtScript;
  /* 0x210/528 */ CExoString AttackedScript;
  /* 0x218/536 */ CExoString DamagedScript;
  /* 0x220/544 */ CExoString DisturbedScript;
  /* 0x228/552 */ CExoString EndCombatScript;
  /* 0x230/560 */ CExoString ConversationScript;
  /* 0x238/568 */ CExoString SpawnScript;
  /* 0x240/576 */ CExoString RestedScript;
  /* 0x248/584 */ CExoString DeathScript;
  /* 0x250/592 */ CExoString UserDefScript;
  /* 0x258/600 */ CExoString BlockedScript;
  /* 0x260/608 */ CExoString HeartbeatScript_;
  /* 0x268/616 */ CExoString PerceptionScript_;
  /* 0x270/624 */ CExoString SpellCastAtScript_;
  /* 0x278/632 */ CExoString AttackedScript_;
  /* 0x280/640 */ CExoString DamagedScript_;
  /* 0x288/648 */ CExoString EndCombatScript_;
  /* 0x290/656 */ CExoString ConversationScript_;
  /* 0x298/664 */ CExoString SpawnScript_;
  /* 0x2A0/672 */ CExoString RestedScript_;
  /* 0x2A8/680 */ CExoString DeathScript_;
  /* 0x2B0/688 */ CExoString UserDefScript_;
  /* 0x2B8/696 */ CExoString BlockedScript_;
  /* 0x2CC/716 */ char rsvd2[12];
  /* 0x2CC/716 */ unsigned long Lootable;
  /* 0x2D0/720 */ unsigned long DecayTime;
  /* 0x2D4/724 */ unsigned long BodyBagID;
  /* 0x304/772 */ char rsvd3[44];
  /* 0x304/772 */ unsigned long BlockedBy;
  /* 0x308/776 */ unsigned long PositionStruct;
  /* 0x488/1160 */ char rsvd4[380];
  /* 0x488/1160 */ unsigned long LawfulChaotic;
  /* 0x48C/1164 */ unsigned long field_48C;
  /* 0x490/1168 */ void **AreaMiniMaps;
  /* 0x494/1172 */ CExoArrayList AreaList;
  /* 0x49C/1180 */ unsigned long field_49C;
  /* 0x4A0/1184 */ unsigned long AreaCount;
  /* 0x4A4/1188 */ unsigned long field_4A4;
  /* 0x4B0/1200 */ char rsvd5[8];
  /* 0x4B0/1200 */ unsigned long field_4B0;
  /* 0x4C0/1216 */ char rsvd6[12];
  /* 0x4C0/1216 */ unsigned long InCombat;
  /* 0x4D4/1236 */ char rsvd7[16];
  /* 0x4D4/1236 */ unsigned long Disarmable;
  /* 0x4D8/1240 */ unsigned long CreatureSize;
  /* 0x4E4/1252 */ char rsvd8[8];
  /* 0x4E4/1252 */ unsigned long AttackTarget;
  /* 0x4E8/1256 */ unsigned long AttemptedAttackTarget;
  /* 0x4F8/1272 */ char rsvd9[12];
  /* 0x4F8/1272 */ unsigned long Attacker;
  /* 0x4FC/1276 */ unsigned long SpellTarget;
  /* 0x9E1/2529 */ char rsvd10[1249];
  /* 0x9E1/2529 */ char field_9E1;
  /* 0x9E2/2530 */ char field_9E2;
  /* 0xA18/2584 */ char rsvd11[53];
  /* 0xA18/2584 */ unsigned long PrimaryRange;
  /* 0xA20/2592 */ char rsvd12[4];
  /* 0xA20/2592 */ unsigned long SecondaryRange;
  /* 0xA84/2692 */ char rsvd13[96];
  /* 0xA84/2692 */ unsigned long field_A84;
  /* 0xA88/2696 */ unsigned long Spotted;
  /* 0xA8C/2700 */ unsigned long field_A8C[15];
  /* 0xAC8/2760 */ unsigned long field_AC8;
  /* 0xACC/2764 */ void *CombatRound;
  /* 0xAD0/2768 */ unsigned long field_AD0;
  /* 0xAD4/2772 */ void *Barter;
  /* 0xAD8/2776 */ unsigned long Gold;
  /* 0xADC/2780 */ unsigned long IsPC;
  /* 0xAE0/2784 */ unsigned long SoundSetFile;
  /* 0xAE4/2788 */ unsigned long FootstepType;
  /* 0xAE8/2792 */ unsigned long BodyBag;
  /* 0xAEC/2796 */ unsigned long field_AEC;
  /* 0xAF8/2808 */ char rsvd14[8];
  /* 0xAF8/2808 */ unsigned long IsImmortal;
  /* 0xAFC/2812 */ unsigned long field_AFC[4];
  /* 0xB0C/2828 */ unsigned short field_B0C;
  /* 0xB0E/2830 */ char AIState;
  /* 0xB10/2832 */ char rsvd15;
  /* 0xB10/2832 */ unsigned long field_B10;
  /* 0xB34/2868 */ char rsvd16[32];
  /* 0xB34/2868 */ unsigned long field_B34;
  /* 0xB5C/2908 */ char rsvd17[36];
  /* 0xB5C/2908 */ void *ReputationInformation;
  /* 0xB70/2928 */ char rsvd18[16];
  /* 0xB70/2928 */ void *Inventory;
  /* 0xB74/2932 */ void *ItemRepository;
  /* 0xB78/2936 */ unsigned short field_B78;
  /* 0xB7A/2938 */ unsigned short field_B7A;
  /* 0xB7C/2940 */ unsigned long field_B7C;
  /* 0xB80/2944 */ unsigned long field_B80[42];
  /* 0xC28/3112 */ unsigned long CNWSCreatureAppearanceInfo[16];
  /* 0xC68/3176 */ void *CNWSCreatureStats;
  /* 0xC6C/3180 */ unsigned long field_C6C;
  /* 0xC70/3184 */ unsigned long field_C70;
};
struct CNWSPlaceable
{
  /* 0x0/0 */ CNWSObject Object;
  /* 0x1C4/452 */ unsigned long field_1C4;
  /* 0x1C8/456 */ unsigned long field_1C8;
  /* 0x1CC/460 */ unsigned long field_1CC;
  /* 0x1D0/464 */ unsigned long field_1D0;
  /* 0x1D4/468 */ unsigned long field_1D4;
  /* 0x1D8/472 */ unsigned long field_1D8;
  /* 0x1DC/476 */ unsigned long field_1DC;
  /* 0x1E0/480 */ unsigned long field_1E0;
  /* 0x1E4/484 */ unsigned long field_1E4;
  /* 0x1E8/488 */ unsigned long field_1E8;
  /* 0x1EC/492 */ unsigned long field_1EC;
  /* 0x1F0/496 */ unsigned long field_1F0;
  /* 0x1F4/500 */ unsigned long field_1F4;
  /* 0x1F8/504 */ unsigned long field_1F8;
  /* 0x1FC/508 */ unsigned long field_1FC;
  /* 0x390/912 */ char rsvd1[400];
  /* 0x390/912 */ unsigned long field_390;
};
struct CNWSStore
{
  /* 0x0/0 */ CNWSObject Object;
  /* 0x1C4/452 */ unsigned long field_1C4;
  /* 0x1C8/456 */ unsigned long field_1C8;
  /* 0x1CC/460 */ unsigned long field_1CC;
  /* 0x1D0/464 */ unsigned long field_1D0;
  /* 0x1D4/468 */ unsigned long field_1D4;
  /* 0x1D8/472 */ unsigned long field_1D8;
  /* 0x1DC/476 */ unsigned long field_1DC;
  /* 0x1E0/480 */ unsigned long field_1E0;
  /* 0x1E4/484 */ unsigned long field_1E4;
  /* 0x1E8/488 */ unsigned long field_1E8;
  /* 0x1EC/492 */ unsigned long field_1EC;
  /* 0x1F0/496 */ unsigned long field_1F0;
  /* 0x1F4/500 */ unsigned long field_1F4;
  /* 0x1F8/504 */ unsigned long field_1F8;
  /* 0x1FC/508 */ unsigned long field_1FC;
  /* 0x200/512 */ unsigned long field_200;
  /* 0x204/516 */ unsigned long field_204;
  /* 0x208/520 */ unsigned long field_208;
  /* 0x20C/524 */ unsigned long field_20C;
  /* 0x210/528 */ unsigned long field_210;
  /* 0x214/532 */ unsigned long field_214;
  /* 0x218/536 */ unsigned long field_218;
  /* 0x21C/540 */ unsigned long field_21C;
  /* 0x220/544 */ unsigned long field_220;
  /* 0x224/548 */ unsigned long field_224;
  /* 0x228/552 */ unsigned long field_228;
  /* 0x22C/556 */ unsigned long field_22C;
  /* 0x230/560 */ unsigned long field_230;
  /* 0x234/564 */ unsigned long field_234;
  /* 0x238/568 */ unsigned long field_238;
};
struct CNWSTrigger
{
  /* 0x0/0 */ CNWSObject Object;
  /* 0x1C4/452 */ unsigned long field_1C4;
  /* 0x1C8/456 */ unsigned long field_1C8;
  /* 0x2B0/688 */ char rsvd1[228];
  /* 0x2B0/688 */ unsigned long field_2B0;
};

#endif

