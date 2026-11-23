/* Blue Crow 0x080C9D68..0x080CA04B. The Red Crow functions at 0x080CA308.. are machine-identical
 * copies (see code_080CA308.c). */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void BlueCrow_UpdateMovement(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
/* code_08060B98.h declares s16 parameters; the callers here pass full ints. */
extern void HitEffect_Spawn(s32 x, s32 y, s32 type);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *entity, s32 interval, s32 width, s32 height);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/* Graphics of the two feather (?) entities spawned by BlueCrow_SpawnDeathParts. */
extern u8 sBlueCrowRedCrowGfx[];
extern u8 sBlueCrowRedCrowSpriteSheet[];

void BlueCrow_SpawnDeathParts(struct EwramData_EntityData *entity);
void BlueCrowDeathPart_Update(struct EwramData_EntityData *entity);
void BlueCrow_UpdateBestiaryPreview(struct EwramData_EntityData *entity);

/**
 * @brief Death state (2): spawn two parts, drop rewards and fall with
 * hit/fragment effects. Restore the palette and update animation internally.
 */
void BlueCrow_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            BlueCrow_SpawnDeathParts(entity);
            if (entity->animScriptIndex != 3)
                Anim_StartIndexed(entity, 3, 0, 0);
            entity->accelX.whole = 0;
            entity->velX.whole = 0;
            entity->accelY.whole = 0;
            entity->velY.whole = 0x8000;
            entity->palette = entity->basePalette;
            entity->timer = 0x40;
            entity->subState = 1;
            entity->stage = 0;
            Enemy_DropRewards(entity);
            HitEffect_Spawn((s16)entity->posX.part16.integer, entity->posY.part16.integer, 1);
        case 1:
            entity->posY.whole += entity->velY.whole;
            if ((entity->timer % 16) == 0)
                HitEffect_Spawn((s16)entity->posX.part16.integer, entity->posY.part16.integer + 4, 1);
            Enemy_UpdateFragmentDeathWithSpread(entity, 6, 0x80000, 0x80000);
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Spawn two frame-8 death parts at opposite offsets, copying the
 * owner palette and selecting mirrored movement by index.
 */
void BlueCrow_SpawnDeathParts(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 i;

    for (i = 0; i <= 1; i++)
    {
        child = EntityCreateInRange(0x1B, 0x48, (u32 *)BlueCrowDeathPart_Update);
        if (child == NULL)
            continue;
        if (i == 0)
        {
            child->posX.part16.integer = entity->posX.part16.integer + 8;
            child->posY.part16.integer = entity->posY.part16.integer + 4;
        }
        else
        {
            child->posX.part16.integer = entity->posX.part16.integer - 8;
            child->posY.part16.integer = entity->posY.part16.integer - 4;
        }
        child->spriteLayer = entity->spriteLayer - 1;
        child->scrollWithBg = 1;
        Sprite_Attach(child, sBlueCrowRedCrowSpriteSheet, ObjGfx_Find(sBlueCrowRedCrowGfx), entity->basePalette);
        child->animFrame = 8;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->spawnParam0 = i;
    }
}

/**
 * @brief Move a mirrored death part with horizontal acceleration, blink
 * from the timer bit, then request deletion after two countdowns.
 * Preserve decrement-to-0xFF completion tests. Exact visual unresolved.
 */
void BlueCrowDeathPart_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            entity->velX.whole = 0x4000;
            entity->velY.whole = 0x2000;
            entity->accelX.whole = -0x300;
            if (entity->spawnParam0 != 0)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
                entity->flipX = 1;
            }
            entity->timer = 0x30;
            entity->state++;
        case 1:
            if ((u8)--entity->timer == 0xFF)
            {
                entity->objMode = 1;
                entity->timer = 0x18;
                entity->state++;
            }
            break;
        case 2:
            if ((u8)--entity->timer == 0xFF)
                entity->deleteRequested = 1;
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    entity->hidden = entity->timer % 2;
}

/**
 * @brief C9FA4 | Blue Crow update: state 3 -> BlueCrow_UpdateBestiaryPreview, state 2 ->
 * BlueCrow_UpdateDeath, otherwise the normal behaviour (BlueCrow_UpdateMovement) unless
 * Enemy_IsStunned says otherwise.
 */
void EnemyBlueCrowUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 3)
    {
        BlueCrow_UpdateBestiaryPreview(entity);
    }
    else if (entity->state == 2)
    {
        BlueCrow_UpdateDeath(entity);
    }
    else
    {
        Player_TrackEntity(entity);
        if (Enemy_IsStunned(entity))
        {
            Hit_CheckWithPlayer(collision, entity);
        }
        else
        {
            BlueCrow_UpdateMovement(entity);
            Entity_AnimateWithBoxes(entity);
            Hit_CheckWithPlayer(collision, entity);
        }
    }
}

/**
 * @brief Bestiary state (3): select animation 0 once and update it.
 */
void BlueCrow_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        if (entity->animScriptIndex != 0)
            Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/**
 * @brief Apply enemy damage; on defeat enter death state 2 with the
 * substate counters reset. The death helper owns reward drops.
 */
void BlueCrow_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}
