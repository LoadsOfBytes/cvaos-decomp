/* SkillTinyDevilUpdate: homing bat of SkillTinyDevilUse; matched.
 * Case 2 uses its own target variable (otherwise the case 1 one lives across calls). */
#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "red_soul_entity.h"
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

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

static inline s32 FixedMul(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

/* Homing bat of SkillTinyDevilUse: when the animation ends it targets
 * selectedTarget or a random non-empty entry of trackedEntities and turns towards it
 * for up to 64 frames, then flies straight. */
void SkillTinyDevilUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *victim;
    s32 r, i, j;
    s32 angle, diff;
    s32 timer;

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
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0xD;
        entity->state = 1;
    case 1:
        r = Anim_Update(entity);
        if (r != 3)
            break;
        if (gEwramData->playerState.selectedTarget != NULL)
            target = gEwramData->playerState.selectedTarget;
        else
        {
            i = 0;
            j = RandomNumberGenerator() & r;
            for (; i <= 3; i++)
            {
                if (gEwramData->playerState.trackedEntities[j] != NULL)
                {
                    target = gEwramData->playerState.trackedEntities[j];
                    goto found;
                }
                j = (j + 1) & 3;
            }
            target = NULL;
        }
    found:
        SKILL_TINY_DEVIL_TARGET(entity) = target;
        if (target != NULL)
            entity->state = 2;
        else
            entity->state = 3;
        Hit_Enable(entity, 5, SoulAttack_MarkHit, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x40;
        child = RedSoul_CreateTrailCopy(entity, 7);
        if (child != NULL)
        {
            child->drawSkip = 1;
            child->objMode = 1;
        }
        child = RedSoul_CreateTrailCopy(entity, 3);
        if (child != NULL)
            child->objMode = 1;
        PlaySong(0x134);
        break;
    case 2:
        if (SKILL_TINY_DEVIL_SPEED(entity) <= 0x4FFFF)
            SKILL_TINY_DEVIL_SPEED(entity) += 0x4000;
        victim = SKILL_TINY_DEVIL_TARGET(entity);
        {
            s32 tx = victim->posX.whole;
            s32 ty = victim->posY.whole;
            angle = (s16)SKILL_TINY_DEVIL_HEADING(entity);
            diff = (u16)ArcTan2((s16)(FIXED_TO_INT(tx) - (s16)entity->posX.part16.integer),
                                (s16)(FIXED_TO_INT(ty) - (s16)entity->posY.part16.integer)) - angle;
        }
        if (diff > 0)
        {
            if (diff <= 0x7FFF)
                angle += 0x100;
            else
                angle -= 0x100;
        }
        else
        {
            if (diff < -0x8000)
                angle += 0x100;
            else
                angle -= 0x100;
        }
        SKILL_TINY_DEVIL_HEADING(entity) = angle;
        entity->velX.whole = FixedMul(Sin((s16)SKILL_TINY_DEVIL_HEADING(entity) + 0x4000), SKILL_TINY_DEVIL_SPEED(entity));
        entity->velY.whole = FixedMul(Sin((s16)SKILL_TINY_DEVIL_HEADING(entity)), SKILL_TINY_DEVIL_SPEED(entity));
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer - 1;
        entity->timer = timer;
        if ((s16)victim->unk_518.part16.hp <= 0 || (u8)RED_SOUL_HIT(entity) != 0 || (u8)timer == 0)
            entity->state = 3;
        break;
    case 3:
        if (SKILL_TINY_DEVIL_SPEED(entity) <= 0x4FFFF)
            SKILL_TINY_DEVIL_SPEED(entity) += 0x4000;
        entity->velX.whole = FixedMul(Sin((s16)SKILL_TINY_DEVIL_HEADING(entity) + 0x4000), SKILL_TINY_DEVIL_SPEED(entity));
        entity->velY.whole = FixedMul(Sin((s16)SKILL_TINY_DEVIL_HEADING(entity)), SKILL_TINY_DEVIL_SPEED(entity));
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    case 4:
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 2) || !entity->roomActive)
    {
        entity->roomActive = 1;
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (entity->state > 1)
        entity->animFrame = (((s16)SKILL_TINY_DEVIL_HEADING(entity) + 0x4000) / 0x1000) & 0xF;
}
