/* HeadStack_Create: create a stack of child segments from table sUnk_081193DC; matched. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

struct HeadStackGraphicsView
{
    void *graphics;
    void *palette;
    void *spriteSheet;
    u32 unk_C;
};
extern struct HeadStackGraphicsView sUnk_081193DC[];
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Enemy_OnContact();
extern void HeadStackSegment_OnHit();
extern void HeadStackSegment_Update(struct EwramData_EntityData *);

struct Flags58 { u8 rest:6; u8 flipX:1; u8 flipY:1; } __attribute__((packed));

static inline u8 InitFromTable(struct EwramData_EntityData *entity, s32 type)
{
    struct HeadStackGraphicsView *info = &sUnk_081193DC[type];
    return Entity_LoadGraphics(entity, info->graphics, info->palette, 0, info->spriteSheet);
}

/**
 * @brief Create the Bone Pillar (type 0) or Fish Head (type 1) controller and its stack of 1..8 segments.
 */
void HeadStack_Create(struct EwramData_EntityData *entity, s32 type)
{
    s32 failed = 0;
    s32 savedY;
    u8 *slot;
    u8 *slots;
    s32 i;
    struct EwramData_EntityData *child;

    if (!InitFromTable(entity, type))
        goto delete;
    entity->drawFunc = (u32 *)failed;
    if (!gEwramData->gameState.bestiaryPreview)
    {
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
    }
    else
    {
        entity->spawnParam0 = 2;
    }
    if (entity->spawnParam0 <= 0)
        entity->spawnParam0 = 1;
    else if (entity->spawnParam0 > 8)
        entity->spawnParam0 = 8;
    savedY = entity->posY.whole;
    slot = entity->ext.headStack.slots;
    i = 1;
    slots = slot;
    goto test;
body:

        *slot = ENTITY_INDEX(child);
        child->ext.headStack.level = i - 1;
        ENTITY_OWNER(child) = entity;
        child->ext.headStack.type = type;
        {
            register s32 off asm("r3") = type << 4; /* FAKEMATCH: pinned register */
            struct HeadStackGraphicsView *info = (void *)(off + (u32)sUnk_081193DC);
            Entity_LoadGraphics(child, info->graphics, info->palette, 0, info->spriteSheet);
        }
        child->enemyId = entity->enemyId;
        Enemy_ResetFromTemplate(child);
        Anim_StartIndexed(child, 0, 0, 1);
        Anim_Update(child);
        {
            struct Flags58 *ef = (void *)(ENTITY_RENDER_FLAGS_ADDRESS(entity));
            u8 v = ef->flipX;
            register struct Flags58 *cf asm("r4") = (void *)(ENTITY_RENDER_FLAGS_ADDRESS(child)); /* FAKEMATCH: pinned register */
            cf->flipX = v;
            ef->flipX ^= 1;
        }
        child->spriteLayer = 10;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        entity->posY.whole -= 0x160000;
        child->scrollWithBg = 1;
        if (gEwramData->gameState.bestiaryPreview)
        {
            child->state = 3;
        }
        else
        {
            child->state = 0;
            Hit_Enable(child, 8, Enemy_OnContact, (s32)HeadStackSegment_OnHit);
            HitBox_SetAttackRaw(child, 0x141CECF2);
            HitBox_SetBodyRaw(child, 0x141CECF2);
        }
        i++;
        slot++;
test:
    if (i <= entity->spawnParam0)
    {
        child = Entity_CreateObject(HeadStackSegment_Update);
        if (child != NULL)
            goto body;
        entity->spawnParam0 = i;
        failed = 1;
    }

    if (failed)
    {
        slot = slots;
        for (i = entity->spawnParam0; i > 0; slot++, i--)
        {
            child = &gEwramData->entityData[*slot];
            EntityDelete(child);
        }
    delete:
        EntityDelete(entity);
        return;
    }
    entity->posY.whole = savedY + 0x160000;
    entity->scrollWithBg = 1;
}
