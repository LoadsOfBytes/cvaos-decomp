/* Headhunter: graphics loader (83A40), damage callback (83AC4), slash arcs
 * (83C1C/83EEC) and the transformation parts (83FD0). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sHeadhunterGfx[];
extern u8 sHeadhunterPalette[];
extern u8 sHeadhunterSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Headhunter_SpawnFallingPart(struct EwramData_EntityData *, s32, s32, s32);
extern u32 RandomModuloAtLeast(u32, u32);
extern s32 Entity_AimVelocityAtPointByDistance(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_AimAccelerationAtPoint(struct EwramData_EntityData *, s32, s32, s32);
extern void Effect_SpawnRisingParticleInBox(s32, s32, s32, s32, s32, s32);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);

/* form of the Headhunter: defeat in phases 1 and 2 starts a transformation,
 * the parts of HeadhunterTransformationPart_Update wait for phases 1-3 */
#define PHASE(e) (e->ext.headhunter.formPhase)

/* Load the graphics and the four palettes (unk_504..507, one per form). */
/**
 * @brief Load the graphics and four palette variants, attach the
 * initial sprite and set its draw callback. Return 0 on graphics
 * allocation failure or 1 after successful attachment.
 */
s32 Headhunter_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sHeadhunterGfx, 0);
    if (graphics < 0) return 0;
    entity->ext.headhunter.palette0 = ObjPalette_FindOrLoadVariant(sHeadhunterPalette, 0);
    entity->ext.headhunter.palette1 = ObjPalette_FindOrLoadVariant(sHeadhunterPalette, 1);
    entity->ext.headhunter.palette2 = ObjPalette_FindOrLoadVariant(sHeadhunterPalette, 2);
    entity->ext.headhunter.palette3 = ObjPalette_FindOrLoadVariant(sHeadhunterPalette, 3);
    entity->ext.headhunter.graphics = graphics;
    entity->basePalette = entity->ext.headhunter.palette0;
    Sprite_Attach(entity, sHeadhunterSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Damage: in phases 1 and 2 a defeat starts the transformation (phase 1
 * sheds its first part), otherwise the Headhunter dies. Survived hits
 * sometimes play a voice. */
/**
 * @brief Apply resolved damage and hit flash. HP exhaustion in phases
 * 1 and 2 resets the enemy template and starts the next transformation;
 * otherwise drop rewards and enter defeat state 2.
 */
void Headhunter_ApplyResolvedDamage(struct EwramData_EntityData *entity, s32 unused1, s32 unused2, s32 damage)
{
    s32 zero;
    entity->hitFlashTimer = 8;
    entity->ext.headhunter.hitReact = 1;
    zero = 0;
    if ((s16)(entity->hp -= damage) <= 0)
    {
        if (PHASE(entity) == 1)
        {
            Enemy_ResetFromTemplate(entity);
            PlaySong(0x1C5);
            if (gEwramData->gameState.bossRushMode) entity->attack += 20;
            Anim_StartIndexed(entity, 0x19, 2, 0);
            Anim_Update(entity);
            Headhunter_SpawnFallingPart(entity, entity->flipX ? 0xE0000 : -0xE0000, -0x290000, 0);
            entity->state = zero;
            entity->subState = 6;
        }
        else if (PHASE(entity) == 2)
        {
            Enemy_ResetFromTemplate(entity);
            PlaySong(0x1D3);
            if (gEwramData->gameState.bossRushMode) entity->attack += 20;
            entity->state = zero;
            entity->subState = 20;
        }
        else
        {
            Enemy_DropRewards(entity);
            PlaySong(0x1C0);
            entity->palette = entity->basePalette;
            entity->timer = zero;
            entity->state = 2;
            entity->subState = zero;
        }
    }
    else
    {
        s32 phase = PHASE(entity);
        if (phase == 1)
        {
            if ((RandomNumberGenerator() & 0x7F) < 0x40)
            {
                PlaySong(0x1C4);
                entity->ext.headhunter.hitReact = phase;
            }
            else
                entity->ext.headhunter.hitReact = zero;
            entity->stage = 0;
        }
        else if (phase == 2)
        {
            if ((RandomNumberGenerator() & 0x7F) < 0x40)
                PlaySong(0x1D2);
        }
    }
}

void HeadhunterTransformationArc_Update(struct EwramData_EntityData *entity);

#define KIND(e) (*(s8 *)&e->ext.headhunterChild.kind)
#define ANGLE(e) ((e)->velX.whole)
#define ANGLE_SPEED(e) ((e)->velY.whole)
#define SCALE(e) ((e)->accelX.whole)

/* Create a swinging slash arc of kind 0-2 at the owner + (x, y) (mirrored
 * with the owner for kinds 1 and 2); big selects the larger sprite, dir 1
 * swings the other way. */
/**
 * @brief Create an affine arc for the transformation part and owner,
 * with kind-dependent mirroring, frame, swing and supplied timer.
 * Remove it if affine allocation fails; preserve allocation order.
 */
void Headhunter_SpawnTransformationArc(struct EwramData_EntityData *parent, struct EwramData_EntityData *owner, s32 x, s32 y,
                  s32 big, s32 timer, s32 kind, s32 dir)
{
    struct EwramData_EntityData *arc = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterTransformationArc_Update);
    s32 graphics;
    u16 slot;
    if (arc == NULL) return;
    arc->spriteLayer = owner->spriteLayer + 1;
    graphics = owner->ext.headhunter.graphics;
    arc->basePalette = owner->ext.headhunter.palette1;
    Sprite_Attach(arc, sHeadhunterSpriteSheet, graphics, arc->basePalette);
    if (big == 0) arc->animFrame = 0x60;
    else arc->animFrame = 0x5F;
    arc->animMode = 2;
    if (kind == 0)
    {
        arc->posX.whole = owner->posX.whole + x;
        arc->posY.whole = owner->posY.whole + y;
        if (dir == 1)
        {
            ANGLE(arc) = big == 0 ? -0x2000 : -0x2600;
            ANGLE_SPEED(arc) = -0x2AA;
            arc->flipX = 1;
        }
        else
        {
            ANGLE(arc) = big == 0 ? 0xA000 : 0xA500;
            ANGLE_SPEED(arc) = 0x2AA;
            arc->flipX = 0;
        }
    }
    else if (kind == 1)
    {
        if (owner->flipX) arc->posX.whole = owner->posX.whole - x;
        else arc->posX.whole = owner->posX.whole + x;
        arc->posY.whole = owner->posY.whole + y;
        if (owner->flipX)
        {
            if (dir == 1)
            {
                ANGLE(arc) = big == 0 ? 0xB000 : 0xB100;
                ANGLE_SPEED(arc) = 0x2AA;
                arc->flipX = 0;
            }
            else
            {
                ANGLE(arc) = big == 0 ? 0xF000 : 0xEB00;
                ANGLE_SPEED(arc) = -0x2AA;
                arc->flipX = 1;
            }
        }
        else
        {
            if (dir == 1)
            {
                ANGLE(arc) = big == 0 ? -0x3000 : -0x3100;
                ANGLE_SPEED(arc) = -0x2AA;
                arc->flipX = 1;
            }
            else
            {
                ANGLE(arc) = big == 0 ? 0x9000 : 0x9500;
                ANGLE_SPEED(arc) = 0x2AA;
                arc->flipX = 0;
            }
        }
    }
    else if (kind == 2)
    {
        if (owner->flipX) arc->posX.whole = owner->posX.whole - x;
        else arc->posX.whole = owner->posX.whole + x;
        arc->posY.whole = owner->posY.whole + y;
        if (owner->flipX)
        {
            if (dir == 1)
            {
                ANGLE(arc) = big == 0 ? 0xB000 : 0xB100;
                ANGLE_SPEED(arc) = 0x2AA;
                arc->flipX = 0;
            }
            else
            {
                ANGLE(arc) = big == 0 ? 0xF000 : 0xEB00;
                ANGLE_SPEED(arc) = -0x2AA;
                arc->flipX = 1;
            }
        }
        else
        {
            if (dir == 1)
            {
                ANGLE(arc) = big == 0 ? -0x3000 : -0x3100;
                ANGLE_SPEED(arc) = -0x2AA;
                arc->flipX = 1;
            }
            else
            {
                ANGLE(arc) = big == 0 ? 0x9000 : 0x9500;
                ANGLE_SPEED(arc) = 0x2AA;
                arc->flipX = 0;
            }
        }
    }
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(arc);
        return;
    }
    arc->affineIndex = slot;
    arc->affine = 1;
    ObjAffine_SetBiased(ANGLE(arc) - 0xC000, SCALE(arc), 0x10000, arc->flipX, arc->flipY, arc->affineIndex);
    arc->timer = timer;
    ENTITY_OWNER(arc) = parent;
    arc->ext.headhunterChild.owner = owner;
    KIND(arc) = kind;
    arc->scrollWithBg = 1;
    arc->drawFunc = (u32 *)Sprite_DrawEntity;
}

