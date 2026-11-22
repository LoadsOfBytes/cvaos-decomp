/* Spawn a terminated list of particle templates at the given position. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct ParticleTemplate {
    u8 animation, palette, layer, lifetime;
    s16 x, y;
    s32 vx, vy, ax, ay;
};
extern struct ParticleTemplate *sParticleTemplateLists[];
extern void Effect_UpdateDelayedParticle(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Spawn the selected particle-template list until palette 0xFF or entity allocation failure, copying offsets, motion and start delay.
 */
void Effect_SpawnParticleTemplateList(s16 x, s16 y, s32 index)
{
    struct ParticleTemplate *info = sParticleTemplateLists[index];
    for (; info->palette != 0xFF; info++)
    {
        s32 graphics;
        struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateDelayedParticle);
        if (entity == NULL) return;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, info->palette);
        Anim_StartIndexed(entity, info->animation, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = info->layer;
        entity->posX.part16.integer = (s16)x + info->x;
        entity->posY.part16.integer = (s16)y + info->y;
        entity->velX.whole = info->vx;
        entity->velY.whole = info->vy;
        entity->accelX.whole = info->ax;
        entity->accelY.whole = info->ay;
        entity->timer = info->lifetime;
        entity->scrollWithBg = 1;
    }
}
