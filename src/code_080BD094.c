/* Screen-fixed effect: wait for the owner's flag, then rise and leave a trail
 * of DeathBlinkEffect_Update sparks before deleting itself. */
#include "code_08009A0.h"
#include "death_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void DeathBlinkEffect_Update(struct EwramData_EntityData *);
extern u8 sDeathGfx[], sDeathPalette[], sDeathSpriteSheet[];

/* The spark pointer is copied to r1 for the position writes, as in the
 * original allocation; this does not establish the original source form. */
void DeathIntroEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)DEATH_OWNER_INDEX(entity)];

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 2, sDeathSpriteSheet))
                goto delete;
            entity->spriteLayer = 0x19;
            Anim_StartIndexed(entity, 0x18, 2, 1);
            entity->scrollWithBg = 0;
            entity->velX.whole = 0x800000;
            entity->velY.whole = 0x600000;
            entity->state = 1;
            /* Fall through. */
        case 1:
            {
                struct BgLayerState *bg = &gEwramData->bgInfo[1];
                entity->posX.whole = entity->velX.whole - bg->xPos.whole / 2;
                entity->posY.whole = entity->velY.whole - bg->yPos.whole / 2;
            }
            if (*(u16 *)&owner->unk_4F8 & 0x20)
            {
                entity->timer = 0x30;
                entity->state = 2;
            }
            break;
        case 2:
            {
                struct BgLayerState *bg = &gEwramData->bgInfo[1];
                entity->posX.whole = entity->velX.whole - bg->xPos.whole / 2;
                entity->posY.whole = entity->velY.whole - bg->yPos.whole / 2;
            }
            if (entity->timer == 0)
            {
                entity->state = 3;
                entity->timer = 0x20;
                entity->velY.whole = -0x10000;
                entity->accelY.whole = -0x2000;
                break;
            }
            entity->timer--;
            break;
        case 3:
            EntityAccelerateAndMoveY(entity, 0x80000);
            if (!(entity->timer & 3))
            {
                struct EwramData_EntityData *spark = Entity_CreateObject((u32 *)DeathBlinkEffect_Update);
                if (spark)
                {
                    DEATH_OWNER_INDEX(spark) = ENTITY_INDEX(entity);
                    spark->scrollWithBg = 1;
                    {
                        register struct EwramData_EntityData *placed asm("r1") = spark; /* FAKEMATCH: pinned register */
                        placed->posX.whole = entity->posX.whole;
                        placed->posY.whole = entity->posY.whole;
                        placed->subState = 0x18;
                    }
                }
            }
            if (entity->timer == 0)
            {
            delete:
                EntityDelete(entity);
                return;
            }
            entity->timer--;
            break;
    }
    Anim_Update(entity);
}
