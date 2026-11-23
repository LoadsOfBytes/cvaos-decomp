/* Chaos: orb aura (40B4/4184) and the core's companion entity (4238). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern s32 Entity_GetVisibilityPattern(s32);
extern void ChaosBackground_Update(struct EwramData_EntityData *);
void ChaosAura_Update(struct EwramData_EntityData *entity);

#define KIND(e) (*(u8 *)&e->ext.chaosOrb.kind)

/* Create the aura of an orb (palette 6 + pal, random start frame). */
struct EwramData_EntityData *ChaosOrb_CreateAura(struct EwramData_EntityData *orb, s32 pal, s32 kind, s32 timer)
{
    struct EwramData_EntityData *aura = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosAura_Update);

    if (aura != NULL)
    {
        s32 graphics;

        aura->posX.part16.integer = 0xFF80;
        aura->posY.part16.integer = 0xFF80;
        aura->spriteLayer = orb->spriteLayer - 2;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        aura->unk_510.part8.basePalette = pal + 6;
        Sprite_Attach(aura, sCommonObjectSpriteSheet, graphics, aura->unk_510.part8.basePalette);
        Anim_StartIndexed(aura, 6, 0, 1);
        Anim_Update(aura);
        aura->animTimer = RandomNumberGenerator() % 8;
        aura->unk_518.part8.enemyId = orb->unk_518.part8.enemyId;
        aura->ext.chaosChild.link = orb;
        aura->scrollWithBg = 0;
        KIND(aura) = kind;
        aura->timer = timer;
        aura->hit.flags |= 4;
        aura->hit.flags |= 2;
        aura->drawFunc = (u32 *)Sprite_DrawEntity;
        return aura;
    }
    return NULL;
}

/* Aura: blink in at the orb, then flicker with it until it breaks. */
void ChaosAura_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;

    if (orb == NULL || orb->updateFunc == NULL
        || orb->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
        goto hide;
    entity->posX.whole = orb->posX.whole;
    entity->posY.whole = orb->posY.whole + 0x40000;
    if (entity->state == 0)
    {
        if (Entity_GetVisibilityPattern(entity->timer))
            entity->hidden = 0;
        else
            entity->hidden = 1;
        if (--entity->timer <= 1)
            entity->state++;
        return;
    }
    entity->hidden = orb->hidden;
    if (RandomNumberGenerator() & 0x1F)
        Anim_Update(entity);
    if (orb->state != 10)
        return;
hide:
    entity->deleteRequested = 1;
}

/* Create the helper entity ChaosBackground_Update at the core. */
struct EwramData_EntityData *Chaos_CreateBackground(struct EwramData_EntityData *core)
{
    struct EwramData_EntityData *helper = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosBackground_Update);

    if (helper != NULL)
    {
        helper->posX.whole = core->posX.whole;
        helper->posY.whole = core->posY.whole;
        helper->unk_518.part8.enemyId = core->unk_518.part8.enemyId;
        helper->ext.chaosChild.link = core;
        return helper;
    }
    return NULL;
}
