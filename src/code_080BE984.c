/* Death_UpdateWeaponTransition: scythe boss phase - glide to the screen centre, then play the swing animation and spawn two scythes. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "syscalls.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time);
extern void DeathWeapon_Update(struct EwramData_EntityData *entity);

static inline struct EwramData_EntityData *SpawnScythe(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)DeathWeapon_Update);
    if (child == NULL)
        return NULL;
    child->unk_4F8.part8.unk_4F8 = entity->unk_4F4.part8.unk_4F4;
    child->scrollWithBg = 1;
    return child;
}

static inline u32 AnimFrame(struct EwramData_EntityData *e) { return *(u16 *)&e->animStep; }

/**
 * @brief Transition state (2): move toward the center position, animate
 * and create the primary and mirrored weapon instances at animation
 * step 0x30. Enter hover state 3 when the animation finishes.
 * Preserve the existing allocation assumptions in the paired spawn.
 */
void Death_UpdateWeaponTransition(struct EwramData_EntityData *entity)
{
    s32 targetX;
    s32 oldX, oldY;
    u32 dist;
    u16 angle;
    s32 ax;
    s32 ay;
    struct BgLayerState *bg;
    struct EwramData_EntityData *a, *b;

    switch (entity->subState)
    {
        case 0:
            entity->timer = 0x40;
            entity->subState = 1;
        case 1:
            bg = &gEwramData->bgInfo[1];
            targetX = (0xA0 - (s16)bg->xPos.part16.integer) << 16;
            oldX = entity->posX.whole;
            oldY = entity->posY.whole;
            dist = Sqrt(((oldX - targetX) >> 16) * ((oldX - targetX) >> 16)
                        + ((oldY - 0x600000) >> 16) * ((oldY - 0x600000) >> 16)) << 16;
            ax = (s16)((targetX < 0 ? -(-targetX >> 16) : targetX >> 16) - (s16)entity->posX.part16.integer);
            ay = 0x60 - (s16)entity->posY.part16.integer;
            ay <<= 16;
            ay >>= 16;
            angle = ((u16 (*)(s32, s32))ArcTan2)(ax, ay);
            dist >>= 6;
            Entity_AccelerateAlongAngle(entity, angle, dist, 0, 0, 0);
            entity->velX.whole = entity->posX.whole - oldX;
            entity->velY.whole = entity->posY.whole - oldY;
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            entity->objMode = 0;
            Anim_StartIndexed(entity, 6, 2, 0);
            PlaySong(0x121);
            entity->subState = 2;
            break;
        case 2:
            {
            s16 mask = -1;
            if ((u16)(*(u16 *)&entity->animStep & mask) == 0x25)
                PlaySong(0x124);
            if ((u16)(*(u16 *)&entity->animStep & mask) == 0x2B)
                PlaySong(0x124);
            if ((u16)(*(u16 *)&entity->animStep & mask) == 0x30)
            {
                PlaySong(0x124);
                a = SpawnScythe(entity);
                b = SpawnScythe(entity);
                b->unk_4F8.part8.unk_4F9 = a->unk_4F4.part8.unk_4F4;
                b->unk_4F8.part8.unk_4FA = 1;
            }
            }
            if (entity->animFlags & 4)
            {
                Anim_StartIndexed(entity, 7, 2, 1);
                entity->state = 3;
                entity->stage = 0;
                entity->subState = 0;
                entity->hit.flags &= ~4;
            }
            break;
    }
}
