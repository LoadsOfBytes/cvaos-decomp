/* Objects 0x11 (timer in entity slot 26) and 0x12 (decorations). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

void BossRushTimer_Update(struct EwramData_EntityData *entity);
void Object12Variant2_Draw(struct EwramData_EntityData *entity);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, void *resource, void *palette, u8 variant, void *sheet);

extern u8 sObject12Variant0AnimScript0[];
extern u8 sObject12Variant0AnimScript1[];
extern u8 sObject12Variant0AnimScript2[];

/* Object 0x12 variants (unk_514). */
struct Object12VariantDefinition {
    void *graphics; // graphics
    void *palette; // palette
    void *spriteSheet; // animation set
    u8 paletteIndex; // palette index
    u8 animationIndex; // animation
    u8 pad_E;
    u8 spriteLayer; // layer (spriteLayer)
    u16 flags; // flags: 1 OBJ mode 1, 2 room coordinates, 4 no animation, 8 only near the screen
    u8 pad_12[2];
    void (*updateFunc)(struct EwramData_EntityData *); // update, NULL for the plain animation
};

extern const struct Object12VariantDefinition sObject12Variants[];

/**
 * @brief 512C4 | Object 0x11 create: replace entity slot 26 by a counter
 * entity (update BossRushTimer_Update).
 */
void Object11Create(struct EwramData_EntityData *entity)
{
    EntityDelete(&gEwramData->entityData[26]);
    EntityCreateInRange(0x1A, 0x1A, (u32 *)BossRushTimer_Update)->roomActive = 1;
}

/**
 * @brief 512F8 | Object 0x11 update: nothing.
 */
void Object11Update(struct EwramData_EntityData *entity)
{
}

/**
 * @brief Timer entity (slot 26): count unk_4F8 up to 0x34BBE (one hour at 60
 * frames per second) while bit 1 of stateFlags (no saving) is set.
 */
void BossRushTimer_Update(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    if ((gEwramData->gameState.stateFlags & 2) && entity->unk_4F8.whole <= 0x34BBD)
        entity->unk_4F8.whole++;
}

/**
 * @brief Update of object 0x12 variant 0 (sObject12Variants[0].updateFunc): idle
 * animation cycle sObject12Variant0AnimScript0, sObject12Variant0AnimScript1, sObject12Variant0AnimScript2 with
 * random waits.
 */
void Object12Variant0_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            Anim_Start(entity, (struct AnimScript *)sObject12Variant0AnimScript0, 0, 0);
            entity->timer = (RandomNumberGenerator() & 0x7F) + 8;
            entity->state++;
        case 1:
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            Anim_Start(entity, (struct AnimScript *)sObject12Variant0AnimScript1, 0, 0);
            entity->state++;
            break;
        case 2:
            if (entity->animFlags & 4)
            {
                entity->timer = (RandomNumberGenerator() & 0x7F) + 0x10;
                entity->state++;
            }
            break;
        case 3:
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            Anim_Start(entity, (struct AnimScript *)sObject12Variant0AnimScript2, 0, 0);
            entity->state++;
            break;
        case 4:
            if (entity->animFlags & 4)
                entity->state = 0;
            break;
    }
    Anim_Update(entity);
}

/**
 * @brief 5140C | Object 0x12 create: load the graphics of variant unk_514,
 * start its animation, layer, and convert room coordinates (flag 2).
 */
void Object12Create(struct EwramData_EntityData *entity)
{
    const struct Object12VariantDefinition *def = &sObject12Variants[entity->unk_514.part16.spawnParam0];

    if (!Entity_LoadGraphics(entity, def->graphics, def->palette, def->paletteIndex, def->spriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    if (def->updateFunc == NULL && !(def->flags & 4))
    {
        Anim_StartIndexed(entity, def->animationIndex, 0, 1);
        if (def->flags & 1)
            entity->objMode = 1;
    }
    entity->spriteLayer = def->spriteLayer;
    if (!(def->flags & 2))
    {
        entity->scrollWithBg = 1;
    }
    else
    {
        u16 *pos;
        struct BgLayerState *bg;

        entity->scrollWithBg = 0;
        pos = &entity->posX.part16.integer;
        bg = &gEwramData->bgInfo[1];
        *pos += bg->xPos.part16.integer;
        pos += 2;
        *pos += bg->yPos.part16.integer;
    }
}

/**
 * @brief 514D8 | Variant with unk_516 == 1 scrolls with background 3
 * instead of background 1; set drawFunc Object12Variant2_Draw on the first frame.
 */
void Object12Variant2_Update(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        if (entity->unk_514.part16.spawnParam1 == 1)
        {
            struct EwramData *ewram = gEwramData;
            struct BgLayerState *mainBg = &ewram->bgInfo[1];
            struct BgLayerState *farBg = &ewram->bgInfo[3];

            entity->posX.whole += mainBg->xPos.whole - farBg->xPos.whole;
        }
        entity->drawFunc = (u32 *)Object12Variant2_Draw;
        entity->state++;
    }
    if (entity->unk_514.part16.spawnParam1 == 1)
    {
        struct BgLayerState *farBg = &gEwramData->bgInfo[3];

        entity->velX.whole = farBg->xVel.whole;
        entity->posX.whole += entity->velX.whole;
    }
}

/**
 * @brief 5153C | Draw frames 0..3 (0 and 1 only while not above the
 * screen), toggling the OBJ mode bit every frame.
 */
void Object12Variant2_Draw(struct EwramData_EntityData *entity)
{
    s32 i;

    entity->objMode = 1 - entity->objMode;
    for (i = 0; i <= 3; i++)
    {
        if ((s16)entity->posY.part16.integer >= -0x10 || i > 1)
        {
            entity->animFrame = i;
            Sprite_Draw(entity);
        }
    }
}

/**
 * @brief 5158C | Object 0x12 update: (only near the screen for flag 8) run
 * the variant's update or the plain animation.
 */
void Object12Update(struct EwramData_EntityData *entity)
{
    const struct Object12VariantDefinition *def = &sObject12Variants[entity->unk_514.part16.spawnParam0];

    if ((def->flags & 8) && !Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120))
        return;
    if (def->updateFunc != NULL)
        def->updateFunc(entity);
    else if (!(def->flags & 4))
        Anim_Update(entity);
}
