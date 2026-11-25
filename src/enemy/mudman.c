/*
 * Mudman: body, thrown shots and pieces.
 */
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "core.h"
#include "sound.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "damage_numbers.h"
#include "entity_motion.h"
#include "entity_hit.h"

void Mudman_UpdateWalk(struct EwramData_EntityData *);
void Mudman_UpdateAttack(struct EwramData_EntityData *);
void Mudman_UpdateDeath(struct EwramData_EntityData *);
void Mudman_UpdateHurt(struct EwramData_EntityData *);
void Mudman_ThrowShot(struct EwramData_EntityData *);
void Mudman_SpawnPiece(struct EwramData_EntityData *, s32, s32, s32, s32);
void MudmanShot_Update(struct EwramData_EntityData *);
u8 MudmanShot_LoadGraphics(struct EwramData_EntityData *, struct EwramData_EntityData *);
void MudmanShot_EnableHitBoxes(struct EwramData_EntityData *);
void MudmanPiece_Update(struct EwramData_EntityData *);
void MudmanShot_DamagePlayerOnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void MudmanShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
u8 Mudman_LoadGraphics(struct EwramData_EntityData *);
void Mudman_DamagePlayerOnContact(struct EwramData_EntityData *entity);

/* Dispatch Mudman states while preserving hit-flash and freeze update gates. */
void EnemyMudmanUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    if (entity->hitFlashTimer && entity->state != 3 && entity->state != 2)
    {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Mudman_UpdateWalk(entity); break;
    case 1: Mudman_UpdateAttack(entity); break;
    case 2: Mudman_UpdateDeath(entity); break;
    case 3: Mudman_UpdateHurt(entity); break;
    case 4: Anim_Update(entity); return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Detect a nearby target, emerge and alternate movement with attacks. */
static inline void Emerge(struct EwramData_EntityData *entity)
{
    entity->subState++;
    entity->animFlags &= ~2;
    entity->animDone = 0;
}
/**
 * @brief Walk state machine (state 0): wait under the ground until the target is
 * in the 120 x 80 box (or as configured by spawnParam1), emerge, then alternate
 * walking toward the target with the mud-throw attack (state 1).
 */
void Mudman_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 substate = entity->subState;
    switch (substate)
    {
    case 0:
        {
            s32 mode;
            entity->flipX = Entity_IsTargetRight(entity);
            mode = entity->spawnParam1;
            if (mode == 0)
            {
                if (Target_IsInRect((s16)(entity->posX.part16.integer - 60),
                                  (s16)(entity->posY.part16.integer - 40), 120, 80))
                    Emerge(entity);
            }
            else
            {
                s32 x;
                if (mode == 1) x = entity->posX.part16.integer + 16;
                else if (mode == 2) x = entity->posX.part16.integer - 76;
                else return;
                if (Target_IsInRect((s16)x, (s16)(entity->posY.part16.integer - 40), 60, 80))
                {
                    Emerge(entity);
                    entity->spawnParam1 = substate;
                }
            }
        }
        break;
    case 1:
        if (entity->animDone)
        {
            entity->subState = 2;
            entity->animDone = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->timer = 100;
            entity->velY.whole = 0x20000;
        }
        break;
    case 2:
        if ((Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 8) & 4) && entity->timer++ > 59)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
            if ((RandomNumberGenerator() & 3) == 0)
            {
                entity->state = 1;
                entity->subState = 0;
                return;
            }
            if (entity->flipX) entity->velX.whole = 0x4000;
            else entity->velX.whole = -0x4000;
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            Anim_StartIndexed(entity, 0, 0, 2);
            entity->subState = 0;
        }
        break;
    }
}

/* Run Mudman's attack animation and timed recovery. */
/**
 * @brief Attack state machine (state 1): animation 3, from its second step
 * Mudman_ThrowShot with sound 0x85; then back to state 0.
 */
void Mudman_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 substate = entity->subState;
    switch (substate)
    {
    case 0:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState++;
        break;
    case 1:
        if (entity->animStep > 1)
        {
            entity->subState = substate + 1;
            entity->timer = 0;
            Mudman_ThrowShot(entity);
            PlaySong(0x85);
        }
        break;
    case 2:
        if (entity->timer++ > 59)
        {
            entity->state = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->animDone = 0;
            entity->timer = 100;
            entity->subState = substate;
            entity->velY.whole = 0x20000;
        }
        break;
    }
}

/* Hit reaction: emit two pieces in the facing direction and recover on landing. */

/* The two velocity locals, offset registers and coordinate-read order preserve
 * agbcc's original allocation. They do not establish the original source form.
 * Keep the first mask expressed as an unsigned remainder: it selects a different
 * destination from the subsequent random mask. */
