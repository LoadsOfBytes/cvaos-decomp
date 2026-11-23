/* Object35_UpdateState, Object35_SpawnChild. */
#include "code_08001194.h"
#include "room_object_entity.h"
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern struct Unk_03002CB0 gUnk_03002CB0;
extern void EntityMoveY(struct EwramData_EntityData *entity);
extern u32 RandomNumberGenerator(void);
extern void Effect_SpawnParticleWithDamping(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant, s32 damping, s32 flags, s32 palette);

#define FLAG(e) ((e)->spawnParam1)

/**
 * @brief 5A30C | Update of a floor switch entity (presumably). State 0:
 * after the player (standingPlatform) has stood on it for 8 frames set event flag
 * spawnParam1 (gameState.gameFlagsTail bit set), song 0x133; 1: sink for 5 frames.
 * State 10: wait for the flag, song 0x113; 11: rise with OBJ window on and
 * dust particles every 8 frames, after 0x60 frames refresh the three map
 * tiles above; 12: OBJ window off.
 */
void Object35_UpdateState(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    switch (entity->subState)
    {
        case 0:
            if (gEwramData->playerState.standingPlatform == entity)
            {
                if ((s32)++entity->ext.object35.standFrames > 7)
                {
                    entity->subState++;
                    entity->velY.whole = 0;
                    entity->timer = 0;
                    gEwramData->gameState.gameFlagsTail[FLAG(entity) >> 5] |= 1 << (FLAG(entity) & 0x1F);
                    PlaySong(0x133);
                }
            }
            else
            {
                entity->ext.object35.standFrames = 0;
            }
            break;
        case 1:
            EntityMoveY(entity);
            entity->velY.whole = 0x10000;
            if (entity->timer++ > 4)
            {
                entity->subState++;
                entity->ext.object35.pressed = 0;
            }
            break;
        case 10:
            if (gEwramData->gameState.gameFlagsTail[FLAG(entity) >> 5] & (1 << (FLAG(entity) & 0x1F)))
            {
                entity->subState++;
                entity->velY.whole = 0x8000;
                entity->timer = 0;
                PlaySong(0x113);
            }
            break;
        case 11:
        {
            s32 phase;

            EntityMoveY(entity);
            gUnk_03002CB0.dispCnt |= 0x8000;
            entity->ext.object35.pressed = 1;
            phase = entity->timer & 7;
            if (phase == 0)
            {
                s32 vx = (RandomNumberGenerator() & 0x7FFF) - 0x4000;
                s32 vy = -0x8000 - (RandomNumberGenerator() & 0x7FFF);

                Effect_SpawnParticleWithDamping(entity->ext.object35.homeX, entity->ext.object35.homeY, vx, vy,
                             entity->spriteLayer - 1, phase, 0x20, 0xF, 6);
            }
            if (entity->timer++ > 0x5F)
            {
                s32 x;
                s32 y;

                entity->subState++;
                x = OBJECT35_HOME_PIXEL_X(entity) + (s16)bg->xPos.part16.integer;
                y = OBJECT35_HOME_PIXEL_Y(entity) + (s16)bg->yPos.part16.integer;
                Collision_ClearBlock(x, y - 8);
                Collision_ClearBlock(x, y - 0x18);
                Collision_ClearBlock(x, y - 0x28);
            }
            break;
        }
        case 12:
            gUnk_03002CB0.dispCnt &= 0x7FFF;
            break;
    }
}

extern u8 sObject35Gfx[];
extern u8 sObject35SpriteSheet[];
void Object35Child_Update(void);

/**
 * @brief 5A50C | Spawn the child part of parent (slots 0x49..0xCF, update
 * Object35Child_Update): same position, one layer below, graphics sObject35Gfx
 * with palette sObject35SpriteSheet and parent's palette slot, frame 3, linked
 * to parent through unk_504.
 */
void Object35_SpawnChild(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object35Child_Update);
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = parent->posX.whole;
        child->posY.whole = parent->posY.whole;
        child->spriteLayer = parent->spriteLayer - 1;
        gfx = ObjGfx_Find(sObject35Gfx);
        child->basePalette = parent->basePalette;
        Sprite_Attach(child, sObject35SpriteSheet, gfx, child->basePalette);
        child->animFrame = 3;
        child->hit.flags |= 4;
        child->hit.flags |= 2;
        child->ext.object35Child.owner = parent;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
