/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ChaosSoulShot_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "code/code_08060B98.h"

struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
};

extern u8 sChaosGfx[];
extern u8 sChaosPalette[];
extern u8 sChaosSpriteSheet[];
extern s8 ObjPalette_FindOrLoadVariant(void *, u8);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Chaos_UpdateFight(struct EwramData_EntityData *);
extern void Chaos_UpdateIntro(struct EwramData_EntityData *);
extern void Chaos_UpdateDefeat(struct EwramData_EntityData *);
extern void Chaos_UpdateIdle(struct EwramData_EntityData *);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief A42A8 | Same as Enemy_DamagePlayerOnContact.
 */
void ChaosSoulShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* A42D4 | Load the Chaos graphics and its two palettes into the core and
 * its ChaosData (unk_504). */
u8 Chaos_LoadGraphics(struct EwramData_EntityData *core)
{
    s32 graphics;
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;

    graphics = ObjGfx_LoadResource(sChaosGfx, 0);

    if (graphics < 0)
        return 0;
    core->basePalette = ObjPalette_FindOrLoadVariant(sChaosPalette, 0);
    data->graphics = graphics;
    data->palette = core->basePalette;
    data->palette2 = ObjPalette_FindOrLoadVariant(sChaosPalette, 1);
    Sprite_Attach(core, sChaosSpriteSheet, graphics, core->basePalette);
    core->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Chaos core: dispatch the states (0 Chaos_UpdateIntro, 1 Chaos_UpdateFight, 2
 * Chaos_UpdateIdle, 3 Chaos_UpdateDefeat dying, 4 frozen). */
void EnemyChaosUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[48];

    if (entity->state != 3 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 3)
            Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Chaos_UpdateIntro(entity);
        break;
    case 1:
        Chaos_UpdateFight(entity);
        break;
    case 2:
        Chaos_UpdateIdle(entity);
        break;
    case 3:
        Chaos_UpdateDefeat(entity);
        break;
    case 4:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
}
