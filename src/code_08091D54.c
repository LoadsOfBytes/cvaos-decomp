/* Parts of the 081CC014 / 08220A38 enemy attached to their owner. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);

extern void DevilChargeEffect_Update(struct EwramData_EntityData *);
extern u8 sDevilGfx[], sDevilSpriteSheet[];

/* Ride on the owner at (unk_4F8, unk_4FC) while the timer timer runs
 * (sound 0x12A and two flashes when it expires; for types 0x48 / 0x66 the
 * hurt box 0x1818F4F4 = x -12, y -12, w 24, h 24 switches on at frame
 * unk_504), then fall; delete off screen. */
void DevilProjectile_Update(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    struct EwramData_EntityData *owner = entity->unk_500.unk_500_entity;
    u32 box = 0x1818F4F4;

    if (entity->timer == 0)
    {
        entity->scrollWithBg = 1;
        EntityAccelerateAndMove(entity, 0x40000);
    }
    else
    {
        entity->timer--;
        entity->posX.whole = owner->posX.whole + entity->unk_4F8.whole;
        entity->posY.whole = owner->posY.whole + entity->unk_4FC.whole;
        if (entity->timer == 0)
        {
            PlaySong(0x12A);
            Afterimage_Create(entity, 0x7FFF, 12, 16);
            Afterimage_Create(entity, 0x7FFF, 6, 16);
        }
        if ((entity->unk_518.part8.enemyId == 0x48 || entity->unk_518.part8.enemyId == 0x66)
            && entity->unk_504.part8.unk_504 == entity->timer)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            HitBox_SetAttack(entity, &box);
        }
    }
    if (EntityIsOutsideScreen(entity, 2))
        EntityDelete(entity);
    else
    {
        Anim_Update(entity);
        Hit_CheckWithPlayer(buffer, entity);
    }
}

/* Spawn a part attached to owner at offset (dx, dy) (kept in unk_4F8 /
 * unk_4FC) in the owner palette, timer timer, the owner type byte and
 * attack; updated by DevilChargeEffect_Update. */
void DevilChargeEffect_Spawn(struct EwramData_EntityData *owner, s32 dx, s32 dy, u8 timer)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)DevilChargeEffect_Update);
    s32 graphics;

    if (entity == NULL)
        return;
    entity->posX.whole = owner->posX.whole + dx;
    entity->posY.whole = owner->posY.whole + dy;
    entity->unk_4F8.whole = dx;
    entity->unk_4FC.whole = dy;
    entity->spriteLayer = owner->spriteLayer - 1;
    graphics = ObjGfx_Find(sDevilGfx);
    entity->unk_510.part8.basePalette = owner->unk_510.part8.basePalette;
    Sprite_Attach(entity, sDevilSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->timer = timer;
    entity->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
    entity->attack = owner->attack;
    entity->unk_500.unk_500_entity = owner;
    entity->scrollWithBg = 0;
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
}
