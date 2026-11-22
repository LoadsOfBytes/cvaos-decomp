#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void GreatArmorShockwave_Update(struct EwramData_EntityData *);
extern void GreatArmorShockwave_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Spawn a ground shockwave at offset (dx, dy) from owner, snapped to the
 * ground and moving 3.5 px per frame in the facing direction; contact damage
 * GreatArmorShockwave_DamagePlayerOnContact, update
 * GreatArmorShockwave_Update.
 */
void GreatArmor_SpawnShockwave(struct EwramData_EntityData *owner, s32 dx, s32 dy)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)GreatArmorShockwave_Update);
    u32 box = 0x1008F0FC;
    if (entity != NULL)
    {
        s32 graphics;
        entity->posX.whole = owner->posX.whole + dx;
        entity->posY.whole = owner->posY.whole + dy;
        entity->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 7);
        Anim_StartIndexed(entity, 0, 0, 1);
        Anim_Update(entity);
        entity->hidden = 0;
        Hit_Enable(entity, 8, GreatArmorShockwave_DamagePlayerOnContact, 0);
        HitBox_SetAttack(entity, &box);
        entity->hit.flags |= 4;
        entity->hit.flags &= ~2;
        Entity_SnapToGround(entity);
        entity->attack = owner->attack;
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->flipX = owner->flipX;
        if (entity->flipX) entity->velX.whole = 0x38000;
        else entity->velX.whole = -0x38000;
        entity->velY.whole = 0x40000;
        entity->accelY.whole = 0x2800;
    }
}
