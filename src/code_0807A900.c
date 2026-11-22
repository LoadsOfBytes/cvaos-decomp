/* Arachne helpers 0x0807A900..0x0807A9EB. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/* Graphics of Arachne. */
extern u8 sArachneGfx[];
extern u8 sArachnePalette[];
extern u8 sArachneSpriteSheet[];

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 param_2, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unk_4 = param_2;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 7A900 | Animate; with timer == 0x80 delete once the animation
 * ended (0x53D bit 0), otherwise count timer down and delete at 0.
 */
void ArachneEffect_Update(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    if (entity->timer == 0x80)
    {
        if (entity->animDone)
            EntityDelete(entity);
    }
    else if ((s8)entity->timer-- <= 0)
    {
        EntityDelete(entity);
    }
}

/**
 * @brief 7A940 | Player_ApplyContactDamage with (attack, 0x200, 0, 0) at the entity
 * position.
 */
void ArachneBeam_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x200;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 7A970 | Load the graphics of Arachne; 0 when no slot is free.
 */
u8 Arachne_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sArachneGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sArachnePalette, 0);
    Sprite_Attach(entity, sArachneSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief Bestiary preview state (state 4): nothing (the caller animates).
 */
void Arachne_UpdatePreview(void)
{
}

/**
 * @brief 7A9C0 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Arachne_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
