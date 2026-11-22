#include "code_08009A0.h"
#include "structs/ewram.h"
struct Unk_085251A8 { u8 frame; u8 life; u16 pad; struct AnimScript *anim; };
extern struct Unk_085251A8 sUnk_085251A8[];
extern void Effect_UpdateTimedDoubleParticle(struct EwramData_EntityData *);
extern void Effect_UpdateTableAnimation(struct EwramData_EntityData *);
/**
 * @brief Create a frame/lifetime or animated effect selected by the shared table, with supplied position, acceleration and palette; return NULL on entity allocation failure.
 */
struct EwramData_EntityData *Effect_CreateFromTable(s32 x, s32 y, s32 ax, s32 ay, s32 index, u8 palette)
{
    struct EwramData_EntityData *entity;
    struct Unk_085251A8 *info = &sUnk_085251A8[index];
    if (info->anim == NULL) entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateTimedDoubleParticle);
    else entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateTableAnimation);
    if (entity != NULL)
    {
        entity->spriteLayer = 12;
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->accelX.whole = ax;
        entity->accelY.whole = ay;
        entity->unk_4FC.whole = index;
        entity->palette = palette;
    }
    return entity;
}
