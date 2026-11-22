/* Object13Tip_Update, decompiled from the original assembly. The mode-0 branch
 * repeats the "+= 0x300" body instead of jumping to it: cross-jumping merges
 * the copies after reload, but their reloads shift the reload register
 * rotation as in the target. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_080211F0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "entity_hit.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, void *resource, void *palette, u8 variant, void *sheet);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void HitEffect_Spawn(s32, s32, u16);
extern u8 sObject13FamilyGfx[], sUnk_08209FD8[], sObject13FamilySpriteSheet[];

struct Object13ArmView
{
    void *parent;
    s32 angle;
    s32 tipTargetAngle;
    s32 x;
    s32 y;
    u8 segmentIndex;
    u8 ownerSlot;
};
#define ARM(e) ((struct Object13ArmView *)&((struct EwramData_EntityData *)(e))->unk_4FC)

static inline s32 IsNotFlippedX(struct EwramData_EntityData *e) { return e->flipX == 0; }
#define PL (gEwramData->playerState.player)
#define PX ((s16)PL->posX.part16.integer)
#define EX ((s16)entity->posX.part16.integer)
static inline void ArmPlace(struct EwramData_EntityData *entity, struct Object13ArmView *p)
{
    struct Object13ArmView *a2 = ARM(entity), *p2 = ARM(p);
    a2->y = Sin(a2->angle + 0x4000) << 5;
    a2->x = Sin(a2->angle + 0x8000) << 5;
    entity->posX.whole = (a2->x >> 1) + p2->x;
    entity->posY.whole = a2->y / 2 + p2->y;
    a2->x += p2->x;
    a2->y += p2->y;
}
/**
 * @brief Last part of object 0x13: has a hit box, checks player hits with a
 * hit effect and sound 0x69, queued for the player while near.
 */
void Object13Tip_Update(struct EwramData_EntityData *entity)
{
    struct Object13ArmView *self = ARM(entity);
    struct Object13ArmView *p = self->parent;
    struct EwramData_EntityData *other = &gEwramData->entityData[self->ownerSlot];
    struct Object13ArmView *a = ARM(other);
    struct EwramData_EntityData *player;
    struct HitResult res;
    s32 tx, ty, v, mode, r;

    if (other->state == 0xFF)
        entity->state = 0xFF;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sObject13FamilyGfx, sUnk_08209FD8, 0, sObject13FamilySpriteSheet))
        {
            EntityDelete(entity);
            break;
        }
        Anim_StartIndexed(entity, 0, 0, 0);
        Anim_Update(entity);
        entity->spriteLayer = 3;
        Hit_Enable(entity, 8, 0, 0);
        HitBox_SetBodyRaw(entity, 0x2020F4F0);
        HitBox_SetAttackRaw(entity, 0x2020F4F0);
        entity->hit.flags |= 2;
        entity->state++;
    case 1:
        r = 0x60;
        tx = Sin(a->tipTargetAngle + 0x8000) * r + a->x;
        ty = Sin(a->tipTargetAngle + 0x4000) * r + a->y;
        self->angle = a->angle;
        ArmPlace(entity, p);
        entity->velX.whole = tx - entity->posX.whole;
        entity->velY.whole = ty - entity->posY.whole;
        if (entity->velY.whole < 0)
            entity->velY.whole = 0;
        player = gEwramData->playerState.player;
        if (!Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x50), (s16)(player->posY.part16.integer - 0x80), 0xA0, 0x100))
            break;
        Player_QueueEntity(entity, 3);
        Hit_CheckWithPlayer(&res, (u8 *)entity);
        if (!(res.flags & 2))
            break;
        v = other->velX.whole & 0xFFFF;
        other->velX.whole = v;
        mode = 0;
        if ((res.flags & 2) && (s16)gEwramData->playerState.player->posY.part16.integer < (s16)entity->posY.part16.integer)
            break;
        HitEffect_Spawn((s16)entity->posX.part16.integer + RandomSigned(0x10), (s16)entity->posY.part16.integer + RandomSigned(0x10), 0);
        PlaySong(0x69);
        if ((u32)(v - 0x4001) <= 0x7FFE)
            mode = 1;
        else if (other->accelX.whole != 0)
            mode = 2;
        if (mode == 0)
        {
            if (PX < EX)
                other->velX.whole = 0x8000;
            other->accelX.whole += 0x300;
        }
        else if ((mode == 1 && PX < EX && IsNotFlippedX(PL) == mode) || (mode == 2 && PX > EX && !IsNotFlippedX(PL)))
            other->accelX.whole += 0x300;
        else
            other->accelX.whole -= 0x60;
        break;
    case 0xFF:
        EntityDelete(entity);
        break;
    }
}