/* Slash arc: swing and grow for 12 frames, fall after 60 frames (kinds 1
 * and 2 also drift with the owner's facing), vanish after 90. */
/**
 * @brief Swing and grow during the first 12 updates, then drift after
 * timer 59 and free the affine slot after timer 89. The supplied
 * initial timer also contributes to the lifetime.
 */
void HeadhunterTransformationArc_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.headhunterChild.owner;
    if (entity->subState != 0) return;
    entity->timer++;
    if (entity->stage < 12)
    {
        entity->stage++;
        ANGLE(entity) += ANGLE_SPEED(entity);
        SCALE(entity) += 0x1555;
    }
    ObjAffine_SetBiased(ANGLE(entity) - 0xC000, SCALE(entity), SCALE(entity), entity->flipX, entity->flipY, entity->affineIndex);
    if (entity->timer > 59) entity->posY.whole += 0x4000;
    if (KIND(entity) == 1)
    {
        if (entity->timer > 59)
        {
            if (owner->flipX) entity->posX.whole += -0x2000;
            else entity->posX.whole += 0x2000;
        }
    }
    else if (KIND(entity) == 2)
    {
        if (entity->timer > 59)
        {
            if (owner->flipX) entity->posX.whole += -0x2000;
            else entity->posX.whole += 0x2000;
        }
    }
    if (entity->timer > 89)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

