#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern void LilithShot_Update(struct EwramData_EntityData *);
extern void LilithShot_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void LilithShot_OnHit(struct EwramData_EntityData *);
extern u8 sLilithSuccubusGfx[], sLilithSuccubusSpriteSheet[];
/**
 * @brief Spawn the wavy shot at owner + (dx, dy), two layers below
 * (LilithShot_Update).
 */
void Lilith_SpawnShot(struct EwramData_EntityData *owner, s32 dx, s32 dy)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)LilithShot_Update);
    u32 box = 0x0A0AFBFB;
    if (entity != NULL)
    {
        s32 graphics;
        entity->posX.whole = owner->posX.whole + dx;
        entity->posY.whole = owner->posY.whole + dy;
        entity->spriteLayer = owner->spriteLayer - 2;
        graphics = ObjGfx_Find(sLilithSuccubusGfx);
        entity->basePalette = owner->basePalette;
        Sprite_Attach(entity, sLilithSuccubusSpriteSheet, graphics, entity->basePalette);
        entity->animFrame = 0x36;
        Hit_Enable(entity, 8, LilithShot_DamagePlayerOnContact, (s32)LilithShot_OnHit);
        HitBox_SetAttack(entity, &box);
        HitBox_SetBody(entity, &box);
        entity->hit.flags &= ~4;
        entity->hit.flags &= ~2;
        entity->enemyId = owner->enemyId;
        entity->attack = owner->attack;
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        if (owner->flipX)
        {
            entity->ext.lilithShot.angle = 0;
            entity->timer = 21;
        }
        else
        {
            entity->ext.lilithShot.angle = 0x8000;
            entity->timer = 7;
        }
        entity->ext.lilithShot.trail0 = Afterimage_Create(entity, 0, 10, 0);
        entity->ext.lilithShot.trail1 = Afterimage_Create(entity, 0, 5, 0);
    }
}
