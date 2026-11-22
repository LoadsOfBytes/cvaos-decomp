/* Man Eater projectile callbacks and Killer Mantle attached effects. */
#include "code_08009A0.h"
#include "killer_mantle_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
// The offsets are passed sign-extended here (u16 in the definition).
extern void Entity_PlaceRelativeToCopy(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, s32 offsetX, s32 offsetY);

/* Graphics of this enemy (see code_080BB664.c). */
extern u8 sKillerMantleGfx[];
extern u8 sKillerMantlePalette[];
extern u8 sKillerMantleSpriteSheet[];

/**
 * @brief Apply stored contact damage with effect type 4 and zero flags,
 * then clear projectile HP so its update spawns a burst.
 */
void ManEaterProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 4;
    contactDamage.unk_4 = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->unk_518.part16.hp = 0;
}

/**
 * @brief Clear projectile HP so its update spawns a burst.
 */
void ManEaterProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->unk_518.part16.hp = 0;
}

/**
 * @brief BB054 | Landing dust of the jump (owner in unk_4F8): sprite 8,
 * layer 0x14, 0x53E bits 1-2 = 1, follows the owner at (-12, 4) until its
 * animation ends.
 */
void KillerMantleLandingEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sKillerMantleGfx, sKillerMantlePalette, 0, sKillerMantleSpriteSheet))
            {
                EntityDelete(entity);
                return;
            }
            Anim_StartIndexed(entity, 8, 0, 0);
            entity->spriteLayer = 0x14;
            entity->objMode = 1;
            entity->state++;
        case 1:
            Entity_PlaceRelativeToCopy(entity, owner, -12, 4);
            Anim_Update(entity);
            if (entity->animFlags & 4)
                EntityDelete(entity);
            break;
    }
}

/**
 * @brief Load an affine part, show and rotate it while the owner attack
 * flag is set, then shrink and hide it. Pause for time stop or owner
 * hit flash; release the affine slot on owner death. Exact visual unresolved.
 */
void KillerMantlePart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    u16 slot;
    s16 angle;

    if (owner->state == 3)
        entity->state = 4;
    else if (entity->state != 0
             && (gEwramData->gameState.timeStopTimer != 0 || owner->unk_510.part8.hitFlashTimer != 0))
        return;
    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sKillerMantleGfx, sKillerMantlePalette, 0, sKillerMantleSpriteSheet))
                goto delete;
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
                goto delete;
            entity->affineIndex = slot;
            entity->affine = 1;
            entity->animFrame = 0x12;
            entity->spriteLayer = owner->spriteLayer + 1;
            entity->hidden = 1;
            entity->state = 1;
        case 1:
            if (!(KILLER_MANTLE_FLAGS(owner) & 1))
                return;
            KILLER_MANTLE_PART_SCALE_X(entity) = 0;
            KILLER_MANTLE_PART_ANGLE(entity) = -0x3000; // 0xD000
            entity->hidden = 0;
            entity->state = 2;
        case 2:
            if (!(KILLER_MANTLE_FLAGS(owner) & 1))
            {
                entity->timer = 8;
                entity->state = 3;
                break;
            }
            if (KILLER_MANTLE_PART_SCALE_X(entity) <= 0xFFFF)
                KILLER_MANTLE_PART_SCALE_X(entity) += 0x1000;
            KILLER_MANTLE_PART_ANGLE(entity) += 0x300;
            Entity_PlaceRelativeToCopy(entity, owner, 8, 0);
            break;
        case 3:
            if (KILLER_MANTLE_PART_SCALE_X(entity) > 0)
                KILLER_MANTLE_PART_SCALE_X(entity) -= 0x2000;
            KILLER_MANTLE_PART_ANGLE(entity) += 0x200;
            Entity_PlaceRelativeToCopy(entity, owner, 8, 0);
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            entity->state = 1;
            entity->hidden = 1;
            break;
        case 4:
            ObjAffine_Free(entity->affineIndex);
        delete:
            EntityDelete(entity);
            return;
    }
    angle = KILLER_MANTLE_PART_ANGLE(entity);
    if (entity->flipX)
        angle = -angle;
    ObjAffine_Set(angle, KILLER_MANTLE_PART_SCALE_X(entity), 0x10000, entity->flipX, 0, entity->affineIndex);
}
