/* Lightning Doll: electrical effect at the player's position. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void LightningDollPlayerEffect_Update(struct EwramData_EntityData *entity);
extern s32 Player_GetOverheadY(void);
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

#define ANIM(e,n,loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e),(n),0,(loop)); } while (0)

static inline s32 RandomFlip(s32 mask) { return RandomNumberGenerator() & mask; }
/**
 * @brief Create an electrical effect at the player position minus 16 pixels vertically, with random horizontal flip.
 */
void LightningDoll_SpawnPlayerEffect(void)
{
    s32 i;
    for (i = 0; i < 1; i++)
    {
        struct EwramData_EntityData *effect;
        effect = EntityCreateInRange(0x49, 0xCF, (u32 *)LightningDollPlayerEffect_Update);
        if (effect != NULL)
        {
            effect->unk_500.part8.unk_501 = i;
            effect->posX.whole = gEwramData->playerState.player->posX.whole;
            effect->posY.whole = gEwramData->playerState.player->posY.whole - 0x100000;
            effect->flipX = RandomFlip(1);
            effect->spriteLayer = gEwramData->playerState.player->spriteLayer - 1;
        }
    }
}

/**
 * @brief Attach palette-6 effect graphics and animation 16, alternate facing and follow player X with a timer-dependent Y offset until animation completion.
 */
void LightningDollPlayerEffect_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 graphics = ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 6);
            Anim_StartIndexed(entity, 16, 0, 0);
            Anim_Update(entity);
            entity->scrollWithBg = 1;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->flipX = RandomNumberGenerator() % 2;
            entity->timer = 0;
            entity->state++;
            break;
        }
        case 1:
            if (entity->timer % 2 == 0)
                entity->flipX = 1 - entity->flipX;
            entity->posX.whole = gEwramData->playerState.player->posX.whole;
            entity->posY.part16.integer = Player_GetOverheadY() + 8;
            entity->posY.whole += entity->timer << 15;
            if (entity->animDone)
                entity->deleteRequested = 1;
            entity->timer++;
            Anim_Update(entity);
            break;
    }
}
