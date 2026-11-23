/* GladiatorShot_CreateTip: spawn the follower; GladiatorShotTip_Update: follow a live owner
 * (type byte 0x5D) at an eight-pixel angular offset. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void GladiatorShotTip_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void GladiatorShotTip_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern u8 sGladiatorGfx[], sGladiatorSpriteSheet[];
void GladiatorShotTip_Update(struct EwramData_EntityData *entity);

/* Create the follower of owner: graphics 081CC0AC / 08229104, animation 15,
 * hit callbacks GladiatorShotTip_DamagePlayerOnContact / GladiatorShotTip_OnHit, box 0x0806FAFD (bytes x -3,
 * y -6, w 6, h 8; not an address), one priority level above the owner and
 * placed eight pixels from it along the owner's angle (unk_502). The
 * original clears unk_53D bit 1 through a byte AND with a -3 kept in r5
 * since the prologue, hence the explicit mask local. */
void GladiatorShot_CreateTip(struct EwramData_EntityData *owner)
{
    s32 mask = -3;
    u32 box = 0x0806FAFD;
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)GladiatorShotTip_Update);
    s32 graphics;
    s32 x, y;

    if (child == NULL)
        return;
    graphics = ObjGfx_Find(sGladiatorGfx);
    if (graphics < 0)
    {
        EntityDelete(child);
        return;
    }
    Sprite_Attach(child, sGladiatorSpriteSheet, graphics, owner->basePalette);
    Anim_StartIndexed(child, 0xF, 0, 1);
    Anim_Update(owner);
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    Hit_Enable(child, 10, GladiatorShotTip_DamagePlayerOnContact, (s32)GladiatorShotTip_OnHit);
    HitBox_SetAttack(child, &box);
    HitBox_SetBody(child, &box);
    child->hit.flags &= ~4;
    child->hit.flags &= ~2;
    mask &= *((u8 *)&child->accelY + 5); /* unk_53D: clear scrollWithBg */
    *((u8 *)&child->accelY + 5) = mask;
    child->spriteLayer = owner->spriteLayer - 1;
    child->basePalette = owner->basePalette;
    ENTITY_OWNER(child) = owner;
    child->attack = owner->attack;
    x = FixedMultiply(Sin((u16)owner->ext.gladiatorShot.angle + 0x4000), 0x80000);
    y = FixedMultiply(Sin((u16)owner->ext.gladiatorShot.angle), 0x80000);
    child->posX.whole = x + owner->posX.whole;
    child->posY.whole = y + owner->posY.whole;
}

void GladiatorShotTip_Update(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    if (owner != NULL && owner->updateFunc != NULL
        && owner->enemyId == 0x5D && entity->state == 0)
    {
        s32 x, y;
        entity->hidden = owner->hidden;
        Anim_Update(entity);
        Hit_CheckWithPlayer(buffer, entity);
        x = FixedMultiply(Sin((u16)owner->ext.gladiatorShot.angle + 0x4000), 0x80000);
        y = FixedMultiply(Sin((u16)owner->ext.gladiatorShot.angle), 0x80000);
        entity->posX.whole = x + owner->posX.whole;
        entity->posY.whole = y + owner->posY.whole;
    }
    else EntityDelete(entity);
}
