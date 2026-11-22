/* Chaos spark drawing (392C) and the soul shot creation (39DC). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
};

extern u8 sChaosSpriteSheet[];
extern void ChaosSoulShot_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void ChaosSoulShot_Update(struct EwramData_EntityData *);

#define OFFSET_X(e) ((e)->unk_4F8.whole)
#define OFFSET_Y(e) ((e)->unk_4FC.whole)
#define SPREAD(e) ((e)->unk_504.whole)

/* Draw the spark four times, mirrored around its position by SPREAD. */
void ChaosSpark_Draw(struct EwramData_EntityData *entity)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;

    entity->posX.whole = x - SPREAD(entity);
    entity->posY.whole = y - SPREAD(entity);
    entity->flipX = 0;
    entity->flipY = 0;
    Sprite_DrawEntity(entity);
    entity->posX.whole = x + SPREAD(entity);
    entity->posY.whole = y - SPREAD(entity);
    entity->flipX = 1;
    entity->flipY = 0;
    Sprite_DrawEntity(entity);
    entity->posX.whole = x - SPREAD(entity);
    entity->posY.whole = y + SPREAD(entity);
    entity->flipX = 0;
    entity->flipY = 1;
    Sprite_DrawEntity(entity);
    entity->posX.whole = x + SPREAD(entity);
    entity->posY.whole = y + SPREAD(entity);
    entity->flipX = 1;
    entity->flipY = 1;
    Sprite_DrawEntity(entity);
    entity->posX.whole = x;
    entity->posY.whole = y;
}

/* Create a soul shot at the orb + (x, y) facing the target (10 HP, box
 * 0x1A30F0E8 = x -24, y -16, w 48, h 26, harmed by ChaosSoulShot_DamagePlayerOnContact). */
void ChaosOrb_SpawnSoulShot(struct EwramData_EntityData *orb, s32 x, s32 y)
{
    u32 box;
    struct EwramData_EntityData *owner = orb->unk_4F4.unk_4F4_entity;
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosSoulShot_Update);
    struct ChaosData *data;

    box = 0x1A30F0E8;
    data = (struct ChaosData *)owner->unk_504.whole;
    if (shot != NULL)
    {
        s32 graphics;

        shot->posX.part16.integer = 0xFF80;
        shot->posY.part16.integer = 0xFF80;
        OFFSET_X(shot) = x;
        OFFSET_Y(shot) = y;
        shot->spriteLayer = orb->spriteLayer - 1;
        graphics = data->graphics;
        shot->unk_510.part8.basePalette = data->palette;
        Sprite_Attach(shot, sChaosSpriteSheet, graphics, shot->unk_510.part8.basePalette);
        shot->animMode = 2;
        Anim_StartIndexed(shot, 1, 2, 0);
        Anim_Update(shot);
        Hit_Enable(shot, 8, ChaosSoulShot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(shot, &box);
        shot->hit.flags |= 4;
        shot->hit.flags &= ~2;
        shot->unk_518.part8.enemyId = orb->unk_518.part8.enemyId;
        shot->unk_4F4.unk_4F4_entity = orb;
        shot->scrollWithBg = 0;
        shot->attack = 0x78;
        shot->unk_518.part16.hp = 10;
        {
            s32 targetX = (*ewram)->playerState.enemyTarget->posX.whole;
            if (orb->posX.whole >= targetX)
                shot->flipX = 0;
            else
                shot->flipX = 1;
        }
        shot->drawFunc = (u32 *)Sprite_DrawEntity;
        PlaySong(0x78);
    }
}
