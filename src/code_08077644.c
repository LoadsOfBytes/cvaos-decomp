/* A projectile changes size, slows down and expires through five animation phases. */
#include "code_08009A0.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void EntityMove(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *,s32);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
void WhiteDragonBreath_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    u32 box0,box1,box2;
    u8 *flags;
    u32 random;
    s32 one;
    u32 vertical;
    u32 *box;
    box0=0x0A0AFBFB;
    box1=0x1010F8F8;
    box2=0x0A0AFBFB;
    random=RandomNumberGenerator();
    one=1;
    flags=ENTITY_RENDER_FLAGS_ADDRESS(entity);
    entity->flipX=random & one;
    vertical=RandomNumberGenerator() & one;
    vertical<<=7;
    *flags=(*flags & 0x7F) | vertical;
    switch(entity->state)
    {
        case 0:
            EntityMove(entity);
            if(entity->timer++<=5) goto collision;
            entity->state++;
            entity->timer=0;
            entity->animFrame++;
            entity->spriteLayer++;
            box=&box0;
            goto set_boxes;
        case 1:
            EntityMove(entity);
            if(entity->timer++<=5) goto collision;
            entity->state++;
            entity->timer=0;
            entity->spriteLayer++;
            entity->animFrame++;
            box=&box1;
            goto set_boxes;
        case 2:
            EntityDampVelocityX(entity,6);
            EntityDampVelocityY(entity,6);
            EntityMove(entity);
            if(entity->timer++<=13) goto collision;
            entity->state++;
            entity->timer=0;
            entity->animFrame++;
            box=&box2;
set_boxes:
            HitBox_SetBody(entity,box);
            HitBox_SetAttack(entity,box);
            goto collision;
        case 3:
            if(entity->timer++>3)
            {
                entity->state++;
                entity->timer=0;
                entity->animFrame++;
            }
collision:
            Hit_CheckWithPlayer(bounds,entity);
            break;
        case 4:
            if(entity->timer++>3)
                EntityDelete(entity);
            break;
    }
}