static inline void EmitLeft(struct EwramData_EntityData *entity)
{
    s32 vx = RandomNumberGenerator() % 0x10000;
    s32 base = -0x1000;
    vx = base - vx;
    {
        s32 vy = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
        register s32 xOffset asm("r9"); /* FAKEMATCH: pinned register */
        register s32 yOffset asm("r8"); /* FAKEMATCH: pinned register */
        s32 x, y;
        x = entity->posX.whole;
        xOffset = -0x60000;
        x += xOffset;
        y = entity->posY.whole;
        yOffset = -0x120000;
        y += yOffset;
        Mudman_SpawnPiece(entity, x, y, vx, vy);
        {
            s32 vx2 = base - (RandomNumberGenerator() & 0x7FFF);
            s32 vy2 = -0x18000 - (RandomNumberGenerator() & 0x7FFF);
            Mudman_SpawnPiece(entity, entity->posX.whole + xOffset, entity->posY.whole + yOffset, vx2, vy2);
        }
    }
}

static inline void EmitRight(struct EwramData_EntityData *entity)
{
    s32 vx = RandomNumberGenerator() % 0x10000;
    s16 base = 0x1000;
    vx += base;
    {
        s32 vy = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
        register s32 xOffset asm("r8"); /* FAKEMATCH: pinned register */
        register s32 yOffset asm("r6"); /* FAKEMATCH: pinned register */
        s32 x, y;
        x = entity->posX.whole;
        xOffset = 0x60000;
        x += xOffset;
        y = entity->posY.whole;
        yOffset = -0x120000;
        y += yOffset;
        Mudman_SpawnPiece(entity, x, y, vx, vy);
        {
            s32 vx2 = RandomNumberGenerator() & 0x7FFF;
            vx2 += base;
            {
                s32 vy2 = -0x18000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole + xOffset, entity->posY.whole + yOffset, vx2, vy2);
            }
        }
    }
}

/**
 * @brief Hurt state machine (state 3): face the target, hop with two mud pieces
 * thrown backwards and return to state 0 on landing.
 */
void Mudman_UpdateHurt(struct EwramData_EntityData *entity)
{
    s32 substate = entity->subState;
    switch (substate)
    {
        case 0:
            entity->flipX = Entity_IsTargetRight(entity);
            entity->velX.whole = substate;
            entity->accelX.whole = substate;
            entity->velY.whole = 0x20000;
            entity->accelY.whole = 0x2800;
            entity->subState++;
            if (entity->flipX)
                EmitLeft(entity);
            else
                EmitRight(entity);
            /* Fall through. */
        case 1:
            if ((Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4) && entity->animDone)
            {
                entity->state = 0;
                entity->subState = 0;
                Anim_StartIndexed(entity, 0, 0, 2);
            }
            break;
    }
}

/* Death: scatter four pieces, wait for landing, then blink and delete. */

/**
 * @brief Death state machine (state 2): fall apart with splashing mud pieces, then
 * request deletion.
 */
void Mudman_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 substate = entity->subState;
    switch (substate)
    {
        case 0:
            entity->flipX = Entity_IsTargetRight(entity);
            entity->velX.whole = substate;
            entity->accelX.whole = substate;
            entity->velY.whole = 0x20000;
            entity->accelY.whole = 0x2800;
            entity->subState++;
            entity->timer = substate;
            {
                s32 vx = -0x1000 - (RandomNumberGenerator() & 0xFFFF);
                s32 vy = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole - 0x60000, entity->posY.whole - 0x100000, vx, vy);
            }
            {
                s32 vx = (RandomNumberGenerator() & 0xFFFF) + 0x1000;
                s32 vy = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole + 0x60000, entity->posY.whole - 0x100000, vx, vy);
            }
            {
                s32 vx = -0x1000 - (RandomNumberGenerator() & 0xFFFF);
                s32 vy = -0x18000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole - 0x60000, entity->posY.whole - 0x100000, vx, vy);
            }
            {
                s32 vx = (RandomNumberGenerator() & 0xFFFF) + 0x1000;
                s32 vy = -0x18000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole + 0x60000, entity->posY.whole - 0x100000, vx, vy);
            }
            /* Fall through. */
        case 1:
            if ((Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4) && entity->animDone)
            {
                entity->subState++;
                entity->timer = 0x28;
            }
            break;
        case 2:
            entity->hidden = Entity_GetVisibilityPattern(entity->timer);
            if ((s8)--entity->timer <= 0)
                entity->deleteRequested = 1;
            break;
    }
}

