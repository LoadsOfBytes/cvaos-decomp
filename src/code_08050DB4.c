/* Object 0x0F, reconstructed from the original ROM. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
extern void Object0F_Draw(struct EwramData_EntityData *entity);
extern s32 Pickup_TrySpawnOnHit(struct EwramData_EntityData *entity, s32 offsetY, s32 kind, s32 amount);
extern s32 RandomSigned(s32 range);

/* Select palette from the high byte of the placement parameter (default 7),
 * retain its low byte, and install drawing/collision data. */
void Object0FCreate(struct EwramData_EntityData *entity)
{
    s32 palette = (u16)entity->spawnParam0 >> 8;
    s32 graphics;
    if (palette == 0)
        palette = 7;
    entity->spawnParam0 &= 0xFF;
    entity->spriteLayer = 0x14;
    entity->flipX = 0;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, palette);
    Anim_StartIndexed(entity, 0xE, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 0, 0, 0);
    entity->hit.flags |= 2;
    /* Packed box {-6, -8, 12, 8}, passed by value. */
    HitBox_SetBodyRaw(entity, 0x080CF8FA);
    entity->drawFunc = (u32 *)Object0F_Draw;
}

/* Activation starts a 40-frame countdown, with particles during its first
 * ten frames. The object is marked for removal when the counter wraps. */
void Object0FUpdate(struct EwramData_EntityData *entity)
{
    s32 x;
    s32 y;
    if (!Entity_IsInRect(entity, -0x10, -0x10, 0x110, 0xC0))
        entity->hidden = 1;
    else
    {
        switch (entity->state)
        {
        case 0:
            entity->hidden = 0;
            if (Pickup_TrySpawnOnHit(entity, 0, entity->spawnParam0, entity->spawnParam1))
            {
                PlaySong(0xA2);
                entity->hidden = 1;
                entity->timer = 0x28;
                entity->state++;
            }
            break;
        case 1:
            if (entity->timer > 0x1E && !(entity->timer & 3))
            {
                x = entity->posX.whole + RandomSigned(0x80000);
                y = entity->posY.whole + RandomSigned(0x80000);
                Effect_SpawnShared(x, y, 0, entity->palette);
            }
            if (--entity->timer == 0xFF)
                entity->deleteRequested = 1;
            break;
        }
    }
}
