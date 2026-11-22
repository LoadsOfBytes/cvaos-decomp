/* DeathBlade_Update, decompiled from the original assembly. The player position
 * is read through s32 statement expressions so it stays ldrsh, and dx is
 * narrowed to s16 before dy is computed. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "syscalls.h"
#include "entity_hit.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Effect_SpawnAt(s32, s32, s32);
extern void Enemy_OnContactMask1(struct EwramData_EntityData *);
extern void DeathBlade_OnHit(struct EwramData_EntityData *);

static inline u16 AngleTo(s32 dx, s32 dy)
{
    s16 x = dx; return ArcTan2(x, dy);
}

/* Spinning blade appearing at a random spot in the upper screen and flying at
 * the player, accelerating, until it is hit or its parent finishes. */
void DeathBlade_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ew = &gEwramData;
    struct EwramData_EntityData *parent = &(*ew)->entityData[(u8)entity->unk_4F8.part8.unk_4F8];
    struct HitResult res;
    u16 slot;
    s16 angle;

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)0x08119498, (void *)0x0820BC1C, 0, (void *)0x08249C48))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->attack = parent->attack * 7 / 10;
        entity->unk_518.part16.hp = 1;
        Hit_Enable(entity, 10, Enemy_OnContactMask1, (s32)DeathBlade_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 2;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x11;
        Anim_StartIndexed(entity, 0x10, 2, 0);
    {
        struct EwramData_EntityData *player;
        s32 y;
        s32 mask;
        { u32 r = RandomNumberGenerator(); mask = 0x7F; entity->posX.part16.integer = (r & mask) + 0x40; }
        y = (RandomNumberGenerator() & mask) + 0x10;
        entity->posY.part16.integer = y;
        player = gEwramData->playerState.player;
        {
            s16 dx = ({ s32 px = (s16)player->posX.part16.integer; px; }) - entity->posX.part16.integer;
            entity->unk_4FC.part16.unk_4FC = ArcTan2(dx, ({ s32 py = (s16)player->posY.part16.integer; py; }) - y - 0x10);
        }
    }
        entity->flipX = RandomNumberGenerator() & 1;
        entity->state = 1;
    case 1:
        if (entity->animFlags & 4)
        {
            entity->state = 2;
            entity->hit.flags &= ~2;
        }
        Hit_CheckWithPlayer(&res, (u8 *)entity);
        if ((s16)entity->unk_518.part16.hp != 0)
            break;
        entity->state = 3;
        break;
    case 2:
        if (entity->unk_504.whole <= 0x17FFF)
            entity->unk_504.whole += 0x800;
        {
            s32 s = Sin(entity->unk_4FC.part16.unk_4FC + 0x4000);
            s32 r = entity->unk_504.whole;
            entity->velX.whole = (s >> 8) * (r >> 8);
        }
        {
            s32 s = Sin(entity->unk_4FC.part16.unk_4FC);
            s32 r = entity->unk_504.whole;
            entity->velY.whole = (s >> 8) * (r >> 8);
        }
        EntityAccelerateAndMove(entity, 0x80000);
        Hit_CheckWithPlayer(&res, (u8 *)entity);
        if ((s16)entity->unk_518.part16.hp == 0)
            entity->state = 3;
        if (parent->unk_4F8.part16.unk_4F8 & 1)
            entity->state = 3;
        break;
    case 3:
        Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer);
    case 4:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3))
        entity->state = 4;
    entity->unk_4FC.part16.unk_4FE[0] += 0x7A0;
    angle = entity->unk_4FC.part16.unk_4FE[0];
    if (entity->flipX)
        angle = -angle;
    ObjAffine_Set((s16)angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}