/* Apply Mudman damage and choose death or a hit-dependent reaction state. */
/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 drop the
 * rewards and die (song 0x70, animation 4, state 2); a hit of type 3 or 4
 * starts the hurt state 3.
 */
void Mudman_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 power = Player_CalculateAttackPower(hit[2]);
    u8 *value554 = &attacker->hit.type;
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, power);
    s32 zero;
    entity->hitFlashTimer = 8;
    {
        s32 hp = entity->hp;

        zero = 0;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x70);
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->animDone = 0;
        entity->palette = entity->basePalette;
        entity->timer = 40;
        entity->state = 2;
        entity->subState = zero;
    }
    else if ((u8)(*value554 - 3) <= 1)
    {
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->animDone = 0;
        entity->state = 3;
        entity->subState = zero;
    }
}

/* Create a Mudman projectile and inherit the parent's visual attributes. */
/**
 * @brief Spawn the mud shot 16 px in front of parent at 2 px per frame in its
 * facing direction.
 */
void Mudman_ThrowShot(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *projectile = EntityCreateInRange(0x1B, 0x48, MudmanShot_Update);
    if (projectile)
    {
        if (!MudmanShot_LoadGraphics(parent, projectile)) EntityDelete(projectile);
        projectile->scrollWithBg = 1;
        MudmanShot_EnableHitBoxes(projectile);
        projectile->spriteLayer = parent->spriteLayer + 1;
        ENTITY_CREATOR(projectile) = parent;
        projectile->flipX = parent->flipX;
        if (parent->flipX)
        {
            projectile->posX.part16.integer = parent->posX.part16.integer + 16;
            projectile->velX.whole = 0x20000;
        }
        else
        {
            projectile->posX.part16.integer = parent->posX.part16.integer - 16;
            projectile->velX.whole = -0x20000;
        }
        projectile->posY.part16.integer = parent->posY.part16.integer - 24;
        projectile->velY.whole = -0x10000;
        projectile->accelX.whole = 0;
        projectile->accelY.whole = 0x1800;
        projectile->spawnIndex = parent->spawnIndex;
        projectile->basePalette = parent->basePalette;
        projectile->attack = parent->attack;
    }
}

/* Handle projectile impacts, debris and attachment to a moving entity. */
static inline void EmitDebris(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy, s32 zero)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, MudmanPiece_Update);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->accelY.whole = 0x2800;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sMudmanGfx);
        Sprite_Attach(part, sMudmanSpriteSheet, graphics, parent->basePalette);
        if (vx > 0) part->animFrame = 21;
        else part->animFrame = 18;
        if (vy > 0) part->flipY = 1;
        else part->flipY = 0;
        part->scrollWithBg = 1;
        part->drawFunc = Sprite_DrawEntity;
        part->timer = zero;
        asm(""); /* FAKEMATCH: keeps jump2 from cross-jumping the two inlined copies */
    }
}
/**
 * @brief Mud shot update: fly with gravity and collision; on a wall or the floor
 * splash into a piece and vanish; state 1 (struck) splashes; state 2
 * sticks to the entity it hit until the splat animation ends.
 */
void MudmanShot_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    struct HitResult result;
    s32 state = entity->state;
    switch (state)
    {
    case 0:
    {
        s32 impact = Entity_MoveWithCollisionDefault(entity, 0x10000, -0x40000);
        if (EntityIsOutsideScreen(entity, 1))
        {
            EntityDelete(entity);
            return;
        }
        if (impact & 12)
        {
            s32 vx = entity->velX.whole / 2;
            EmitDebris(entity, entity->posX.whole, entity->posY.whole, vx, -entity->velY.whole, 0);
            EntityDelete(entity);
            return;
        }
        if (impact & 3)
        {
            entity->state = 2;
            entity->subState = state;
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->animDone = 0;
        }
        break;
    }
    case 1:
        EmitDebris(entity, entity->posX.whole, entity->posY.whole, -entity->velX.whole / 2, 1, 0);
        EntityDelete(entity);
        return;
    case 2:
        switch (entity->subState)
        {
        case 0:
            entity->subState = 1;
            EmitDebris(entity, entity->posX.whole, entity->posY.whole, -entity->velX.whole / 2, 1, 0);
            /* fall through */
        case 1:
        {
            struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
            if (parent && parent->updateFunc)
            {
                entity->posX.whole += parent->posX.whole + background->xPos.whole - entity->ext.mudmanShot.ownerX;
                entity->posY.whole += parent->posY.whole + background->yPos.whole - entity->ext.mudmanShot.ownerY;
                entity->ext.mudmanShot.ownerX = parent->posX.whole + background->xPos.whole;
                entity->ext.mudmanShot.ownerY = parent->posY.whole + background->yPos.whole;
            }
            if (entity->animDone)
            {
                EntityDelete(entity);
                return;
            }
            break;
        }
        }
        break;
    }
    if (entity->state == 0) Hit_CheckWithPlayer(&result, entity);
    Anim_Update(entity);
}

