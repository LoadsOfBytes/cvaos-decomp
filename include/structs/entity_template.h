#ifndef ENTITY_TEMPLATE_H
#define ENTITY_TEMPLATE_H

#include "types.h"

struct EwramData_EntityData;

/* 0x24-byte object/enemy template at 0x080E9644. Only fields supported
 * by the currently decompiled users are named. */
struct EntityTemplate {
    void (*createFunc)(struct EwramData_EntityData *);
    u32 *updateFunc;
    u16 dropItem0; // item id + 1 shown by BestiaryMenu_PrintDrops (bit set enemyFlags[4..7])
    u16 dropItem1; // item id + 1 shown by BestiaryMenu_PrintDrops (bit set enemyFlags[8..11])
    u16 hp; // initial/base enemy HP, copied to runtime entity HP
    u16 mp; /* enemy MP (record order hp, mp, exp); copied to entity mp by Enemy_ResetFromTemplate */
    u8 pad_10[3];
    u8 attack; // copied to entity attack
    u8 defense; // base entity defense, loaded by Enemy_ResetFromTemplate
    u8 pad_15[0x1C - 0x15];
    u16 flags_1C;
    s16 previewOffsetX; // added to X by BestiaryMenu_SpawnEnemyPreview
    s16 previewOffsetY; // added to preview Y; zero also gates Enemy_PreUpdate above-screen hiding
    s16 unk_22;
};

extern struct EntityTemplate sEnemyData[];

#endif
