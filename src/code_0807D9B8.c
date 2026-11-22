/* Follow the main body, pulse its affine scale, then shrink away on death. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *);
extern s32 Sin(s32);
extern void GiantGhost_SpawnAuraTrail(struct EwramData_EntityData *, s32, s32);
static inline s32 Shrink(s32 value, s32 duration) { return value - value / duration; }
/**
 * @brief Aura part update: sit above the owner (type 0x13) with its visibility,
 * palette and facing, pulse its affine scale and leave fading trails
 * (GiantGhost_SpawnAuraTrail) every 4 frames; when the owner dies (state 2)
 * fade out for 110 frames.
 */
void GiantGhostAura_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    if (entity->state == 0)
    {
        s32 pulse;
        if (!parent || !parent->updateFunc || parent->unk_518.part8.enemyId != 0x13) goto remove;
        entity->hidden = parent->hidden;
        entity->palette = parent->palette;
        entity->flipX = parent->flipX;
        if (!Enemy_IsStunnedNoFlash(parent))
        {
            entity->unk_500.whole += 0x400;
            Anim_Update(entity);
            entity->timer++;
            if ((entity->timer & 3) == 0)
            {
                s32 x = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
                s32 y = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
                GiantGhost_SpawnAuraTrail(entity, entity->posX.whole + x, entity->posY.whole + y);
            }
        }
        if (entity->flipX) entity->posX.part16.integer = parent->posX.part16.integer - 4;
        else entity->posX.part16.integer = parent->posX.part16.integer + 4;
        entity->posY.part16.integer = parent->posY.part16.integer - 18;
        pulse = Sin(entity->unk_500.whole) / 16;
        entity->velX.whole = pulse + 0x1B000;
        entity->velY.whole = pulse + 0x1D000;
        ObjAffine_SetBiased(0, entity->velX.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
        if (parent->state == 2)
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 110;
        }
        goto finished;
remove:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        s32 scaleX, scaleY;
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
        }
        scaleX = Shrink(entity->velX.whole, entity->timer);
        entity->velX.whole = scaleX;
        scaleY = Shrink(entity->velY.whole, entity->timer);
        entity->velY.whole = scaleY;
        if (scaleX < 0x10000) entity->shrinkPartOffsets = 1;
        ObjAffine_SetBiased(0, entity->velX.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
        Anim_Update(entity);
        if ((s8)--entity->timer <= 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
finished:
    return;
}
