#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"

extern s32 Target_GetX(struct EwramData_EntityData *entity);
extern void HeadStackEffect_UpdateAttack(struct EwramData_EntityData *entity);
extern void HeadStackEffect_UpdateParticle(struct EwramData_EntityData *entity);
extern void HeadStackProjectile_Update(struct EwramData_EntityData *entity);

#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)

/* Attack routine of a spawner enemy: spawns HeadStackEffect_UpdateAttack / HeadStackEffect_UpdateParticle /
 * HeadStackProjectile_Update on animation frames, then turns towards the player. */
/**
 * @brief Emit attacks selected by segment-index parity at animation events; wait and turn toward the target.
 */
void HeadStackSegment_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *spawned;
    s32 dx; /* also reused as the facing flag in state 4 */
    s32 offset;
    s32 i;
    u8 timer;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        return;
    case 1:
        if (!(entity->animFlags & 4))
            break;
        Anim_StartIndexed(entity, 3, 0, 0);
        if (!(entity->ext.headStack.level & 1))
        {
            entity->subState = 2;
            return;
        }
        entity->subState = 3;
        return;
    case 2:
        if (FRAME_PAIR(entity) == 2)
        {
            spawned = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadStackEffect_UpdateAttack);
            if (spawned != NULL)
            {
                u16 off = 0xFFFC;
                spawned->flipX = entity->flipX;
                if (entity->flipX)
                    off = 4;
                {
                    u16 *x = &entity->posX.part16.integer;
                    s32 value = (s16)off;
                    value += *x;
                    x = &spawned->posX.part16.integer;
                    *x = value;
                }
                spawned->posY.part16.integer = entity->posY.part16.integer - 10;
                spawned->scrollWithBg = 1;
                spawned->attack = entity->attack;
                ENTITY_OWNER(spawned) = entity;
            }
        }
        if (FRAME_PAIR(entity) == 0x2802)
        {
            dx = -12;
            if (entity->flipX)
                dx = 12;
            offset = dx * 2;
            for (i = 2; i >= 0; i--)
            {
                spawned = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadStackEffect_UpdateParticle);
                if (spawned != NULL)
                {
                    spawned->posX.part16.integer = entity->posX.part16.integer + offset + RandomSigned(0x10);
                    spawned->posY.part16.integer = ({ s32 t = RandomSigned(0x10) - 0x10; t; }) /* keep order */ + entity->posY.part16.integer;
                    spawned->velY.whole = RandomSigned(0x4000) - 0x1000;
                    spawned->accelY.whole = -0x800;
                    spawned->scrollWithBg = 1;
                    spawned->subState = entity->ext.headStack.type;
                }
                offset += dx;
            }
        }
        entity->timer++;
        if (entity->animFlags & 4)
        {
            entity->timer = 0x20;
            entity->subState = 4;
        }
        break;
    case 3:
        if (FRAME_PAIR(entity) == 2)
        {
            spawned = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadStackProjectile_Update);
            if (spawned != NULL)
            {
                spawned->scrollWithBg = 1;
                spawned->attack = entity->attack;
                ENTITY_OWNER(spawned) = entity;
            }
        }
        if (entity->animFlags & 4)
        {
            entity->timer = 0x20;
            entity->subState = 4;
        }
        break;
    case 4:
        if (entity->timer != 0)
            goto count;
        {
            s32 t = Target_GetX(entity);
            s32 px = entity->posX.whole;
            dx = 1;
            if (px > t)
                dx = 0;
        }
        if (entity->flipX != dx)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState = 5;
        }
        else
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 6;
            entity->timer = 0x80;
        }
        break;
    case 5:
        if (entity->animFlags & 4)
        {
            entity->flipX ^= 1;
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 6:
        if (entity->timer != 0)
        {
        count:
            entity->timer--;
            break;
        }
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        break;
    }
}
