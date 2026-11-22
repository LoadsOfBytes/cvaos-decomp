/* Bouncing fragment (EvilButcherDebris_Update): falls until it hits the floor, then
 * flies off spinning with a random speed and direction; deleted off screen. */
#include "code_08009A0.h"
#include "evil_butcher_entity.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

#include "structs/entity_render_flags.h"


extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern s32 Collision_GetGroundOffset(s32, s32);
extern u8 sEvilButcherGfx[];
extern u8 sEvilButcherPalette[];
extern u8 sEvilButcherSpriteSheet[];

void EvilButcherDebris_Update(struct EwramData_EntityData *entity)
{
    u16 slot;

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sEvilButcherGfx, sEvilButcherPalette, entity->unk_514.part16.spawnParam1, sEvilButcherSpriteSheet))
                goto delete;
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
                goto delete;
            entity->affineIndex = slot;
            entity->affine = 1;
            entity->doubleSize = 1;
            entity->animFrame = 8;
            entity->spriteLayer = 0x12;
            entity->scrollWithBg = 1;
            EVIL_BUTCHER_DEBRIS_ANGLE(entity) = 0xC000;
            entity->state = 1;
            entity->velY.whole = 0;
            entity->accelY.whole = 0x2000;
            break;
        case 1:
            EntityAccelerateAndMove(entity, 0x80000);
            if (EntityIsOutsideScreen(entity, 3))
            {
                entity->state = 3;
                break;
            }
            {
                s32 x = (s16)entity->posX.part16.integer;
                struct BgLayerState *camera = &gEwramData->bgInfo[1];
                if (!Collision_GetGroundOffset(x + (s16)camera->xPos.part16.integer,
                                  (s16)entity->posY.part16.integer + (s16)camera->yPos.part16.integer + 4))
                    break;
            }
            PlaySong(0x13A);
            EVIL_BUTCHER_DEBRIS_SPIN(entity) = RandomSigned(0x800) + 0x1000;
            entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) + 0x8000;
            if (RandomNumberGenerator() & 1)
            {
                EVIL_BUTCHER_DEBRIS_SPIN(entity) = -EVIL_BUTCHER_DEBRIS_SPIN(entity);
                entity->velX.whole = -entity->velX.whole;
            }
            entity->velY.whole = RandomSigned(0x8000) - 0x20000;
            entity->state = 2;
        case 2:
            EntityAccelerateAndMove(entity, 0x80000);
            EVIL_BUTCHER_DEBRIS_ANGLE(entity) += EVIL_BUTCHER_DEBRIS_SPIN(entity);
            if (!EntityIsOutsideScreen(entity, 3))
                break;
        case 3:
            if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
                ObjAffine_Free(entity->affineIndex);
        delete:
            EntityDelete(entity);
            return;
    }
    ObjAffine_Set(EVIL_BUTCHER_DEBRIS_ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
}
