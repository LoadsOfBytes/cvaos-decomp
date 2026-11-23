#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"

struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};
extern struct SoulGraphicsResources sSkillGraphicsSets[];
extern void SoulAttack_MarkHit(struct EwramData_EntityData *);
extern void RedSoulHistoryTrail_UpdateUnhide(struct EwramData_EntityData *);
extern void SkillHarpyTrailDraw(struct EwramData_EntityData *);
extern struct EwramData_EntityData *RedSoul_CreateHistoryTrail(struct EwramData_EntityData *, u32 *, u32 *);
#include "code_08001194.h"

static inline void SkillLoadGraphicsInline(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 2);
    Sprite_Attach(entity, sheet, gEwramData->playerState.redSoulGfx, 2);
}

static inline struct SoulGraphicsResources *GetSet(s32 index)
{
    return &sSkillGraphicsSets[index];
}


/**
 * @brief Child of SkillHarpyUse: projectile (graphics set 24, frame 1, box
 * 0x0202FFFF, SoulAttack_MarkHit) with a history trail; stop on a hit
 * or at a wall 6 px ahead, linger 16 updates hidden, end off screen.
 */
void SkillHarpyProjectileUpdate(struct EwramData_EntityData *entity)
{
    u16 x, y;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(24);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x0202FFFF);
        entity->hit.flags |= 4;
        entity->state = 1;
        RedSoul_CreateHistoryTrail(entity, (u32 *)RedSoulHistoryTrail_UpdateUnhide, (u32 *)SkillHarpyTrailDraw);
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if ((u8)entity->ext.redSoul.hit != 0)
        {
            entity->state = 4;
            break;
        }
        x = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity);
        if (entity->flipX)
        {
            if (Collision_GetPushRight((s16)x - 6, (s16)y))
                entity->state = 2;
        }
        else if (Collision_GetPushLeft((s16)x + 6, (s16)y))
            entity->state = 2;
        break;
    case 2:
        entity->timer = 0x10;
        entity->drawSkip = 1;
        entity->state = 3;
    case 3:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 4;
        break;
    case 4:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 4;
    }
    entity->roomActive = 1;
}
