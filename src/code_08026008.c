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

extern void SkillWingedSkeletonHitBoxUpdate(struct EwramData_EntityData *);

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief 26008 | Winged Skeleton red soul: graphics set 0 in an affine slot,
 * the SkillWingedSkeletonHitBoxUpdate child, placed at the caster (8 / -0x1A), song 0xE4. It
 * flies off at 0x40000 / -0x12000 (gravity 0x2000) with the sprite angle
 * unk_510 turning by unk_512 = 0x100 from -0x800 towards 0x4000 (mirrored);
 * its velocity unk_508 / unk_50C follows the angle. When the point one step
 * ahead hits a solid tile (Collision_GetTile, not Collision_IsPointSolid) it waits 32
 * frames, then frees its slot.
 */
void SkillWingedSkeletonUse(struct EwramData_EntityData *entity)
{
    u16 slot;

    switch (entity->state)
    {
    case 0:
    {
        struct SoulGraphicsResources *t = GetSet(0);
        void *gfx = t->graphics;
        void *pal = t->palette;
        void *oam = t->spriteSheet;
        SkillLoadGraphicsInline(entity, 0, gfx, pal, oam);
    }
        entity->animFrame = 0;
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
            EntityDelete(entity);
            return;
        }
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillWingedSkeletonHitBoxUpdate);
        if (child == NULL)
        {
            ObjAffine_Free(entity->affineIndex);
            gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = child;
            EntityDelete(entity);
            return;
        }
    }
        Entity_PlaceRelativeTo(entity, entity->unk_4F8.unk_4F8_entity, 8, -0x1A);
        entity->scrollWithBg = 1;
        entity->spriteLayer = 0xD;
        entity->state++;
    {
        u16 angle, step;
        s32 vx;
        angle = 0xF800;
        entity->unk_510.part16.unk_510 = angle;
        step = 0x100;
        entity->unk_510.part16.unk_512 = step;
        vx = 0x40000;
        entity->velX.whole = vx;
        entity->velY.whole = -0x12000;
        entity->accelY.whole = 0x2000;
        if (entity->flipX)
        {
            entity->unk_510.part16.unk_510 = -angle;
            entity->unk_510.part16.unk_512 = -step;
            entity->velX.whole = -vx;
        }
    }
        PlaySong(0xE4);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
    case 1:
        if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
        {
            entity->roomActive = 1;
            entity->state = 3;
            entity->hidden = 1;
            break;
        }
    {
        s32 angle;
        u16 x, y;
        u16 step = entity->unk_510.part16.unk_512;
        if (entity->unk_510.part16.unk_512 != 0)
        {
            u16 cur = entity->unk_510.part16.unk_510;
            if (entity->unk_510.part16.unk_510 <= 0x3FFF)
                entity->unk_510.part16.unk_510 = cur + step;
        }
        else
        {
            u16 cur = entity->unk_510.part16.unk_510;
            if (entity->unk_510.part16.unk_510 > 0x4000)
                entity->unk_510.part16.unk_510 = cur + step;
        }
        angle = entity->unk_510.part16.unk_510;
        if (entity->flipX)
            angle += -0x8000;
        entity->unk_508.whole = Sin(angle + 0x4000) << 4;
        entity->unk_50C.whole = Sin(angle) << 4;
        EntityAccelerateAndMoveX(entity, 0x80000);
        EntityAccelerateAndMoveY(entity, 0x80000);
        x = (s16)gEwramData->bgInfo[1].xPos.part16.integer + (s16)entity->posX.part16.integer + FIXED_TO_INT(entity->unk_508.whole);
        y = (s16)gEwramData->bgInfo[1].yPos.part16.integer + (s16)entity->posY.part16.integer + FIXED_TO_INT(entity->unk_50C.whole);
        if (Collision_GetTile((s16)x, (s16)y) && !Collision_IsPointSolid((s16)x, (s16)y))
        {
            entity->state = 2;
            entity->timer = 0x20;
        }
    }
        break;
    case 2:
        entity->drawSkip = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 3;
        break;
    case 3:
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    entity->roomActive = 1;
    ObjAffine_Set(entity->unk_510.part16.unk_510, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}
