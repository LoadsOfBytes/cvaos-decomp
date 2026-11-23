/* Red soul projectile: homes on the nearest target, or wanders randomly. */
#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "syscalls.h"

struct SoulGraphicsResources {
    void *graphics;
    void *palette;
    void *spriteSheet;
};
extern struct SoulGraphicsResources sSkillGraphicsSets[];
extern struct EwramData_EntityData *SkillGhost_CreateHistoryTrail(struct EwramData_EntityData *);
extern void SoulAttack_MarkHit(struct EwramData_EntityData *);

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

static inline s16 FixedDelta(s32 fixed, s32 base)
{
    s32 delta;

    if (fixed < 0)
        delta = -(-fixed >> 16) - base;
    else
        delta = (fixed >> 16) - base;
    return delta;
}

static inline s32 FixedMul(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

/**
 * @brief Ghost spirit created by SkillGhostUse: appear, leave a trail, home
 * in on the nearest enemy and fly off.
 */
void SkillGhostSpiritUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    s32 i;
    s32 angle, diff;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *set = GetSet(21);
        void *gfx = set->graphics;
        void *pal = set->palette;
        void *oam = set->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->state = 1;
    case 1:
        if (Anim_Update(entity) != 3)
            break;
        SkillGhost_CreateHistoryTrail(entity);
        {
            u32 bestDistance = -1;
            struct EwramData_EntityData *nearest = NULL;
            struct EwramData_EntityData *found;

            if (gEwramData->playerState.selectedTarget != NULL)
                found = gEwramData->playerState.selectedTarget;
            else
            {
                for (i = 0; i < 4; i++)
                {
                    if (gEwramData->playerState.trackedEntities[i] != NULL)
                    {
                        s32 dx = (s16)gEwramData->playerState.trackedEntities[i]->posX.part16.integer - (s16)entity->posX.part16.integer;
                        s32 dy = gEwramData->playerState.trackedEntities[i]->posY.part16.integer - entity->posY.part16.integer;
                        u32 distance = dx * dx + dy * dy;
                        if (bestDistance > distance)
                        {
                            bestDistance = distance;
                            nearest = gEwramData->playerState.trackedEntities[i];
                        }
                    }
                }
                found = nearest;
            }
            entity->ext.skillHoming.target = found;
            if (found != NULL)
                entity->state = 2;
            else
                entity->state = 3;
        }
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x40;
        break;
    case 2:
    {
        s32 tx, ty;

        if (entity->ext.skillHoming.speed <= 0x3FFFF)
            entity->ext.skillHoming.speed += 0x2000;
        target = entity->ext.skillHoming.target;
        tx = target->posX.whole;
        ty = target->posY.whole;
        angle = SKILL_GHOST_HEADING(entity);
        {
            s32 dx, dy;

            dx = FixedDelta(tx, (s16)(dy = entity->posX.part16.integer));
            dy = FixedDelta(ty, (s16)entity->posY.part16.integer);
            diff = ArcTan2(dx, dy) - angle;
        }
        if (diff > 0)
        {
            if (diff <= 0x7FFF)
                angle += 0x300;
            else
                angle -= 0x300;
        }
        else if (diff < -0x8000)
            angle += 0x300;
        else
            angle -= 0x300;
        SKILL_GHOST_HEADING(entity) = angle;
        entity->velX.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity) + 0x4000), entity->ext.skillHoming.speed);
        entity->velY.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity)), entity->ext.skillHoming.speed);
        EntityAccelerateAndMove(entity, 0x80000);
        entity->timer--;
        if ((s16)target->unk_518.part16.hp <= 0 || (u8)entity->ext.redSoul.hit != 0 || entity->timer == 0)
            entity->state = 4;
        break;
    }
    case 3:
    {
        s32 dx, dy, tx, ty;

        if (entity->ext.skillHoming.speed <= 0x3FFFF)
            entity->ext.skillHoming.speed += 0x2000;
        tx = (RandomNumberGenerator() & 0x1FF) << 16;
        ty = (RandomNumberGenerator() & 0xFF) << 16;
        angle = SKILL_GHOST_HEADING(entity);
        {

            dx = FixedDelta(tx, (s16)(dy = entity->posX.part16.integer));
            dy = FixedDelta(ty, (s16)entity->posY.part16.integer);
            diff = ArcTan2(dx, dy) - angle;
        }
        if (diff > 0)
        {
            if (diff <= 0x7FFF)
                angle += 0x300;
            else
                angle -= 0x300;
        }
        else if (diff < -0x8000)
            angle += 0x300;
        else
            angle -= 0x300;
        SKILL_GHOST_HEADING(entity) = angle;
        entity->velX.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity) + 0x4000), entity->ext.skillHoming.speed);
        entity->velY.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity)), entity->ext.skillHoming.speed);
        EntityAccelerateAndMove(entity, 0x80000);
        if (--entity->timer == 0)
            entity->state = 4;
        break;
    }
    case 4:
        if (entity->ext.skillHoming.speed <= 0x3FFFF)
            entity->ext.skillHoming.speed += 0x2000;
        entity->velX.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity) + 0x4000), entity->ext.skillHoming.speed);
        entity->velY.whole = FixedMul(Sin(SKILL_GHOST_HEADING(entity)), entity->ext.skillHoming.speed);
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    case 5:
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