#define COUNT(e) (*(s8 *)&e->ext.headhunterPart.count)
#define TARGET_X(e) (e->ext.headhunterPart.targetX)
#define TARGET_Y(e) (e->ext.headhunterPart.targetY)

/* Use palette 4 (TODO: flash?) while the owner does, else our own. */
#define SYNC_PALETTE(entity, owner)                                       \
    if ((owner)->palette == 4) (entity)->palette = 4;                \
    else (entity)->palette = (entity)->basePalette;

/* Transformation part of the Headhunter (entity type 0x6A, the owner):
 * once the owner reaches phase KIND + 1 this part jumps around (two
 * bounces), flies next to the owner, slashes four arcs (Headhunter_SpawnTransformationArc), then
 * either ends the phase-0 transformation or bursts into particles and
 * switches the owner to the next form (palette unk_505 / unk_506). */
/**
 * @brief Wait for the matching owner phase, move through two turning
 * passes and toward its attachment point, emit transformation arcs
 * and advance the owner to its next form or opening movement.
 */
void HeadhunterTransformationPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 i;
    s32 unused[4]; /* only reserves 16 bytes of stack like the original */
    if (entity->state == 0)
    {
        if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x6A)
            goto delete;
        switch (entity->subState)
        {
        case 0:
            switch (KIND(entity))
            {
            case 0:
                if (PHASE(owner) == 1)
                {
                    entity->subState++;
                    COUNT(entity) = 0;
                    entity->timer = 0;
                    entity->flipX = owner->flipX;
                    entity->spriteLayer -= 2;
                }
                break;
            case 1:
                if (PHASE(owner) == 2)
                {
                    entity->subState++;
                    COUNT(entity) = 0;
                    entity->timer = 0;
                    entity->flipX = owner->flipX;
                    entity->spriteLayer -= 2;
                }
                break;
            case 2:
                if (PHASE(owner) == 3)
                {
                    entity->subState++;
                    COUNT(entity) = 0;
                    entity->timer = 0;
                    entity->flipX = owner->flipX;
                    entity->spriteLayer -= 2;
                }
                break;
            }
            break;
        case 1:
            entity->subState++;
        case 2:
            TARGET_X(entity) = owner->posX.whole + camera->xPos.whole
                + (RandomModuloAtLeast((2 - COUNT(entity)) << 19, (2 - COUNT(entity)) << 18) - ((2 - COUNT(entity)) << 18));
            TARGET_Y(entity) = owner->posY.whole + camera->yPos.whole - ((2 - COUNT(entity)) * 0x14 + 0x28) * 0x10000;
            entity->subState++;
        case 3:
        {
            s32 velX;
            u8 changed;
            SYNC_PALETTE(entity, owner);
            Entity_AimAccelerationAtPoint(entity, TARGET_X(entity), TARGET_Y(entity), 0x2000);
            entity->velY.whole = entity->accelY.whole << 3;
            entity->accelY.whole = 0;
            velX = entity->velX.whole;
            EntityAccelerateAndMove(entity, 0x10000);
            changed = ((entity->velX.whole & 0x80000000) ^ (velX & 0x80000000)) != 0;
            if (changed)
            {
                if (++COUNT(entity) <= 1)
                    entity->subState--;
                else
                {
                    entity->subState++;
                    entity->stage = 0;
                }
            }
            break;
        }
        case 4:
            SYNC_PALETTE(entity, owner);
            if (entity->stage == 0)
            {
                s32 x;
                entity->stage++;
                TARGET_X(entity) = x = owner->posX.whole + camera->xPos.whole;
                switch (KIND(entity))
                {
                case 0:
                    TARGET_Y(entity) = owner->posY.whole + camera->yPos.whole - 0x340000;
                    if (owner->flipX) TARGET_X(entity) = x + 0x20000;
                    else TARGET_X(entity) = x - 0x20000;
                    break;
                case 1:
                    TARGET_Y(entity) = owner->posY.whole + camera->yPos.whole - 0x2A0000;
                    if (owner->flipX) TARGET_X(entity) = x + 0x120000;
                    else TARGET_X(entity) = x - 0x120000;
                    break;
                case 2:
                    TARGET_Y(entity) = owner->posY.whole + camera->yPos.whole - 0x280000;
                    if (owner->flipX) TARGET_X(entity) = x + 0x100000;
                    else TARGET_X(entity) = x - 0x100000;
                    break;
                }
            }
            if (Entity_AimVelocityAtPointByDistance(entity, TARGET_X(entity), TARGET_Y(entity), 0x8000))
            {
                entity->subState++;
                entity->timer = 0;
                return;
            }
            EntityAccelerateAndMove(entity, 0x10000);
            break;
        case 5:
        {
            SYNC_PALETTE(entity, owner);
            entity->timer++;
            switch (KIND(entity))
            {
            case 0:
                if (entity->timer == 2) Headhunter_SpawnTransformationArc(entity, owner, -0x50000, -0x200000, 0, entity->timer, 0, 0);
                else if (entity->timer == 5) Headhunter_SpawnTransformationArc(entity, owner, 0x30000, -0x220000, 0, entity->timer, 0, 1);
                else if (entity->timer == 20) Headhunter_SpawnTransformationArc(entity, owner, -0x90000, -0x1C0000, 1, entity->timer, 0, 0);
                else if (entity->timer == 28) Headhunter_SpawnTransformationArc(entity, owner, 0x70000, -0x1B0000, 1, entity->timer, 0, 1);
                else if (entity->timer == 90) entity->subState++;
                if (entity->timer > 59) entity->posY.whole += 0x4000;
                break;
            case 1:
                if (entity->timer == 2) Headhunter_SpawnTransformationArc(entity, owner, -0xD0000, -0x1C0000, 0, entity->timer, 1, 0);
                else if (entity->timer == 5) Headhunter_SpawnTransformationArc(entity, owner, -0xA0000, -0x1E0000, 0, entity->timer, 1, 1);
                else if (entity->timer == 20) Headhunter_SpawnTransformationArc(entity, owner, -0xF0000, -0x160000, 1, entity->timer, 1, 0);
                else if (entity->timer == 28) Headhunter_SpawnTransformationArc(entity, owner, 0, -0x180000, 1, entity->timer, 1, 1);
                else if (entity->timer == 90) entity->subState++;
                if (entity->timer > 59)
                {
                    entity->posY.whole += 0x4000;
                    if (entity->flipX) entity->posX.whole += -0x2000;
                    else entity->posX.whole += 0x2000;
                }
                break;
            case 2:
                if (entity->timer == 2) Headhunter_SpawnTransformationArc(entity, owner, -0xB0000, -0x140000, 0, entity->timer, 2, 0);
                else if (entity->timer == 5) Headhunter_SpawnTransformationArc(entity, owner, -0x80000, -0x160000, 0, entity->timer, 2, 1);
                else if (entity->timer == 20) Headhunter_SpawnTransformationArc(entity, owner, -0xF0000, -0x100000, 1, entity->timer, 2, 0);
                else if (entity->timer == 28) Headhunter_SpawnTransformationArc(entity, owner, 0x10000, -0x110000, 1, entity->timer, 2, 1);
                else if (entity->timer == 90) entity->subState++;
                if (entity->timer > 59)
                {
                    entity->posY.whole += 0x4000;
                    if (entity->flipX) entity->posX.whole += -0x2000;
                    else entity->posX.whole += 0x2000;
                }
                break;
            }
            break;
        }
        case 6:
            SYNC_PALETTE(entity, owner);
            PlaySong(0x11D);
            switch (KIND(entity))
            {
            case 0:
                owner->subState++;
                Anim_StartIndexed(owner, 1, 2, 1);
                Anim_Update(owner);
                goto delete;
            case 1:
                for (i = 0; i < 10; i++)
                    Effect_SpawnRisingParticleInBox(owner->posX.whole, owner->posY.whole - 0x100000, 0x200000, 0x200000, owner->spriteLayer - 1, 1);
                entity->timer = 5;
                entity->subState++;
                break;
            case 2:
                for (i = 0; i < 10; i++)
                    Effect_SpawnRisingParticleInBox(owner->posX.whole, owner->posY.whole - 0x80000, 0x200000, 0x200000, owner->spriteLayer - 1, 1);
                entity->timer = 5;
                entity->subState++;
                break;
            }
            break;
        case 7:
            SYNC_PALETTE(entity, owner);
            switch (KIND(entity))
            {
            case 0:
                break;
            case 1:
                if ((s8)--entity->timer > 0) break;
                owner->basePalette = owner->ext.headhunter.palette1;
                owner->palette = owner->basePalette;
                Anim_StartIndexed(owner, 5, 2, 0);
                Anim_Update(owner);
                owner->subState++;
                owner->posY.whole += -0x130000;
                goto delete;
            case 2:
                if ((s8)--entity->timer > 0) break;
                owner->basePalette = owner->ext.headhunter.palette2;
                owner->palette = owner->basePalette;
                Anim_StartIndexed(owner, 9, 2, 1);
                Anim_Update(owner);
                owner->subState++;
                owner->posY.whole += 0xB0000;
                goto delete;
            }
            break;
        }
        Anim_Update(entity);
        return;
    delete:
        EntityDelete(entity);
    }
    else
        entity->deleteRequested = 1;
}
