#ifndef ENTITY_TEMPLATE_H
#define ENTITY_TEMPLATE_H

#include "types.h"
#include "structs/entity_func.h"

struct EwramData_EntityData;

/* 0x24-byte object/enemy template at 0x080E9644. Only fields supported
 * by the currently decompiled users are named. */
struct EntityTemplate {
    void (*createFunc)(struct EwramData_EntityData *);
    EntityFunc updateFunc;
    u16 dropItem0; // item id + 1 shown by BestiaryMenu_PrintDrops (bit set enemyFlags[4..7])
    u16 dropItem1; // item id + 1 shown by BestiaryMenu_PrintDrops (bit set enemyFlags[8..11])
    u16 hp; // initial/base enemy HP, copied to runtime entity HP
    u16 mp; /* enemy MP (record order hp, mp, exp); copied to entity mp by Enemy_ResetFromTemplate */
    u16 exp; // experience, shown by the bestiary
    u8 soulRate; // soul drop chance
    u8 attack; // copied to entity attack
    u8 defense; // base entity defense, loaded by Enemy_ResetFromTemplate
    u8 dropRate0; // drop chance of dropItem0
    u8 dropRate1; // drop chance of dropItem1
    u8 soulType; // soul dropped: type (bestiary icon)
    u8 soulIndex; // soul dropped: 1-based index, 0 = none
    u8 pad_19;
    u16 resist; // resisted attack attributes (bestiary icons)
    u16 flags_1C; // weak attack attributes (bestiary icons); 0x400 also tested by the hit code
    s16 previewOffsetX; // added to X by BestiaryMenu_SpawnEnemyPreview
    s16 previewOffsetY; // added to preview Y; zero also gates Enemy_PreUpdate above-screen hiding
    s16 unk_22;
};

extern struct EntityTemplate sEnemyData[];

#endif
