/* SkillDeathSickleUpdate, decompiled from the original assembly. The second random
 * number goes into its own block variable: a block-local value ties with the
 * shifted bit in local-alloc, as in the target. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "code_080D73B8.h"
#include "code/code_08060B98.h"
#include "structs/entity_render_flags.h"

extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);

static inline void LoadGfx(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}

/* Same as BlueSoul_DeleteEntity: end a blue soul entity. */
static inline void EndBlueSoul(struct EwramData_EntityData *entity)
{
    if (entity->affineIndex != 0 || entity->affine)
        ObjAffine_Free(entity->affineIndex);
    gEwramData->playerState.activeBlueSoulEntities[entity->unk_508.part16.unk_508] = NULL;
    EntityDelete(entity);
}
extern s32 Entity_StepAngleTowardPoint(struct EwramData_EntityData *, s32, s32, s32, s32);
extern struct EwramData_EntityData *Player_GetSelectedOrNearestTarget(struct EwramData_EntityData *);

static inline void SetVelocity(struct EwramData_EntityData *entity)
{
    {
        s32 s = Sin((s16)entity->unk_518.part16.hp + 0x4000);
        s32 r = entity->unk_500.whole;
        entity->velX.whole = (s >> 8) * (r >> 8);
    }
    {
        s32 s = Sin((s16)entity->unk_518.part16.hp);
        s32 r = entity->unk_500.whole;
        entity->velY.whole = (s >> 8) * (r >> 8);
    }
}

/**
 * @brief Projectile spawned by SkillDeathUse: appear with sound 0x96, pick the
 * selected or nearest target and home in on it.
 */
void SkillDeathSickleUpdate(struct EwramData_EntityData *entity)
{
    u16 slot;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(0);
        if ((s16)slot < 0)
            goto end;
        entity->affineIndex = slot;
        entity->affine = 1;
    {
        u8 *graphics = (u8 *)0x081E2174;
        u8 *palette = (u8 *)0x0820B694;
        u8 *sprite = (u8 *)0x08237770;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->spriteLayer = 0xF;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
    {
        s32 r = RandomNumberGenerator();
        s32 one;
        entity->unk_510.part16.unk_510 = r;
        entity->unk_518.part16.hp = r;
        entity->unk_510.part16.unk_512 = 0xF700;
        { s32 r2 = RandomNumberGenerator(); s32 f = r2 & (one = 1); entity->flipX = f; }
        if (*(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x40)
            entity->unk_510.part16.unk_512 = -entity->unk_510.part16.unk_512;
        PlaySong(0x96);
        entity->state = one;
    }
    case 1:
    {
        s32 r;
        if (entity->unk_50C.whole <= 0xFFFF)
            entity->unk_50C.whole += 0x2000;
        entity->unk_510.part16.unk_510 += entity->unk_510.part16.unk_512;
        r = Anim_Update(entity);
        if (r != 3)
            break;
        entity->unk_514.unk_514_32 = Player_GetSelectedOrNearestTarget(entity);
        if (entity->unk_514.unk_514_32 != NULL)
            entity->state = 2;
        else
            entity->state = r;
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->timer = 0x40;
        break;
    }
    case 2:
    {
        struct EwramData_EntityData *target;
        s32 timer;
        if (entity->unk_500.whole <= 0x1FFFF)
            entity->unk_500.whole += 0x2000;
        target = entity->unk_514.unk_514_32;
        entity->unk_518.part16.hp = Entity_StepAngleTowardPoint(entity, target->posX.whole, target->posY.whole, (s16)entity->unk_518.part16.hp, 0x555);
        entity->unk_510.part16.unk_510 += entity->unk_510.part16.unk_512;
        SetVelocity(entity);
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer - 1;
        entity->timer = timer;
        if ((s16)target->unk_518.part16.hp <= 0 || (u8)timer == 0)
            entity->state = 3;
        break;
    }
    case 3:
        if (entity->unk_500.whole <= 0x1FFFF)
            entity->unk_500.whole += 0x2000;
        entity->unk_518.part16.hp += entity->unk_510.part16.unk_512 / 16;
        entity->unk_510.part16.unk_510 += entity->unk_510.part16.unk_512;
        SetVelocity(entity);
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    case 4:
    end:
        EndBlueSoul(entity);
        return;
    }
    ObjAffine_Set(entity->unk_510.part16.unk_510, entity->unk_50C.whole, entity->unk_50C.whole, entity->flipX, 0, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 1))
        entity->state = 4;
}
