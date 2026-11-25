/*
 * Skull Archer: death pieces, create, hits, arrows and update (the shared OnHit file has its own CFLAGS: enemy/waiter_skeleton.c).
 */
#include "skull_archer.h"
#include "structs/ewram.h"
#include "graphics.h"
#include "core.h"
#include "sound.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "waiter_skeleton.h"
#include "data/data_118B48.h"

void SkullArcher_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);

/* Enemy helpers decompiled from assembly. */

/* States of SkullArcher (entity->state), named after the update function the dispatcher calls. */
enum {
    SKULL_ARCHER_STATE_IDLE = 0,
    SKULL_ARCHER_STATE_SHOOT = 1,
    SKULL_ARCHER_STATE_PATROL = 2,
    SKULL_ARCHER_STATE_SPAWN_DEATH_PIECES = 3,
    SKULL_ARCHER_STATE_ANIM_ONLY = 4,
};

/* Seven animated fragments when the archer dies. */
void SkullArcher_SpawnDeathPieces(struct EwramData_EntityData *entity)
{
    s32 i;
    PlaySong(0x6B);
    for (i = 5; i <= 11; i++)
    {
        struct EwramData_EntityData *fragment = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, Effect_UpdateSpriteDebris);
        if (fragment != NULL)
        {
            s32 graphics;
            fragment->posX.whole = entity->posX.whole;
            fragment->posY.whole = entity->posY.whole;
            fragment->velX.whole = entity->ext.skullArcher.hitSide * (((RandomNumberGenerator() & 15) << 13) + 0x8000);
            fragment->velY.whole = ((RandomNumberGenerator() & 31) << 12) - 0x38000;
            fragment->accelY.whole = 0x2000;
            fragment->spriteLayer = entity->spriteLayer;
            graphics = ObjGfx_Find(sSkullArcherGfx);
            Sprite_Attach(fragment, sSkullArcherSpriteSheet, graphics, entity->basePalette);
            fragment->flipX = entity->flipX;
            fragment->drawFunc = Sprite_DrawEntity;
            fragment->scrollWithBg = 1;
            fragment->timer = (RandomNumberGenerator() & 15) + 40;
            Anim_StartIndexed(fragment, i, 0, 1);
            Anim_Update(fragment);
        }
    }
}

void EnemySkullArcherCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sSkullArcherGfx, sSkullArcherPalette, 0, sSkullArcherSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = SKULL_ARCHER_STATE_ANIM_ONLY;
        return;
    }
    Enemy_ResetFromTemplate(entity);
    Entity_EnableBoxes(entity, Enemy_OnContact, SkullArcher_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    if (entity->spawnParam0 == 0)
        entity->state = SKULL_ARCHER_STATE_IDLE;
    else
    {
        entity->state = SKULL_ARCHER_STATE_PATROL;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 1, 0, 1);
    }
}

/* SkullArcher_OnHit (hit, knocked away from the attacker). */

/**
 * @brief AFD3C | Hit by attacker: when killed (Enemy_TakeDamage), unk_504 is
 * the direction away from attacker (1 / -1), palette into 0x53E bits 3-6,
 * state 3.
 */
void SkullArcher_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, (s32)attacker, hit))
    {
        Enemy_DropRewards(entity);
        {
            s32 speed = 1;

            entity->ext.skullArcher.hitSide = speed;
            if (entity->posX.whole < attacker->posX.whole)
                entity->ext.skullArcher.hitSide = -speed;
        }
        entity->palette = entity->basePalette;
        entity->state = SKULL_ARCHER_STATE_SPAWN_DEATH_PIECES;
        entity->stage = 0;
        entity->subState = 0;
    }
}

void SkullArcherArrow_OnHit(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

void EnemySkullArcherUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case SKULL_ARCHER_STATE_IDLE: SkullArcher_UpdateIdle(entity); break;
    case SKULL_ARCHER_STATE_SHOOT: SkullArcher_UpdateShoot(entity); break;
    case SKULL_ARCHER_STATE_PATROL: SkullArcher_UpdatePatrol(entity); break;
    case SKULL_ARCHER_STATE_SPAWN_DEATH_PIECES: SkullArcher_SpawnDeathPieces(entity); EntityDelete(entity); return;
    case SKULL_ARCHER_STATE_ANIM_ONLY: Anim_Update(entity); return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}
