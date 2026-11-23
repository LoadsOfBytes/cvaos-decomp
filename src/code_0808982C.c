/* Update a tumbling hit effect with a mutable one-frame animation. */
#include "code_08009A0.h"
#include "gorgon_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityMove(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
/* Animation format 1 embeds four-byte frames after the common header. */
struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sGorgonFrameAnim;
/**
 * @brief Breath puff update: slow down, flicker with random flips, grow its hit box
 * at frame 25 and vanish after 40 frames; contact test every other frame.
 */
void GorgonBreath_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct HitBoxRecord box;
    struct SingleFrameAnimation animation = sGorgonFrameAnim;
    box = (struct HitBoxRecord){-7, -7, 14, 14};
    EntityDampVelocityX(entity, 24);
    EntityDampVelocityY(entity, 24);
    entity->flipX = RandomNumberGenerator() % 2;
    entity->flipY = RandomNumberGenerator() % 2;
    EntityMove(entity);
    if (entity->timer == 25)
    {
        entity->ext.gorgon.forcedFrame = 26;
        HitBox_SetAttack(entity, &box);
    }
    Anim_Start(entity, (struct AnimScript *)&animation, 0, 1);
    animation.frame = entity->ext.gorgon.forcedFrame;
    Anim_Update(entity);
    if (entity->timer++ > 39) EntityDelete(entity);
    else
    {
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->timer & 1) Hit_CheckWithPlayer(collision, entity);
    }
}