/* MudmanShot_EnableHitBoxes (hit boxes, same code as MermanShot_EnableHitBoxes). */

/**
 * @brief 72434 | Activate box slot 0xA with MudmanShot_DamagePlayerOnContact / MudmanShot_OnHit and set both
 * boxes to the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a ROM
 * address); clear flags bits 1 and 2.
 */
void MudmanShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 0xA, MudmanShot_DamagePlayerOnContact, MudmanShot_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags &= ~4;
}

/* Enter the special reaction state and remember the attacker's world position. */
/**
 * @brief Contact: Player_ApplyContactDamage(attack, 0, 0, 0), remember the entity
 * hit and its room position (ENTITY_OWNER / ext.mudmanShot.ownerX / _Y) and stick to it
 * (state 2).
 */
void MudmanShot_DamagePlayerOnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    struct PlayerContactDamage effect;
    s32 zero;
    effect.attack = entity->attack;
    zero = 0;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    ENTITY_OWNER(entity) = attacker;
    entity->ext.mudmanShot.ownerX = attacker->posX.whole + background->xPos.whole;
    entity->ext.mudmanShot.ownerY = attacker->posY.whole + background->yPos.whole;
    entity->state = 2;
    entity->subState = zero;
    Anim_StartIndexed(entity, 6, 0, 0);
    Anim_Update(entity);
    entity->animDone = 0;
}

/* Create a falling debris sprite, selecting its frame and vertical flip. */
/**
 * @brief Spawn a mud piece at (x, y) moving with (vx, vy) under gravity, frame 21
 * or 18 by direction, flipped vertically when moving down.
 */
void Mudman_SpawnPiece(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, MudmanPiece_Update);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->accelY.whole = 0x2800;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sMudmanGfx);
        Sprite_Attach(part, sMudmanSpriteSheet, graphics, parent->basePalette);
        if (vx > 0) part->animFrame = 21;
        else part->animFrame = 18;
        if (vy > 0) part->flipY = 1;
        else part->flipY = 0;
        part->scrollWithBg = 1;
        part->drawFunc = Sprite_DrawEntity;
        part->timer = 0;
    }
}

/* Advance a debris sprite's motion and its three successive frames. */
/**
 * @brief Mud piece update: fall and step the frame at 6 and 12 frames; delete
 * after 18 frames.
 */
void MudmanPiece_Update(struct EwramData_EntityData *entity)
{
    entity->timer++;
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer == 6) entity->animFrame++;
    if (entity->timer == 12) entity->animFrame++;
    if (entity->timer > 17) EntityDelete(entity);
}

/* MudmanShot_LoadGraphics (Mudman graphics for another entity). */
/* Graphics of Mudman. */

/**
 * @brief 72608 | Give projectile the already loaded Mudman graphics (palette
 * of entity), animation 5; 0 when they are not loaded.
 */
u8 MudmanShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *projectile)
{
    s32 gfx = ObjGfx_Find(sMudmanGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(projectile, sMudmanSpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(projectile, 5, 0, 1);
    Anim_Update(projectile);
    projectile->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* MudmanShot_OnHit. */

/**
 * @brief 72658 | Hit: call Player_CalculateAttackPower on the damage value (result
 * unused) and go to state 1.
 */
void MudmanShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 1;
    entity->subState = 0;
}

/* Mudman begins its spawn animation or the display-mode idle animation. */
void EnemyMudmanCreate(struct EwramData_EntityData *entity)
{
    u8 demo;
    if(!Mudman_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,2);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    demo=*(u8 *)((u8 *)gEwramData+0x8E) & 0x40;
    if(demo)
    {
        entity->state=4;
        Anim_StartIndexed(entity,2,0,1);
        Anim_Update(entity);
    }
    else
    {
        Entity_EnableBoxes(entity,Mudman_DamagePlayerOnContact,Mudman_OnHit);
        Entity_SnapToGround(entity);
        entity->state=demo;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Mudman_LoadGraphics = LegionWalkingCorpse_LoadGraphics
 *   Mudman_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief 726F0 | Same as LegionWalkingCorpse_LoadGraphics.
 */
u8 Mudman_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sMudmanGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sMudmanPalette, 0);
    Sprite_Attach(entity, sMudmanSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 7273C | Same as Enemy_DamagePlayerOnContact.
 */
void Mudman_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
