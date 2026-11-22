#include "code_08009A0.h"
#include "soul_skill_entity.h"
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


/* Inlined Entity_SetAttackBoxFromFrame (src/code_0802D510.c). */
static inline void SetFrameBox(struct EwramData_EntityData *entity)
{
    u8 *spriteSlot = &entity->spriteSlot;
    struct SpriteBox *boxes = gEwramData->spriteSlots[*spriteSlot].sheet->frames[entity->animFrame].boxes;

    if (boxes != NULL)
    {
        entity->hit.flags &= ~2;
        HitBox_SetAttack(entity, boxes);
    }
    else
    {
        entity->hit.flags |= 2;
    }
}

/**
 * @brief 2BD1C | Fireball of Flame Demon / Demon Lord (spawned by
 * SkillDemonLordUse): affine sprite (graphics set 28, frame 9, layer 0xD)
 * with the current frame's hit box in slot 5, spinning by unk_512 = 0x234
 * (mirrored) while moving; every 8th frame it drops a Effect_SpawnShared flame
 * within +-8 / +-2 px with accelY -0x1000. Ends off screen.
 */
void SkillFlameDemonUpdate(struct EwramData_EntityData *entity)
{
    u16 slot;
    s32 spin;
    s32 timer;
    struct EwramData_EntityData *flame;

    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
        {
            gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
            gEwramData->playerState.nbrActiveRedSoulEntities--;
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        struct SoulGraphicsResources *t = GetSet(28);
        s32 zero;
        zero = 0;
        /* the OR with this zero variable survives in the original */
        SkillLoadGraphicsInline(entity, zero, t->graphics, t->palette, t->spriteSheet);
        entity->animFrame = 9;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        Hit_Enable(entity, 5, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= zero;
        SetFrameBox(entity);
    }
        spin = 0x234;
        SKILL_FLAME_DEMON_SPIN(entity) = spin;
        if (entity->flipX)
            SKILL_FLAME_DEMON_SPIN(entity) = -spin;
        entity->state++;
    case 1:
        SKILL_FLAME_DEMON_ANGLE(entity) += SKILL_FLAME_DEMON_SPIN(entity);
        EntityAccelerateAndMove(entity, 0x80000);
        ObjAffine_Set(SKILL_FLAME_DEMON_ANGLE(entity), 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
        timer = entity->timer + 1;
        entity->timer = timer;
        if ((timer & 7) == 0)
        {
            s32 x, y;
            {
                s32 d = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
                x = entity->posX.whole + d;
            }
            {
                s32 d = (RandomNumberGenerator() & 0x3FFFF) - 0x20000;
                y = entity->posY.whole + d;
            }
            flame = Effect_SpawnShared(x, y, 0xF, 7);
            if (flame != NULL)
                flame->accelY.whole = -0x1000;
        }
    }
    if (EntityIsOutsideScreen(entity, 3) || !entity->roomActive)
    {
        entity->roomActive = 1;
        ObjAffine_Free(entity->affineIndex);
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
}
