/* Chaos: orb halo (3F84/4010). */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
};
struct MarkFrames {
    s32 v[3];
};
struct MarkOffsets {
    struct {
        s32 x;
        s32 y;
    } v[3];
};

extern u8 sChaosSpriteSheet[];
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern struct MarkFrames sChaosMarkFrames;
extern struct MarkOffsets sChaosMarkOffsets;
extern s32 Entity_GetVisibilityPattern(s32);
void ChaosMark_Update(struct EwramData_EntityData *entity);
void ChaosHalo_Update(struct EwramData_EntityData *entity);
extern void ChaosAura_Update(struct EwramData_EntityData *);

#define KIND(e) (*(u8 *)&CHAOS_ORB_KIND((e)))

/* Create the orb's halo (frame 0x13) for `timer` frames. */
void ChaosOrb_CreateHalo(struct EwramData_EntityData *orb, s32 timer)
{
    struct EwramData_EntityData *halo = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosHalo_Update);
    struct ChaosData *data = (struct ChaosData *)CHAOS_DATA(CHAOS_LINK(orb));

    if (halo != NULL)
    {
        s32 graphics;

        halo->posX.part16.integer = 0xFF80;
        halo->posY.part16.integer = 0xFF80;
        halo->spriteLayer = orb->spriteLayer - 1;
        graphics = data->graphics;
        halo->unk_510.part8.basePalette = data->palette2;
        Sprite_Attach(halo, sChaosSpriteSheet, graphics, halo->unk_510.part8.basePalette);
        halo->animMode = 2;
        halo->animFrame = 0x13;
        halo->unk_518.part8.enemyId = orb->unk_518.part8.enemyId;
        CHAOS_LINK(halo) = orb;
        halo->scrollWithBg = 0;
        halo->timer = timer;
        halo->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Halo: blink in under the orb, then show with it until it breaks. */
void ChaosHalo_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *orb = CHAOS_LINK(entity);

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
    if (orb->state != 10)
        return;
hide:
    entity->deleteRequested = 1;
}
