/* Death_UpdateDeath: boss death sequence (flash, debris parts); matched. */
#include "code_08009A0.h"
#include "death_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void DeathDebris_Update(struct EwramData_EntityData *entity);
extern void DeathRewardEffect_Update(struct EwramData_EntityData *entity);

static inline struct EwramData_EntityData *SpawnPart(struct EwramData_EntityData *owner, u32 *func)
{
    struct EwramData_EntityData *c = Entity_CreateObject(func);
    if (c == NULL)
        return NULL;
    DEATH_OWNER_INDEX(c) = ENTITY_INDEX(owner);
    c->scrollWithBg = 1;
    return c;
}

struct Flags58 { u8 lo:5; u8 bit5:1; u8 hi:2; };

/**
 * @brief Death state (7): wait, start palette effects when enabled, create
 * four inward-spiraling debris parts and the reward effect, then end
 * the fades and return nonzero for boss removal.
 */
s32 Death_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 i;
    u16 *px;
    struct Flags58 *flags;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 0xE, 2, 1);
        entity->timer = 0x40;
        entity->subState = 1;
    case 1:
        if (entity->timer != 0)
            goto count;
        if (!(gEwramData->gameState.stateFlags & 0x10))
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x10);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
        }
        i = 3;
        flags = (struct Flags58 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity));
        px = &entity->posX.part16.integer;
        for (; i >= 0; i--)
        {
            child = SpawnPart(entity, (u32 *)DeathDebris_Update);
            if (child != NULL)
            {
                child->scrollWithBg = 0;
                child->subState = i;
                {
                    struct BgLayerState *bg = &gEwramData->bgInfo[1];
                    child->posX.part16.integer = *px + bg->xPos.part16.integer;
                    child->posY.part16.integer = entity->posY.part16.integer + bg->yPos.part16.integer + (s16)((i << 4) - 0x30);
                }
                child->timer = i << 2;
            }
        }
        entity->subState = 2;
        SpawnPart(entity, (u32 *)DeathRewardEffect_Update);
        flags->bit5 = 1;
        entity->timer = 0x10;
        break;
    case 2:
        if (entity->timer != 0)
        {
        count:
            entity->timer--;
            break;
        }
        entity->subState = 3;
        if (!(gEwramData->gameState.stateFlags & 0x10))
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
        }
        break;
    case 3:
        return 1;
    }
    return 0;
}
