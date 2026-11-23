/* Balore's hand: extension/rotation phases and coordinated retreat (B4C7C). */
#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sBalorePalette[], sBaloreSpriteSheet[];
extern const u8 sBaloreHandSteeringParams0[];
extern const u8 sBaloreHandSteeringParams1[];
extern void *Heap_Alloc(u16);
extern void Heap_Free(void *);
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern s32 BaloreHand_FadeIn(struct EwramData_EntityData *);
extern s32 Entity_SteerToPoint(struct EwramData_EntityData *, const void *);
extern s32 BaloreHand_UpdateSwipe(struct EwramData_EntityData *);
extern s32 BaloreHand_UpdateDive(struct EwramData_EntityData *);

/* The two low bytes of enemyStatus: animation byte and flag byte. */
struct HandBytes
{
    u8 anim;
    u8 flags;
};
#define FLAGS(e) (((struct HandBytes *)&(e)->enemyStatus)->flags)
#define ANIM(e) (BALORE_HAND_ANIM((e)))

/* State byte stores through a plain u8 pointer: a struct-member store is
 * expanded as (x & 0) | v, whose zero would be reused for subState. */
#define SET_STATE_BYTE(e, s)                \
    {                                       \
        *(u8 *)&(e)->state = (s);         \
        *(u8 *)&(e)->subState = 0;           \
    }

static inline s32 ToPixels(s32 x)
{
    return x < 0 ? -(-x >> 16) : x >> 16;
}

/**
 * @brief Hand update (first family): fade in, alternate the swipe and dive
 * attacks, then retract; deleted with the owner.
 */
void BaloreHand_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *body = entity->ext.balorePart.body;
    u16 matrix;
    u8 state = entity->state;
    u8 pal, level, index;

    if (state != 0 && (body->ext.balore.flags & 2) && state <= 6)
        entity->state = 7;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
            goto delete;
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->spriteLayer = 0x1A;
        entity->ext.baloreHand.steer = (s32)Heap_Alloc(7);
        entity->basePalette = ObjPalette_FindOrLoadVariant(sBalorePalette, 15);
        Sprite_Attach(entity, sBaloreSpriteSheet, (s8)body->ext.balore.gfx, entity->basePalette);
        Anim_StartIndexed(entity, 0x2D, 2, 0);
        Anim_Update(entity);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        ANIM(entity) = 0x30;
        entity->ext.baloreHand.armLength = 0x260000;
        entity->state = 1;
        break;
    case 1:
        entity->hidden = 0;
        FLAGS(entity) &= 0xFB;
        if (BaloreHand_FadeIn(entity))
            SET_STATE_BYTE(entity, 2);
        break;
    case 2:
    {
        /* follow the body's requested state */
        u8 next = body->ext.balore.handCommand;
        u8 test = next;

        asm("" : "+r"(test)); /* FAKEMATCH */
        if (test != 0 && test != entity->state)
        {
            entity->state = next;
            entity->subState = 0;
        }
        break;
    }
    case 3:
        FLAGS(entity) |= 4;
        *(s32 *)&entity->spawnParam0 = 0x40000;
        entity->state = 4;
    case 4:
        if (Entity_SteerToPoint(entity, sBaloreHandSteeringParams1))
        {
            entity->state = entity->stage;
            entity->subState = 0;
        }
        break;
    case 5:
        FLAGS(entity) |= 4;
        if (BaloreHand_UpdateSwipe(entity))
        {
            SET_STATE_BYTE(entity, 3);
            entity->stage = 6;
        }
        break;
    case 6:
        FLAGS(entity) &= 0xFB;
        if (BaloreHand_UpdateDive(entity))
        {
            SET_STATE_BYTE(entity, 3);
            entity->stage = 2;
        }
        break;
    case 7:
        FLAGS(entity) &= 0xFB;
        if (entity->flipX)
            body->ext.balore.flags |= 8;
        else
            body->ext.balore.flags |= 16;
        if ((body->ext.balore.flags & 24) != 24)
            break;
        entity->timer = 16;
        entity->state = 8;
        entity->subState = 0;
    case 8:
        *(s32 *)&entity->spawnParam0 = 0x40000;
        if (entity->timer != 0)
        {
            entity->timer--;
            pal = entity->basePalette;
            level = 15 - (*(volatile u8 *)&entity->timer >> 1); /* FAKEMATCH: volatile read forces the reload of the original */
            index = pal - 10;
            if (gEwramData->gameState.sharedObjPaletteVariants[index] != level)
            {
                gEwramData->gameState.sharedObjPaletteVariants[index] = level;
                ObjPalette_LoadRange((void *)gEwramData->gameState.sharedObjPaletteSources[index], level, 1, pal);
            }
        }
        if (Entity_SteerToPoint(entity, sBaloreHandSteeringParams0))
        {
            u16 old = body->ext.balore.flags;
            register u32 newFlags asm("r1"); /* FAKEMATCH: result in the constant register */
            newFlags = old | 32;
            newFlags &= 0xFFE7;
            body->ext.balore.flags = newFlags;
            if (newFlags & 2)
                entity->state = 10;
            else
            {
                entity->state = 9;
                entity->hidden = 1;
            }
        }
        break;
    case 9:
        if ((u8)body->ext.balore.handCommand == 1)
        {
            entity->hidden = 0;
            {
                /* the zero for subState is set up right after the address */
                u8 *anim = (u8 *)&entity->enemyStatus;
                u8 zero = 0;

                *anim = 0x30;
                entity->ext.baloreHand.armLength = 0x260000;
                entity->state = (u8)body->ext.balore.handCommand;
                entity->subState = zero;
            }
        }
        break;
    case 10:
        Heap_Free((void *)entity->ext.baloreHand.steer);
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    if (entity->hidden)
        return;
    entity->ext.baloreHand.armScale = (ToPixels(entity->ext.baloreHand.armLength) << 16) / 76;
    ObjAffine_Set(entity->ext.baloreHand.armAngle, entity->ext.baloreHand.armScale, 0x10000, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}
