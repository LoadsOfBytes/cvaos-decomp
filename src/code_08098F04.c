/* Mimic: dispatch state, refresh the box on frame changes and run collision. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
union MimicAnimation { u32 words[2]; u8 bytes[8]; };
extern union MimicAnimation sMimicAnimation;
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Mimic_UpdateWait(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Mimic_UpdatePreview(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
void EnemyMimicUpdate(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    union MimicAnimation animation = sMimicAnimation;
    u8 frame;
    entity->ext.mimic.flags = 0;
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: Mimic_UpdateWait(entity); break;
    case 1: Enemy_UpdateFragmentDeath(entity); break;
    case 2:
        Mimic_UpdatePreview(entity);
        Anim_Update(entity);
        return;
    }
    frame = entity->animFrame;
    if (entity->ext.mimic.flags & 2)
    {
        Anim_Start(entity, (struct AnimScript *)&animation, 0, 1);
        animation.bytes[4] = entity->ext.mimic.heldFrame;
        Anim_Update(entity);
    }
    else Anim_Update(entity);
    if (frame != entity->animFrame) Entity_UpdateBoxes(entity);
    if (entity->state != 1 && !(entity->ext.mimic.flags & 1)) Hit_CheckWithPlayer(buffer, entity);
}
