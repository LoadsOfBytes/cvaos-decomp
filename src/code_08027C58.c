#include "code_08009A0.h"
#include "red_soul_entity.h"
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
#include "code_08013960.h"

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
#include "code_08001194.h"

extern s32 Entity_MoveAndCollide(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);

static inline struct EwramData_EntityData *NearestTarget(struct EwramData_EntityData *entity)
{
    u32 bestDistance = -1;
    struct EwramData_EntityData *best = NULL;
    struct EwramData_EntityData *selected = gEwramData->playerState.selectedTarget;
    s32 i;
    if (selected != NULL)
        return selected;
    for (i = 0; i < 4; i++)
    {
        struct EwramData_EntityData *target = gEwramData->playerState.trackedEntities[i];
        if (target != NULL)
        {
            s32 dx = (s16)target->posX.part16.integer - (s16)entity->posX.part16.integer;
            s32 dy = target->posY.part16.integer - entity->posY.part16.integer;
            u32 distance = dx * dx + dy * dy;
            if (bestDistance > distance)
            {
                bestDistance = distance;
                best = target;
            }
        }
    }
    return best;
}

void SkillKillerFishUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(11);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_SetTimerOverride(1, 30);
        Entity_PlaceRelativeTo(entity, parent, 0x14, -0xC);
        entity->flipX ^= 1;
    {
        s32 vx = -0x28000;
        entity->velX.whole = vx;
        if (entity->flipX)
            entity->velX.whole = -vx;
    }
        entity->scrollWithBg = 1;
        entity->state = 1;
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x2030F0E8);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        PlaySong(0x19C);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
    case 1:
        if (!(entity->animFlags & 4))
            break;
    {
        s16 x = GetEntityRoomXPositionInteger(entity);
        s16 y = GetEntityRoomYPositionInteger(entity);
        if (Collision_IsPointSolid(x, y) || Collision_IsPointSolid(x, y - 0x10))
        {
            struct EwramData_EntityData *target = NearestTarget(entity);
            if (target != NULL)
            {
                register s32 dy asm("r4"); /* FAKEMATCH: pinned register */
                s32 ady;
                {
                    register s32 a asm("r1") = target->posY.whole; /* FAKEMATCH: pinned register */
                    register s32 b asm("r0") = entity->posY.whole; /* FAKEMATCH: pinned register */
                    dy = a - b;
                }
                ady = dy;
                if (dy < 0)
                    ady = -dy;
                if (ady > 0x200000)
                {
                    s32 q = dy;
                    if (dy < 0)
                        q += 127;
                    entity->velY.whole = q >> 7;
                    entity->accelY.whole = entity->velY.whole / 128;
                }
            }
            entity->hit.flags &= ~2;
            entity->state = 3;
            PlaySong(0xDA);
        }
        else
        {
            entity->velX.whole /= 2;
            entity->velY.whole = -0x38000;
            entity->accelY.whole = 0x2000;
            entity->flipY = 1;
            entity->state = 2;
            entity->drawSkip = 1;
            PlaySong(0x1AD);
            PlaySong(0xDB);
        }
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        entity->timer++;
        if ((entity->timer & 3) == 0)
        {
            s32 x, y;
            {
                s32 d = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
                x = entity->posX.whole + d;
            }
            {
                /* -k keeps the offset out of CSE (reloaded from the pool as in the original) */
                s32 d = (RandomNumberGenerator() & 0x7FFFF) + ({ s32 k = 0x40000; -k; });
                y = entity->posY.whole + d;
            }
            Effect_SpawnShared(x, y, 0, 7);
        }
        break;
    case 3:
    {
        s32 r = Entity_MoveAndCollide(entity, 0x80000, -0x70000, 1, 0x30000);
        if (r & 3)
        {
            Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 0, 7);
            entity->state = 4;
        }
        if (!(r & 0x80))
        {
            entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 7);
            entity->velY.whole = 0;
            entity->accelY.whole = 0x400;
        }
    }
        break;
    case 4:
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 4;
        entity->hidden = 1;
    }
    Anim_Update(entity);
}
