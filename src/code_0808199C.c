/* Golem_UpdateParts: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code_08009A0.h"
#include "joint_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#define GREAT_ARMOR_PART_COUNT 14

extern void Golem_UpdatePart(struct EwramData_EntityData *entity, s32 index);
extern void Golem_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void GolemIdleEffect_Update(struct EwramData_EntityData *entity);
extern u8 sGolemGfxEnemy50[], sGolemSpriteSheet[];

/**
 * @brief 8199C | Like GreatArmor_UpdateParts, but 11 parts.
 */
void Golem_UpdateParts(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < 11; i++)
            Golem_UpdatePart(entity, order[i]);
    }
}

/* 819C4 | Clear unk_C and unk_10 of part param_1 (like GreatArmor_ResetPartAngles). */
void Golem_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct GolemPart *parts = entity->unk_500.golemParts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/* 819D4 | Pass a hit on to the parent entity unk_500 (Golem_OnDamaged). */
void GolemSegment_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Golem_OnDamaged(JOINT_PART_OWNER(entity), attacker, hit);
}

/* 819E0 | Spawn the debris GolemIdleEffect_Update at the owner + (x, y), one layer in
 * front, facing like the owner. */
/**
 * @brief Create the animation-0 effect at the owner-relative offset,
 * using Flesh Golem graphics and the owner palette/facing. Called
 * from the Flesh Golem idle and bestiary sequences.
 */
void Golem_SpawnIdleEffect(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *debris = Entity_CreateObject((u32 *)GolemIdleEffect_Update);

    if (debris != NULL)
    {
        s32 graphics;

        debris->posX.whole = owner->posX.whole + x;
        debris->posY.whole = owner->posY.whole + y;
        debris->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sGolemGfxEnemy50);
        Sprite_Attach(debris, sGolemSpriteSheet, graphics, owner->unk_510.part8.basePalette);
        Anim_StartIndexed(debris, 0, 0, 0);
        Anim_Update(debris);
        debris->flipX = owner->flipX;
        debris->scrollWithBg = 1;
        debris->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
