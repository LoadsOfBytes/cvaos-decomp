/* Object 0x19 helpers 0x08053EC4..0x08053F33. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 RandomSigned(s32 range);
/* Called with untruncated arguments (HitEffect_Spawn takes u16 values). */
extern void HitEffect_Spawn(s32 x, s32 y, s32 type);

/**
 * @brief Hit callback of object 0x19: sound 0x69 and a hit effect at a random
 * spot (0..15 px) from the entity.
 */
void Object19_OnHit(struct EwramData_EntityData *entity)
{
    s32 x;

    PlaySong(0x69);
    x = (s16)entity->posX.part16.integer + RandomSigned(0x10);
    HitEffect_Spawn(x, (s16)entity->posY.part16.integer + RandomSigned(0x10), 0);
}

/**
 * @brief Draw frame 0 without and frame 1 with bit 4 of 0x53D.
 */
void Object19_Draw(struct EwramData_EntityData *entity)
{
    entity->affine = 0;
    entity->animFrame = 0;
    Sprite_Draw(entity);
    entity->affine = 1;
    entity->animFrame = 1;
    Sprite_Draw(entity);
}
