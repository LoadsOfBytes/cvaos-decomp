/* Linked child creation. */
#include "code_08009A0.h"
#include "nemesis_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void NemesisPartner_Update(struct EwramData_EntityData *);
extern u8 sNemesisSpriteSheet[], sNemesisGfx[], sNemesisPalette[];

/**
 * @brief Allocate and attach the linked partner, store mutual slot references and run its first update; return 0 if allocation or sprite attachment fails.
 */
s32 Nemesis_CreatePartner(struct EwramData_EntityData *parent)
{
    void (*update)(struct EwramData_EntityData *) = NemesisPartner_Update;
    struct EwramData_EntityData *e = EntityCreateInRange(0x49, 0xCF, (u32 *)update);

    if (e == NULL)
        return 0;
    if (!Sprite_Attach(e, sNemesisSpriteSheet, ObjGfx_Find(sNemesisGfx), ObjPalette_FindOrLoad(sNemesisPalette)))
        return 0;
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(parent);
    NEMESIS_PARTNER_INDEX(parent) = ENTITY_INDEX(e);
    e->objMode = 1;
    e->drawFunc = (u32 *)Sprite_DrawEntity;
    e->scrollWithBg = 1;
    e->spriteLayer = parent->spriteLayer + 2;
    if (e->state != 1)
    {
        e->state = 1;
        e->stage = 0;
        e->subState = 0;
    }
    update(e);
    return 1;
}
