/* Witch projectile and particle helpers. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void WitchOrbitParticle_Update(struct EwramData_EntityData *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[];

/**
 * @brief Load and move an animation-13 particle; delete on animation completion.
 */
void WitchMovementParticle_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 0xD, 0, 0);
        entity->spriteLayer = 0x11;
        entity->timer = 8;
        entity->state++;
        /* fall through */
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (Anim_Update(entity) == 3) EntityDelete(entity);
        break;
    }
}

/**
 * @brief Spawn a movement particle at a randomized mirrored offset, with
 * velocity and acceleration derived from the owner velocity.
 */
void Witch_SpawnMovementParticle(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)WitchMovementParticle_Update);
    u16 offset;
    s32 random;
    if (child != NULL)
    {
        offset = 0x10;
        if (entity->flipX) offset = (-(u32)offset << 16) >> 16;
        {
            s32 randomX = RandomSigned(4);
            u16 *position = &entity->posX.part16.integer;
            s32 x = (s16)offset;
            x += *position;
            x += randomX;
            position = &child->posX.part16.integer;
            *position = x;
        }
        {
            s32 y = RandomSigned(4);
            u16 *position = (u16 *)&entity->posY.part16.integer;
            y -= 4;
            y += *position;
            child->posY.part16.integer = y;
        }
        child->velX.whole = -entity->velX.whole / 2;
        random = RandomSigned(8);
        child->velY.whole = -entity->velY.whole / 4 + (random << 14);
        child->accelX.whole = -entity->velX.whole / 64;
        child->accelY.whole = -entity->velY.whole / 64;
        child->scrollWithBg = 1;
    }
}
