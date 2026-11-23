/* Lightning Doll body parts after death. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s8 ObjPalette_FindOrLoad(void *palette);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern void LightningDollPartEffect_Update(struct EwramData_EntityData *entity);
extern s32 Collision_GetPushUp(s32 x, s32 y);
extern s32 Collision_GetPushDown(s32 x, s32 y);
extern s32 Collision_GetPushLeft(s32 x, s32 y);
extern s32 Collision_GetPushRight(s32 x, s32 y);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 p2, s32 p3, s32 layer);
extern void Effect_SpawnRisingParticleInBox(s32 x, s32 y, s32 p2, s32 p3, s32 layer, s32 p5);
extern u8 sLightningDollSpriteSheet[];
extern u8 sKillerDollLightningDollPalette[];
extern s8 sLightningDollPartOffsets[][6];

#define ABS(x) ((x) < 0 ? -(x) : (x))
static inline s32 RoomX(s32 pos)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return pos + (s16)bg->xPos.part16.integer;
}
static inline s32 RoomY(s32 pos)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return pos + (s16)bg->yPos.part16.integer;
}

static inline u16 Hidden(struct EwramData_EntityData *e) { return 1 - e->hidden; }
/**
 * @brief Initialize a table-configured body part and attached effect, fall and optionally bounce; decrement the parent count after the attached animation completes.
 */
void LightningDollPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *child = &gEwramData->entityData[entity->ext.dollPart.effectIndex];
    s32 x, y;

    switch (entity->state)
    {
        case 0:
        {
            s32 graphics;
            s32 sprite, vx, vy, timer;
            void *spriteData;
            struct EwramData_EntityData *effect;
            entity->scrollWithBg = 1;
            spriteData = sLightningDollSpriteSheet;
            graphics = Sprite_GetTileSlot(parent);
            Sprite_Attach(entity, spriteData, graphics, ObjPalette_FindOrLoad(sKillerDollLightningDollPalette));
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->posX.whole = parent->posX.whole;
            entity->posY.whole = parent->posY.whole;
            x = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][0];
            sprite = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][2];
            vx = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][3] << 12;
            vy = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][4] << 12;
            timer = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][5];
            entity->velX.whole = vx;
            entity->velY.whole = -vy;
            entity->accelY.whole = 0x1800;
            entity->timer = timer;
            entity->animFrame = sprite;
            entity->flipX = parent->flipX;
            if ((s8)Entity_GetTargetDirection(parent) > 0)
            {
                entity->velX.whole = -ABS(entity->velX.whole);
                x = -ABS(x);
            }
            effect = EntityCreateInRange(0x49, 0xCF, (u32 *)LightningDollPartEffect_Update);
            if (effect != NULL)
            {
                effect->posX.whole = entity->posX.whole;
                effect->posY.whole = entity->posY.whole - 0x100000;
                effect->flipX = RandomNumberGenerator() % 2;
                effect->spriteLayer = entity->spriteLayer + 1;
                ENTITY_PARENT_INDEX(effect) = ENTITY_INDEX(entity);
                entity->ext.dollPart.effectIndex = ENTITY_INDEX(effect);
            }
            entity->state++;
            break;
        }
        case 1:
        {
            s32 correction;
            s32 zero;
            s32 done;
            zero = entity->timer & 15;
            if (zero == 0 && entity->timer < 0x40 && entity->timer > 0x20)
            {
                x = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][0];
                y = sLightningDollPartOffsets[(u8)entity->ext.dollPart.index][1];
                Effect_SpawnRandom(entity->posX.whole + x, entity->posY.whole + y, 0, 0, entity->spriteLayer - 1);
                Effect_SpawnRisingParticleInBox(entity->posX.whole + x, entity->posY.whole + y, 0, 0, entity->spriteLayer - 2, zero);
            }
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velY.whole += entity->accelY.whole;
            if ((u8)entity->ext.dollPart.index == 1)
            {
                if (entity->velY.whole >= 0)
                {
                    correction = Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer), RoomY((s16)entity->posY.part16.integer) - 15);
                    if (correction != 0)
                    {
                        PlaySong(0x153);
                        entity->posY.part16.integer += correction;
                        entity->velY.whole = -entity->velY.whole / 2;
                        if (ABS(entity->velY.whole) < 0x6000)
                        {
                            entity->accelX.whole = 0;
                            entity->velX.whole = 0;
                            entity->accelY.whole = 0;
                            entity->velY.whole = 0;
                        }
                    }
                }
                else
                {
                    correction = Collision_GetPushDown(RoomX((s16)entity->posX.part16.integer), RoomY((s16)entity->posY.part16.integer) - 32);
                    if (correction != 0)
                        entity->posY.part16.integer += correction;
                }
                if (entity->velX.whole > 0)
                    correction = Collision_GetPushLeft(RoomX((s16)entity->posX.part16.integer) + 8, RoomY((s16)entity->posY.part16.integer) - 16);
                else
                {
                    if (entity->velX.whole == 0)
                        goto timer;
                    correction = Collision_GetPushRight(RoomX((s16)entity->posX.part16.integer) - 8, RoomY((s16)entity->posY.part16.integer) - 16);
                }
                if (correction != 0)
                {
                    entity->posX.part16.integer += correction;
                    entity->velX.whole = 0;
                }
            }
timer:
            if ((entity->timer & 1) && entity->timer < 0x40)
                entity->hidden = Hidden(entity);
            done = 0;
            if (--entity->timer == 0)
                done = 1;
            if ((s16)entity->posY.part16.integer > 200)
                done = 2;
            if (done != 0)
            {
                entity->velY.whole = 0;
                entity->velX.whole = 0;
                entity->state++;
                entity->hidden = 1;
            }
            break;
        }
        case 2:
            if (child->animDone)
            {
                parent->ext.doll.deathPartCount--;
                entity->deleteRequested = 1;
                child->deleteRequested = 1;
            }
            break;
    }
}
