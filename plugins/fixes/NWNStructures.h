#include "typedefs.h"

#ifndef NWNXStructures_h_
#define NWNXStructures_h_

struct CExoString
{
	char *Text;
	dword Length;
};

struct CNWObjectVarListElement
{
    CExoString sVarName;
    dword      nVarType;
    dword       nVarValue;
};

struct CNWObjectVarList
{
	CNWObjectVarListElement *VarList;
	dword                    VarCount;
};

struct CExoLinkedList;
struct CResRef;
struct CExoLocString;
struct CNWSObject;
struct CNWSCreatureStats;
struct CNWSCreature;
struct CExoLinkedList
{
  /* 0x0/0 */ void *Header;
  /* 0x4/4 */ unsigned long Count;
};
struct CResRef
{
  /* 0x0/0 */ char ResRef[16];
};
struct CExoLocString
{
  /* 0x0/0 */ CExoLinkedList List;
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
  /* 0xB8/184 */ signed long HitPoints;
  /* 0xBC/188 */ unsigned short field_BC;
  /* 0xBE/190 */ char field_BE;
  /* 0xC0/192 */ char rsvd8;
  /* 0xC0/192 */ unsigned short TempHitPoints;
  /* 0xC2/194 */ unsigned short field_C2;
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
struct CNWSCreatureStats
{
  /* 0x0/0 */ unsigned long field_0;
  /* 0x4/4 */ unsigned long field_4;
  /* 0x8/8 */ unsigned short field_8;
  /* 0xA/10 */ unsigned short field_A;
  /* 0x10/16 */ char rsvd1[4];
  /* 0x10/16 */ unsigned long field_10;
  /* 0x24/36 */ char rsvd2[16];
  /* 0x24/36 */ CNWSCreature *OriginalObject;
  /* 0x34/52 */ char rsvd3[12];
  /* 0x34/52 */ CExoLocString FirstName;
  /* 0x3C/60 */ char rsvd4[4];
  /* 0x3C/60 */ CExoLocString LastName;
  /* 0x44/68 */ char rsvd5[4];
  /* 0x44/68 */ CResRef Conversation;
  /* 0x54/84 */ unsigned long ConvInterruptable;
  /* 0x58/88 */ unsigned long field_58;
  /* 0x60/96 */ char rsvd6[4];
  /* 0x60/96 */ unsigned long field_60;
  /* 0x64/100 */ unsigned long field_64;
  /* 0x68/104 */ unsigned long Age;
  /* 0x6C/108 */ unsigned long Gender;
  /* 0x70/112 */ unsigned long XP;
  /* 0x74/116 */ unsigned long IsPC;
  /* 0x78/120 */ unsigned long IsDM;
  /* 0x7C/124 */ unsigned long field_7C;
  /* 0x80/128 */ unsigned long field_80;
  /* 0x84/132 */ unsigned long AIDisabled;
  /* 0x88/136 */ unsigned long field_88;
  /* 0x8C/140 */ unsigned long field_8C;
  /* 0x90/144 */ unsigned long FactionID;
  /* 0x94/148 */ unsigned long field_94;
  /* 0x98/152 */ unsigned long field_98;
  /* 0x9C/156 */ unsigned long field_9C;
  /* 0xA0/160 */ unsigned long field_A0;
  /* 0x49A/1178 */ char rsvd7[1014];
  /* 0x49A/1178 */ char ArmorPart_RFoot;
  /* 0x49B/1179 */ char BodyPart_LFoot;
  /* 0x49C/1180 */ char BodyPart_RShin;
  /* 0x49D/1181 */ char BodyPart_LShin;
  /* 0x49E/1182 */ char BodyPart_LThigh;
  /* 0x49F/1183 */ char BodyPart_RThigh;
  /* 0x4A0/1184 */ char BodyPart_Pelvis;
  /* 0x4A1/1185 */ char BodyPart_Torso;
  /* 0x4A2/1186 */ char BodyPart_Belt;
  /* 0x4A3/1187 */ char BodyPart_Neck;
  /* 0x4A4/1188 */ char BodyPart_RFArm;
  /* 0x4A5/1189 */ char BodyPart_LFArm;
  /* 0x4A6/1190 */ char BodyPart_RBicep;
  /* 0x4A7/1191 */ char BodyPart_LBicep;
  /* 0x4A8/1192 */ char BodyPart_RShoul;
  /* 0x4A9/1193 */ char BodyPart_LShoul;
  /* 0x4AA/1194 */ char BodyPart_RHand;
  /* 0x4AB/1195 */ char BodyPart_LHand;
  /* 0x4E4/1252 */ char rsvd8[56];
  /* 0x4E4/1252 */ unsigned long field_4E4;
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
  /* 0x490/1168 */ void *AreaMiniMaps;
  /* 0x49C/1180 */ char rsvd5[8];
  /* 0x49C/1180 */ unsigned long field_49C;
  /* 0x4A0/1184 */ unsigned long AreaCount;
  /* 0x4A4/1188 */ unsigned long field_4A4;
  /* 0x4B0/1200 */ char rsvd6[8];
  /* 0x4B0/1200 */ unsigned long field_4B0;
  /* 0x4C0/1216 */ char rsvd7[12];
  /* 0x4C0/1216 */ unsigned long InCombat;
  /* 0x4D4/1236 */ char rsvd8[16];
  /* 0x4D4/1236 */ unsigned long Disarmable;
  /* 0x4D8/1240 */ unsigned long CreatureSize;
  /* 0x4E4/1252 */ char rsvd9[8];
  /* 0x4E4/1252 */ unsigned long AttackTarget;
  /* 0x4E8/1256 */ unsigned long AttemptedAttackTarget;
  /* 0x4F8/1272 */ char rsvd10[12];
  /* 0x4F8/1272 */ unsigned long Attacker;
  /* 0x4FC/1276 */ unsigned long SpellTarget;
  /* 0x9E1/2529 */ char rsvd11[1249];
  /* 0x9E1/2529 */ char field_9E1;
  /* 0x9E2/2530 */ char field_9E2;
  /* 0xA18/2584 */ char rsvd12[53];
  /* 0xA18/2584 */ unsigned long PrimaryRange;
  /* 0xA20/2592 */ char rsvd13[4];
  /* 0xA20/2592 */ unsigned long SecondaryRange;
  /* 0xA84/2692 */ char rsvd14[96];
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
  /* 0xAF8/2808 */ char rsvd15[8];
  /* 0xAF8/2808 */ unsigned long IsImmortal;
  /* 0xAFC/2812 */ unsigned long field_AFC[4];
  /* 0xB0C/2828 */ unsigned short field_B0C;
  /* 0xB0E/2830 */ char AIState;
  /* 0xB10/2832 */ char rsvd16;
  /* 0xB10/2832 */ unsigned long field_B10;
  /* 0xB34/2868 */ char rsvd17[32];
  /* 0xB34/2868 */ unsigned long MasterID;
  /* 0xB5C/2908 */ char rsvd18[36];
  /* 0xB5C/2908 */ void *ReputationInformation;
  /* 0xB70/2928 */ char rsvd19[16];
  /* 0xB70/2928 */ void *Inventory;
  /* 0xB74/2932 */ void *ItemRepository;
  /* 0xB78/2936 */ unsigned short field_B78;
  /* 0xB7A/2938 */ unsigned short field_B7A;
  /* 0xB7C/2940 */ unsigned long field_B7C;
  /* 0xB80/2944 */ unsigned long field_B80[42];
  /* 0xC28/3112 */ unsigned long CNWSCreatureAppearanceInfo[16];
  /* 0xC68/3176 */ CNWSCreatureStats *CreatureStats;
  /* 0xC6C/3180 */ unsigned long field_C6C;
  /* 0xC70/3184 */ unsigned long field_C70;
};

#endif
