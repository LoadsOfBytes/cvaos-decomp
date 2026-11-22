/* Spinning breath/beam emitter attached to its parent: spawns pieces along
 * the sweep and draws itself through an affine slot. */
#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "code_08013960.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void BaloreEmitter_Draw(struct EwramData_EntityData *entity);
extern void BaloreSpark_Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(void *update);

#define F32(e, field) (*(s32 *)&(e)->field)

static inline struct EwramData_EntityData *SpawnAtEntity(void *update, struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *spawned = Entity_CreateObject(update);

    if (spawned == NULL)
        return NULL;
    spawned->posX.whole = entity->posX.whole;
    spawned->posY.whole = entity->posY.whole;
    spawned->scrollWithBg = 1;
    return spawned;
}

/**
 * @brief B53F8 | Emitter update (state: 0 init, 1 sweep and spawn, 2 fade, 3 delete).
 *
 * Notes for matching: SpawnAtEntity() returns NULL early, so the caller's
 * NULL test is threaded away.  `v` is reused after the scale division; being
 * set again later keeps it the canonical CSE register, which gives
 * `cmp r0, #0; adds r6, r0, #7` for `v / 8`.
 */
void BaloreEmitter_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BALORE_BODY(entity);
    u16 slot;
    struct EwramData_EntityData *piece;
    s32 scale;
    s32 v;

    if (entity->state != 0)
    {
        if (entity->state <= 1 && (BALORE_FLAGS(parent) & 2))
            entity->state = 2;
    }
    switch (entity->state)
    {
    case 0:
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x14;
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant((void *)0x0820B92C, 0x10);
        Sprite_Attach(entity, (void *)0x08244220, parent->unk_4F8.bytes[3], entity->unk_510.part8.basePalette);
        entity->drawFunc = (u32 *)BaloreEmitter_Draw;
        Anim_StartIndexed(entity, 0x32, 2, 0);
        Anim_Update(entity);
        entity->objMode = 1;
        F32(entity, unk_500) = 0x1000000;
        entity->timer = 0x40;
        entity->flipX = RandomNumberGenerator2() & 1;
        if (!entity->flipX)
        {
            F32(entity, unk_504) = 0x7800;
            F32(entity, unk_514) = -0x1C0;
        }
        else
        {
            F32(entity, unk_504) = 0x800;
            F32(entity, unk_514) = 0x1C0;
        }
        Sound_PlayTracked(0x66);
        entity->state = 1;
    case 1:
        if ((entity->timer & 3) == 0)
        {
            piece = SpawnAtEntity(BaloreSpark_Update, entity);
            if (piece != NULL)
            {
                piece->posX.whole = entity->posX.whole + Sin(F32(entity, unk_504) + 0x4000) * 0xB0;
                piece->posY.part16.integer = 0x90;
                piece->velX.whole = ((0x40 - entity->timer) << 9) + 0x8000;
                if (entity->flipX)
                    piece->velX.whole = -piece->velX.whole;
                piece->accelX.whole = -piece->velX.whole / 64;
                BALORE_BODY(piece) = parent;
                piece->timer = ((0x40 - entity->timer) >> 2) + 8;
            }
        }
        F32(entity, unk_508) = Sin(entity->timer << 9);
        F32(entity, unk_504) += F32(entity, unk_514);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 3;
        break;
    case 2:
        if (F32(entity, unk_508) > 0)
        {
            F32(entity, unk_508) += -0x400;
            break;
        }
    case 3:
        Sound_StopTracked(0x66);
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    entity->posX.part16.integer = parent->posX.part16.integer - 0x15;
    entity->posY.part16.integer = parent->posY.part16.integer - 9;
    v = F32(entity, unk_500);
    if (v > 0x1000000)
        v = 0x1000000;
    {
        s32 eighth = v / 8;
        scale = eighth / 32;
    }
    v = Sin(F32(entity, unk_504) + 0x4000) << 5;
    F32(entity, unk_50C) = v;
    F32(entity, unk_510) = Sin(F32(entity, unk_504)) << 5;
    F32(entity, unk_50C) = F32(entity, unk_50C) / 0x10800 * scale;
    F32(entity, unk_510) = F32(entity, unk_510) / 0x10800 * scale;
    ObjAffine_Set(F32(entity, unk_504), scale, F32(entity, unk_508), 0, 0, entity->affineIndex);
}
