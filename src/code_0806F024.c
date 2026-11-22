/* Werewolf attack phases, 0x0806F024..0x0806F41B. */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code_08009A0.h"

extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Tile_Classify(s16, s16);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern void Effect_SpawnRisingParticleInBox(s32, s32, s32, s32, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void Werewolf_SpawnShockwave(struct EwramData_EntityData *, s32);

/**
 * @brief Attack state machine (state 1): randomly a dash (sound 0x77) or a ground
 * slam spawning shockwaves (Werewolf_SpawnShockwave at 0x40, then 0x30 and
 * 0x50 px, sound 0x79); then back to state 0.
 */
void Werewolf_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags;
    s32 x;
    switch (entity->subState)
    {
    case 0:
        flags = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        if (!(flags & 4)) break;
        if (entity->timer++ <= 0x1D) break;
        if (RandomNumberGenerator() & 1)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->animDone = 0;
            break;
        }
        else
        {
            if (entity->flipX)
                x = entity->posX.part16.integer + 0x40;
            else
                x = entity->posX.part16.integer - 0x20;
            if (!Tile_Classify((s16)(background->xPos.part16.integer + x),
                             (s16)(background->yPos.part16.integer + entity->posY.part16.integer)))
            {
                entity->subState = 3;
                Anim_StartIndexed(entity, 3, 0, 0);
            }
            else
            {
                entity->subState = 1;
                Anim_StartIndexed(entity, 2, 0, 0);
            }
        }
        entity->animDone = 0;
        break;
    case 1:
        if (entity->animStep <= 1) break;
        entity->subState++;
        if (entity->flipX) entity->velX.whole = 0x50000;
        else entity->velX.whole = -0x50000;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        entity->timer = 0;
        entity->unk_4F4.whole = 0;
        /* fall through */
    case 2:
        if (entity->unk_4F4.whole != 0)
        {
            entity->unk_4F4.whole = 0;
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->state = 3;
            entity->subState = 0;
            break;
        }
        if (entity->animDone)
        {
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0x20000;
            entity->accelY.whole = 0x2800;
            Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
            entity->state = 0;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 1);
            break;
        }
        if (entity->animStep > 2)
        {
            EntityDampVelocityX(entity, 4);
            Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
            break;
        }
        EntityDampVelocityX(entity, 0x10);
        flags = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        if (!(entity->timer++ & 3) && (flags & 4))
            Effect_SpawnRisingParticleInBox(entity->posX.whole + entity->velX.whole * 6,
                         entity->posY.whole - 0x20000, 0x60000, 0x40000, entity->spriteLayer - 1, 1);
        break;
    case 3:
        if (entity->animStep <= 1) break;
        entity->subState++;
        entity->animTimer = 0;
        entity->timer = 0;
        entity->animDone = 0;
        break;
    case 4:
        if (!(entity->timer & 7))
        {
            PlaySong(0x77);
            if (entity->flipX)
                Effect_SpawnRandom(entity->posX.whole - 0xE0000, entity->posY.whole - 0x140000, 0x40000, 0x40000, entity->spriteLayer - 1);
            else
                Effect_SpawnRandom(entity->posX.whole + 0xE0000, entity->posY.whole - 0x140000, 0x40000, 0x40000, entity->spriteLayer - 1);
        }
        entity->timer++;
        entity->animTimer = 0;
        if (entity->timer == 0x1E) entity->animTimer = 0xC8;
        if (entity->timer == 0x1F) Werewolf_SpawnShockwave(entity, 0x40);
        if (entity->timer > 0x23)
        {
            entity->subState++;
            Werewolf_SpawnShockwave(entity, 0x30);
            Werewolf_SpawnShockwave(entity, 0x50);
            PlaySong(0x79);
        }
        break;
    case 5:
        if (entity->animStep <= 3) break;
        if (entity->flipX) entity->velX.whole = 0x160000;
        else entity->velX.whole = -0x160000;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        entity->subState++;
        entity->velX.whole = 0;
        break;
    case 6:
        if (!entity->animDone) break;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        Anim_StartIndexed(entity, 0, 0, 1);
        Entity_UpdateBoxes(entity);
        entity->timer = 0;
        entity->subState++;
        break;
    case 7:
        if (++entity->timer > 0x2C)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->subState = 1;
            entity->timer = 0;
        }
        break;
    }
}
