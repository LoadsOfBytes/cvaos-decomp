/*
 * Big Golem (boss): body, parts and thrown rocks.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "sound.h"
#include "syscalls.h"
#include "player_level.h"

/* Big Golem update (like EnemyGolemUpdate): screen/hit checks unless dying;
 * states 0 / 1 (BigGolem_UpdateWalk / BigGolem_UpdateAttack), 2 dying: dust every 4th frame
 * while timer is in 0x79..0xD6, then frees the parts; otherwise
 * Entity_ClampXToRoom and a 1-in-4 chance to clear unk_4FE. */

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern void Heap_Free(void *parts);
extern void Effect_SpawnRisingParticle(s32 x, s32 y, s32 layer, s32 mode);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *entity, s32 inset);
extern void BigGolem_UpdateWalk(struct EwramData_EntityData *entity);
extern void BigGolem_UpdateAttack(struct EwramData_EntityData *entity);

void EnemyBigGolemUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[48];

    if (entity->state != 2)
    {
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (entity->state != 2)
            Enemy_UpdateHitFlash(entity);
    }
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
        goto idle;
    switch (entity->state)
    {
    case 0:
        BigGolem_UpdateWalk(entity);
        break;
    case 1:
        BigGolem_UpdateAttack(entity);
        break;
    case 2:
        if (entity->timer > 0x78 && entity->timer < 0xD7 && (entity->timer & 3) == 0)
        {
            s32 x = RandomNumberGenerator() % 0x280000 - 0x140000;
            RandomNumberGenerator();
            Effect_SpawnRisingParticle(entity->posX.whole + x, entity->posY.whole, entity->spriteLayer - 1, 1);
        }
        if ((u8)--entity->timer <= 1)
            entity->deleteRequested = 1;
        if (entity->deleteRequested && entity->ext.golem.parts != NULL)
            Heap_Free(entity->ext.golem.parts);
        return;
    case 3:
        return;
    }
idle:
    Entity_ClampXToRoom(entity, 1);
    if ((RandomNumberGenerator() & 0x7F) < 0x20)
        entity->ext.bigGolem.hit = 0;
}

/* Big Golem create: allocates the 14-joint skeleton and one part entity per joint. */
extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void *memset(void *dst, int value, unsigned long size);
extern u8 BigGolem_LoadGraphics(struct EwramData_EntityData *entity);
extern void *Heap_Alloc(u16 words);
extern void Heap_Free(void *allocation);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void BigGolemPart_Update(struct EwramData_EntityData *entity);
extern void BigGolemPart_OnContact(struct EwramData_EntityData *entity);
extern void BigGolemPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void BigGolem_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void BigGolem_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void BigGolem_AttachPart(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration);
extern void BigGolem_UpdateParts(struct EwramData_EntityData *entity, s8 *order);
extern void BigGolem_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex);
extern void BigGolem_FaceTarget(struct EwramData_EntityData *entity);
extern void Player_SetSelectedTarget(struct EwramData_EntityData *entity);
extern s32 Music_SetCurrent(s32 song);
extern u8 sBigGolemGfx[], sBigGolemGfx2[], sBigGolemSpriteSheet[], sBigGolemSpriteSheet2[];
extern const u8 sBigGolemFrames[14], sBigGolemAnchors[14];
extern const s8 sBigGolemOrder[14], sBigGolemChain[14];
extern const s16 *const sBigGolemPartPoses[];
extern const s32 *const sBigGolemPartAngles[];

static inline s32 Angle(s32 angle)
{
    return angle;
}

void EnemyBigGolemCreate(struct EwramData_EntityData *entity)
{
    u8 frames[14];
    u8 anchors[14];
    s8 order[14];
    s8 chain[14];
    s8 altGraphics[14];
    s8 altPalette[14];
    s32 i;
    s32 index;
    struct EwramData_EntityData *part;
    s32 *parts; /* struct BigGolemPart viewed as 14 words */
    const s16 *pose;
    const s32 *angles;
    s32 frozen;
    struct EwramData **ewram = &gEwramData;

    memcpy(frames, sBigGolemFrames, sizeof(frames));
    memcpy(anchors, sBigGolemAnchors, sizeof(anchors));
    memcpy(order, sBigGolemOrder, sizeof(order));
    memcpy(chain, sBigGolemChain, sizeof(chain));
    memset(altGraphics, 0, sizeof(altGraphics));
    altGraphics[2] = 1;
    altGraphics[8] = 1;
    altGraphics[13] = 1;
    memset(altPalette, 0, sizeof(altPalette));
    altPalette[0] = 1;
    altPalette[1] = 1;
    altPalette[2] = 1;

    if (entity->spawnParam0 != 0 && !(*ewram)->gameState.bossRushMode && ((*ewram)->gameState.bossEventFlags & 0x1000))
    {
        EntityDelete(entity);
        return;
    }
    if (!BigGolem_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.bigGolem.parts = Heap_Alloc(0xC4);
    if (entity->ext.bigGolem.parts == NULL)
    {
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    entity->ext.joint.pose = 0;
    Enemy_ResetFromTemplate(entity);
    if (entity->spawnParam0 != 0 && !gEwramData->gameState.bossRushMode)
        entity->attack = entity->attack - (entity->attack >> 2) - (entity->attack >> 3);
    if (gEwramData->gameState.bossRushMode)
        entity->hp += 1000;
    for (i = 0; i <= 13; i++)
    {
        index = order[i];
        part = Entity_CreateObject((u32 *)BigGolemPart_Update);
        if (part != NULL)
        {
            part->spriteLayer = entity->spriteLayer;
            if (altGraphics[index])
                Sprite_Attach(part, sBigGolemSpriteSheet2, ObjGfx_LoadResource(sBigGolemGfx2, 0), entity->basePalette);
            else
                Sprite_Attach(part, sBigGolemSpriteSheet, ObjGfx_LoadResource(sBigGolemGfx, 0), entity->basePalette);
            if (altPalette[index])
                part->basePalette = entity->ext.bigGolem.palette1;
            else
                part->basePalette = entity->ext.bigGolem.palette0;
            part->enemyId = entity->enemyId;
            part->scrollWithBg = 0;
            part->hidden = 0;
            part->attack = entity->attack;
            part->drawFunc = (u32 *)Sprite_DrawEntity;
            part->ext.jointPart.owner = entity;
            part->ext.jointPart.index = index;
            part->ext.jointPart.anchor = anchors[index];
            part->posX.whole = entity->posX.whole;
            part->posY.whole = entity->posY.whole;
            part->animFrame = frames[index];
            if (index == 2 || index == 4 || index == 5 || index == 8 || index == 12 || index == 13)
            {
                u16 palette = ObjAffine_Alloc(1);
                if ((s16)palette < 0)
                {
                    EntityDelete(part);
                    EntityDelete(entity);
                    return;
                }
                part->affineIndex = palette;
                part->affine = 1;
                ObjAffine_Set(0, 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
            }
            if (!gEwramData->gameState.bestiaryPreview)
                Hit_Enable(part, 8, BigGolemPart_OnContact, (s32)BigGolemPart_OnHit);
        }
        else
        {
            Heap_Free(entity->ext.bigGolem.parts);
            EntityDelete(entity);
            return;
        }
    }
    parts = (s32 *)entity->ext.bigGolem.parts;
    pose = sBigGolemPartPoses[(s8)entity->ext.joint.pose];
    angles = sBigGolemPartAngles[(s8)entity->ext.joint.pose];
    for (index = 0; index <= 13; index++)
    {
        parts[index * 14] = pose[index * 2] << 16;
        parts[index * 14 + 1] = pose[index * 2 + 1] << 16;
        if (index == 5)
            parts[5 * 14 + 2] = angles[0];
        else if (index == 12)
            parts[12 * 14 + 2] = angles[1];
    }
    entity->hidden = 1;
    entity->animFrame = 0;
    frozen = (u8)(*((u8 *)gEwramData + 0x8E) & 0x40);
    if (frozen)
    {
        /* same joint set-up as below; the ROM has it twice */
        entity->timer = 1;
        BigGolem_AttachPart(entity, 1, 3, Angle(0x14000), 0x50000, entity->timer);
        BigGolem_AttachPart(entity, 2, 1, Angle(0x16000), 0x110000, entity->timer);
        BigGolem_AttachPart(entity, 0, 2, Angle(0x13000), 0x300000, entity->timer);
        BigGolem_AttachPart(entity, 7, 3, Angle(0x14000), 0x50000, entity->timer);
        BigGolem_AttachPart(entity, 8, 7, Angle(0x16000), 0x110000, entity->timer);
        BigGolem_AttachPart(entity, 6, 8, Angle(0x13000), 0x300000, entity->timer);
        BigGolem_AttachPart(entity, 10, 9, Angle(0x14000), 0x160000, entity->timer);
        BigGolem_AttachPart(entity, 11, 10, Angle(0x14000), 0xE0000, entity->timer);
        BigGolem_AttachPart(entity, 13, 11, Angle(0x14000), 0x80000, entity->timer);
        BigGolem_AttachPart(entity, 12, 13, Angle(0x15000), 0x2C0000, entity->timer);
        BigGolem_UpdateParts(entity, chain);
        BigGolem_ResetPartAngles(entity, 0);
        BigGolem_ResetPartAngles(entity, 1);
        BigGolem_ResetPartAngles(entity, 2);
        BigGolem_ResetPartAngles(entity, 3);
        BigGolem_ResetPartAngles(entity, 4);
        BigGolem_ResetPartAngles(entity, 5);
        BigGolem_ResetPartAngles(entity, 6);
        BigGolem_ResetPartAngles(entity, 7);
        BigGolem_ResetPartAngles(entity, 8);
        BigGolem_ResetPartAngles(entity, 9);
        BigGolem_ResetPartAngles(entity, 10);
        BigGolem_ResetPartAngles(entity, 11);
        BigGolem_ResetPartAngles(entity, 12);
        BigGolem_ResetPartAngles(entity, 13);
        entity->state = 3;
        entity->subState = 0;
        entity->timer = 0;
        return;
    }
    entity->timer = 1;
    BigGolem_AttachPart(entity, 1, 3, Angle(0x14000), 0x50000, entity->timer);
    BigGolem_AttachPart(entity, 2, 1, Angle(0x16000), 0x110000, entity->timer);
    BigGolem_AttachPart(entity, 0, 2, Angle(0x13000), 0x300000, entity->timer);
    BigGolem_AttachPart(entity, 7, 3, Angle(0x14000), 0x50000, entity->timer);
    BigGolem_AttachPart(entity, 8, 7, Angle(0x16000), 0x110000, entity->timer);
    BigGolem_AttachPart(entity, 6, 8, Angle(0x13000), 0x300000, entity->timer);
    BigGolem_AttachPart(entity, 10, 9, Angle(0x14000), 0x160000, entity->timer);
    BigGolem_AttachPart(entity, 11, 10, Angle(0x14000), 0xE0000, entity->timer);
    BigGolem_AttachPart(entity, 13, 11, Angle(0x14000), 0x80000, entity->timer);
    BigGolem_AttachPart(entity, 12, 13, Angle(0x15000), 0x2C0000, entity->timer);
    BigGolem_UpdateParts(entity, chain);
    BigGolem_ResetPartAngles(entity, 0);
    BigGolem_ResetPartAngles(entity, 1);
    BigGolem_ResetPartAngles(entity, 2);
    BigGolem_ResetPartAngles(entity, 3);
    BigGolem_ResetPartAngles(entity, 4);
    BigGolem_ResetPartAngles(entity, 5);
    BigGolem_ResetPartAngles(entity, 6);
    BigGolem_ResetPartAngles(entity, 7);
    BigGolem_ResetPartAngles(entity, 8);
    BigGolem_ResetPartAngles(entity, 9);
    BigGolem_ResetPartAngles(entity, 10);
    BigGolem_ResetPartAngles(entity, 11);
    BigGolem_ResetPartAngles(entity, 12);
    BigGolem_ResetPartAngles(entity, 13);
    Hit_Enable(entity, 8, BigGolem_DamagePlayerOnContact, (s32)BigGolem_OnHit);
    BigGolem_FaceTarget(entity);
    entity->ext.bigGolem.heading = entity->flipX;
    Entity_SnapToGround(entity);
    entity->state = frozen;
    entity->subState = frozen;
    entity->ext.joint.hurtMask = 0x312C;
    if (entity->spawnParam0 != 0)
    {
        Music_SetCurrent(0x10);
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags |= 2;
    }
}

/* Big Golem walk state machine (state == 0): shifts weight from foot to foot, stomps (dust, shake), turns at walls. */
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *, s32);
extern void Effect_SpawnFadingParticleCopy(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant);
extern void ScreenShake_RequestOffset(u8 priority, s8 offsetX, s8 offsetY);
extern void BigGolem_AttachPartEased(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration);
extern void BigGolem_RecenterOnParts(struct EwramData_EntityData *entity, s32 partIndex0, s32 partIndex1);
extern void BigGolem_SetPartYOffset(struct EwramData_EntityData *entity, s32 partIndex, s32 offsetY);
extern const s8 sBigGolemOrderA[14], sBigGolemOrderB[14];

static inline s32 ABS(s32 x)
{
    if (x < 0)
        x = -x;
    return x;
}

/* Joint A hangs off joint B at ANGLE and distance LEN, reached in timer frames. */
#define J(A, B, ANGLE, LEN) BigGolem_AttachPart(entity, A, B, ANGLE, LEN, entity->timer)
#define JT(A, B, ANGLE, LEN, T) BigGolem_AttachPart(entity, A, B, ANGLE, LEN, T)
#define ARM(A, B, ANGLE, LEN) BigGolem_AttachPartEased(entity, A, B, ANGLE, LEN, 0x96)

/* Standing pose on foot F (knee K, hip H); the other leg is OH, OK, OF. */
#define STAND(F, K, H, OH, OK, OF)\
    J(F, F, 0, 0);\
    J(K, F, 0x1B800, 0x300000);\
    J(H, K, 0x1D800, 0x110000);\
    J(3, H, 0x1B000, 0x50000);\
    J(OH, 3, 0x15000, 0x50000);\
    J(OK, OH, 0x16800, 0x110000);\
    J(OF, OK, 0x14000, 0x300000);\
    J(4, 3, 0x1A000, 0x80000);\
    J(5, 3, 0x1AC00, 0x3A0000);\
    J(9, 3, 0x1B800, 0x260000);

#define ARMS_A()\
    ARM(10, 9, 0x15000, 0x160000);\
    ARM(11, 10, 0x15000, 0xE0000);\
    ARM(13, 11, 0x15000, 0x80000);\
    ARM(12, 13, 0x18000, 0x2C0000);

#define ARMS_B()\
    ARM(10, 9, 0x12000, 0x160000);\
    ARM(11, 10, 0x12000, 0xE0000);\
    ARM(13, 11, 0x12000, 0x80000);\
    ARM(12, 13, 0x13000, 0x2C0000);

#define LEAN(F, K, H, OH, OK, OF, A5, A9)\
    J(F, F, 0, 0);\
    J(K, F, 0x1A800, 0x300000);\
    J(H, K, 0x1D000, 0x110000);\
    J(3, H, 0x1A800, 0x50000);\
    J(OH, 3, 0x15800, 0x50000);\
    J(OK, OH, 0x16400, 0x110000);\
    J(OF, OK, 0x15000, 0x300000);\
    J(4, 3, 0x1A000, 0x80000);\
    J(5, 3, A5, 0x3A0000);\
    J(9, 3, A9, 0x260000);

#define STRIDE(F, K, H, OH, OK, OF, A5, A9)\
    J(F, F, 0, 0);\
    J(K, F, 0x1D000, 0x300000);\
    J(H, K, 0x1E400, 0x110000);\
    J(3, H, 0x1D800, 0x50000);\
    J(OH, 3, 0x12800, 0x50000);\
    J(OK, OH, 0x15000, 0x110000);\
    J(OF, OK, 0x12800, 0x300000);\
    J(4, 3, 0x1A000, 0x80000);\
    J(5, 3, A5, 0x3A0000);\
    J(9, 3, A9, 0x260000);

#define DUST(DX)\
    Effect_SpawnFadingParticleCopy(entity->posX.whole + (DX), entity->posY.whole, (RandomNumberGenerator() & 0x7FFF) + 0x8000,\
                 -0x2000 - (RandomNumberGenerator() & 0x3FFF), entity->spriteLayer - 1, 1);\
    Effect_SpawnFadingParticleCopy(entity->posX.whole + (DX), entity->posY.whole, -0x8000 - (RandomNumberGenerator() & 0x7FFF),\
                 -0x2000 - (RandomNumberGenerator() & 0x3FFF), entity->spriteLayer - 1, 1);

#define WAIT(ORDER)\
    BigGolem_UpdateParts(entity, ORDER);\
    if ((s8)--entity->timer > 0)\
        break;

#define STOMP(ORDER, ANIM)\
    WAIT(ORDER)\
    entity->timer = 0x14;\
    entity->subState++;\
    entity->stage = 0;\
    BigGolem_RecenterOnParts(entity, 0, 6);\
    BigGolem_SetPartYOffset(entity, 0, -0xD0000);\
    BigGolem_SetPartYOffset(entity, 6, -0xD0000);\
    entity->ext.joint.pose = ANIM;

#define STOMP_FX()\
    ScreenShake_RequestOffset(0, 1, 4);\
    PlaySong(0x7C);

#define SHAKE(ORDER)\
    BigGolem_UpdateParts(entity, ORDER);\
    if (entity->timer > 3 && !(entity->timer & 3))\
        ScreenShake_RequestOffset(0, 0, 2);

#define ATTACK()\
    {\
        entity->state = 1;\
        entity->subState = 0;\
        break;\
    }

#define NEAR() (ABS((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= 0x8B)

/* res = 0: walk on, res = 1: turn around. */
#define WALL_CHECK()\
    if ((s8)entity->ext.bigGolem.heading == entity->flipX)\
    {\
        dx = entity->flipX ? 0x6E : -0x6E;\
        if (Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + dx,\
                         bg->yPos.part16.integer + entity->posY.part16.integer)\
            || (!entity->flipX && Entity_GetHorizontalRoomEdge(entity, 0x18) == 1)\
            || (entity->flipX && Entity_GetHorizontalRoomEdge(entity, 0x18) == 2))\
        {\
            res = 1;\
            entity->ext.bigGolem.heading = entity->flipX ^ 1;\
        }\
        else\
            res = 0;\
    }\
    else\
    {\
        s32 hit;\
        dx = entity->flipX ? -0x6E : 0x6E;\
        hit = Tile_Classify(bg->xPos.part16.integer + entity->posX.part16.integer + dx,\
                           bg->yPos.part16.integer + entity->posY.part16.integer);\
        res = 1;\
        if (hit)\
        {\
            res = 0;\
            entity->ext.bigGolem.heading = entity->flipX;\
        }\
    }

#define RANDOM_TURN(CHANCE)\
    if (NEAR())\
    {\
        if ((RandomNumberGenerator2() & 0x7F) < 0x30)\
            ATTACK()\
        if ((RandomNumberGenerator2() & 0x7F) < CHANCE)\
            entity->ext.bigGolem.heading = entity->flipX ^ 1;\
    }\
    if (*(s8 *)&entity->ext.bigGolem.hit && (s8)entity->ext.bigGolem.hitCount > 3)\
    {\
        entity->ext.bigGolem.hit = 0;\
        entity->ext.bigGolem.hitCount = 0;\
        entity->ext.bigGolem.heading = entity->flipX ^ 1;\
    }

/**
 * @brief Walk state machine (state 0): wake when the target is near or the golem
 * is damaged, shift weight from foot to foot, stomp (dust, shake) and turn
 * at walls.
 */
void BigGolem_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 dx, dx2; /* dust offsets; dx is also the wall probe offset. Declared before bg: dx2 gets the lower spill slot. */
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s8 orderA[14];
    s8 orderB[14];
    s32 res;

    memcpy(orderA, sBigGolemOrderA, sizeof(orderA));
    memcpy(orderB, sBigGolemOrderB, sizeof(orderB));

    switch (entity->subState)
    {
    case 0:
        if (!Target_IsNearEntity(entity, 0xF0, 0xA0) && !Enemy_IsDamaged(entity))
            break;
        entity->subState = 1;
        entity->stage = 0;
        entity->timer = 0x28;
        entity->ext.joint.pose = 2;
        STAND(6, 8, 7, 1, 2, 0)
        ARMS_A()
        break;
    case 1:
        WAIT(orderA)
        entity->timer = 0x32;
        entity->subState++;
        entity->ext.joint.pose = 3;
        LEAN(6, 8, 7, 1, 2, 0, 0x1A400, 0x1B000)
        /* fallthrough */
    case 2:
        STOMP(orderA, 3)
        {
            dx = entity->flipX ? 0x180000 : -0x180000;
            DUST(dx)
            DUST(dx * 2)
        }
        STOMP_FX()
        JT(7, 8, 0x1D400, 0x110000, 10);
        JT(1, 3, 0x15C00, 0x50000, 10);
        JT(8, 6, 0x1A600, 0x300000, 10);
        JT(2, 1, 0x16A00, 0x110000, 10);
        JT(0, 2, 0x14D00, 0x300000, 10);
        break;
    case 3:
        SHAKE(orderA)
        if (entity->timer == 10)
        {
            JT(7, 8, 0x1D000, 0x110000, 10);
            JT(1, 3, 0x15800, 0x50000, 10);
            JT(8, 6, 0x1A800, 0x300000, 10);
            JT(2, 1, 0x16400, 0x110000, 10);
            JT(0, 2, 0x15000, 0x300000, 10);
        }
        if ((s8)--entity->timer > 0)
            break;
        RANDOM_TURN(0x20)
        WALL_CHECK()
        if (res == 0)
        {
            entity->timer = 0x50;
            entity->subState++;
            entity->stage = 0;
            entity->ext.joint.pose = 4;
            STAND(0, 2, 1, 7, 8, 6)
            ARMS_B()
        }
        else if (res == 1)
        {
            entity->subState = 10;
            entity->ext.joint.pose = 4;
            entity->timer = 0x32;
            STAND(6, 8, 7, 1, 2, 0)
            ARMS_B()
        }
        break;
    case 4:
        WAIT(orderB)
        entity->timer = 0x32;
        entity->subState++;
        entity->ext.joint.pose = 5;
        LEAN(0, 2, 1, 7, 8, 6, 0x1B400, 0x1C000)
        /* fallthrough */
    case 5:
        STOMP(orderB, 5)
        {
            dx = entity->flipX ? 0x180000 : -0x180000;
            DUST(dx)
            DUST(dx * 2)
        }
        STOMP_FX()
        JT(1, 2, 0x1D400, 0x110000, 10);
        JT(7, 3, 0x15C00, 0x50000, 10);
        JT(2, 0, 0x1A600, 0x300000, 10);
        JT(8, 7, 0x16A00, 0x110000, 10);
        JT(6, 8, 0x14D00, 0x300000, 10);
        break;
    case 6:
        SHAKE(orderB)
        if (entity->timer == 10)
        {
            JT(1, 2, 0x1D000, 0x110000, 10);
            JT(7, 3, 0x15800, 0x50000, 10);
            JT(2, 0, 0x1A800, 0x300000, 10);
            JT(8, 7, 0x16400, 0x110000, 10);
            JT(6, 8, 0x15000, 0x300000, 10);
        }
        if ((s8)--entity->timer > 0)
            break;
        RANDOM_TURN(0x40)
        WALL_CHECK()
        if (res == 0)
        {
            entity->timer = 0x50;
            entity->subState = 1;
            entity->stage = 0;
            entity->ext.joint.pose = 2;
            STAND(6, 8, 7, 1, 2, 0)
            ARMS_A()
        }
        else if (res == 1)
        {
            entity->subState = 7;
            entity->timer = 0x32;
            entity->ext.joint.pose = 2;
            STAND(0, 2, 1, 7, 8, 6)
            ARMS_A()
        }
        break;
    case 7:
        WAIT(orderB)
        entity->timer = 0x50;
        entity->subState++;
        entity->ext.joint.pose = 3;
        STRIDE(0, 2, 1, 7, 8, 6, 0x1A400, 0x1B000)
        break;
    case 8:
        STOMP(orderB, 3)
        {
            if (entity->flipX)
            {
                dx = -0x60000;
                dx2 = -0x1E0000;
            }
            else
            {
                dx = 0x60000;
                dx2 = 0x1E0000;
            }
            DUST(dx)
            DUST(dx2)
        }
        STOMP_FX()
        JT(1, 2, 0x1E800, 0x110000, 10);
        JT(8, 7, 0x15400, 0x110000, 10);
        break;
    case 9:
        SHAKE(orderB)
        if (entity->timer == 10)
        {
            JT(2, 0, 0x1D000, 0x300000, 10);
            JT(1, 2, 0x1E400, 0x110000, 10);
            JT(7, 3, 0x12800, 0x50000, 10);
            JT(8, 7, 0x15000, 0x110000, 10);
            JT(6, 8, 0x12800, 0x300000, 10);
        }
        if ((s8)--entity->timer > 0)
            break;
        if (NEAR() && (RandomNumberGenerator2() & 0x7F) < 0x30)
            ATTACK()
        WALL_CHECK()
        if (res == 0)
        {
            entity->timer = 0x50;
            entity->subState = 4;
            entity->ext.joint.pose = 4;
            STAND(0, 2, 1, 7, 8, 6)
            ARMS_B()
        }
        else if (res == 1)
        {
            entity->subState = 10;
            entity->timer = 0x32;
            entity->ext.joint.pose = 4;
            STAND(6, 8, 7, 1, 2, 0)
            ARMS_B()
        }
        break;
    case 10:
        WAIT(orderA)
        entity->timer = 0x50;
        entity->subState++;
        entity->ext.joint.pose = 5;
        STRIDE(6, 8, 7, 1, 2, 0, 0x1B400, 0x1C000)
        break;
    case 11:
        STOMP(orderA, 5)
        {
            if (entity->flipX)
            {
                dx = -0x60000;
                dx2 = -0x1E0000;
            }
            else
            {
                dx = 0x60000;
                dx2 = 0x1E0000;
            }
            DUST(dx)
            DUST(dx2)
        }
        STOMP_FX()
        JT(7, 8, 0x1E800, 0x110000, 10);
        JT(2, 1, 0x15400, 0x110000, 10);
        break;
    case 12:
        SHAKE(orderA)
        if (entity->timer == 10)
        {
            JT(8, 6, 0x1D000, 0x300000, 10);
            JT(7, 8, 0x1E400, 0x110000, 10);
            JT(1, 3, 0x12800, 0x50000, 10);
            JT(2, 1, 0x15000, 0x110000, 10);
            JT(0, 2, 0x12800, 0x300000, 10);
        }
        if ((s8)--entity->timer > 0)
            break;
        if (NEAR() && (RandomNumberGenerator2() & 0x7F) < 0x30)
            ATTACK()
        WALL_CHECK()
        if (res == 0)
        {
            entity->timer = 0x50;
            entity->subState = 1;
            entity->ext.joint.pose = 2;
            STAND(6, 8, 7, 1, 2, 0)
            ARMS_A()
        }
        else if (res == 1)
        {
            entity->subState = 7;
            entity->timer = 0x32;
            entity->ext.joint.pose = 2;
            STAND(0, 2, 1, 7, 8, 6)
            ARMS_A()
        }
        break;
    }
}

/* Big Golem attack state machine (state == 1): punch (states 0-19, two variants) and rock throw (20-39). */
extern void Effect_SpawnFallingPiece(s32, s32, s32, s32, s32, s32);
extern void BigGolem_SaveParts(struct EwramData_EntityData *entity);
extern void BigGolem_RestoreParts(struct EwramData_EntityData *entity);
extern void BigGolem_SpawnRock(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy);
extern const s8 sBigGolemOrderA[14], sBigGolemOrderB[14], sBigGolemOrderC[14];

/* Joint A hangs off joint B at ANGLE and distance LEN, reached in T frames (A*: eased). */
#define A(A, B, ANGLE, LEN) BigGolem_AttachPartEased(entity, A, B, ANGLE, LEN, entity->timer)
#define AT(A, B, ANGLE, LEN, T) BigGolem_AttachPartEased(entity, A, B, ANGLE, LEN, T)

#define VARIANT (*(s8 *)&entity->ext.bigGolemAttack.variant)
#define ANIM entity->ext.joint.pose

/* Legs for a wind-up on foot F (knee K, hip H); the other leg is OH, OK. */
#define WINDUP_LEGS(F, K, H, OH, OK)\
    A(6, 6, 0, 0);\
    A(K, F, 0x19800, 0x300000);\
    A(H, K, 0x1E000, 0x110000);\
    A(3, H, 0x1A800, 0x50000);\
    A(OH, 3, 0x16000, 0x50000);\
    A(OK, OH, 0x18000, 0x110000);\
    A(0, 0, 0, 0);

#define ARMS(ANGLE)\
    A(10, 9, ANGLE, 0x160000);\
    A(11, 10, ANGLE, 0xE0000);\
    A(13, 11, ANGLE, 0x80000);

#undef LEAN
#define LEAN(F, K, H, OH, OK, OF, A5, A9, ARM)\
    A(F, F, 0, 0);\
    A(K, F, 0x1A800, 0x300000);\
    A(H, K, 0x1D000, 0x110000);\
    A(3, H, 0x1A800, 0x50000);\
    A(OH, 3, 0x15800, 0x50000);\
    A(OK, OH, 0x16400, 0x110000);\
    A(OF, OF, 0, 0);\
    A(4, 3, 0x1A000, 0x80000);\
    A(5, 3, A5, 0x3A0000);\
    A(9, 3, A9, 0x260000);\
    ARMS(ARM)

#define PUNCH_START()\
    J(9, 3, 0x19800, 0x260000);\
    AT(5, 3, 0x18800, 0x3A0000, 0x32);\
    J(10, 9, 0x16000, 0x160000);\
    J(11, 10, 0x16000, 0xE0000);\
    J(13, 11, 0x16000, 0x80000);\
    J(12, 13, 0x16E00, 0x2C0000);

#define PUNCH_START_LOW()\
    J(9, 3, 0x19C00, 0x260000);\
    AT(5, 3, 0x18C00, 0x3A0000, 0x32);\
    J(10, 9, 0x18000, 0x160000);\
    J(11, 10, 0x18000, 0xE0000);\
    J(13, 11, 0x18000, 0x80000);\
    J(12, 13, 0, 0x2C0000);

#define PUNCH_BACK(A9)\
    A(9, 3, A9, 0x260000);\
    ARMS(0x13000)\
    A(12, 13, -0x200, 0x2C0000);

#define PUNCH_BACK_HIGH()\
    A(9, 3, 0x19C00, 0x260000);\
    ARMS(0x12000)\
    A(12, 13, 0x17000, 0x2C0000);

#define ROCK(STEP, CALL)\
    if (entity->flipX)\
    {\
        x = 0x200000;\
        vx = (RandomNumberGenerator() & 0xFFFF) + STEP;\
    }\
    else\
    {\
        x = -0x200000;\
        vx = (RandomNumberGenerator() & 0xFFFF) - 0x10000 - STEP;\
    }\
    CALL;

#define THROW_PREPARE(K, F, H, OH, OK, AOH)\
    J(9, 3, 0x1B000, 0x260000);\
    AT(5, 3, 0x1A000, 0x3A0000, 0x28);\
    AT(10, 9, 0x13000, 0x160000, 0x28);\
    J(11, 10, 0x13000, 0xE0000);\
    J(13, 11, 0x13000, 0x80000);\
    AT(12, 13, -0x3400, 0x2C0000, 0x28);\
    A(6, 6, 0, 0);\
    A(K, F, 0x19C00, 0x300000);\
    A(H, K, 0x1DC00, 0x110000);\
    A(3, H, 0x1AC00, 0x50000);\
    A(OH, 3, AOH, 0x50000);\
    A(OK, OH, 0x17C00, 0x110000);\
    A(0, 0, 0, 0);

#define SETTLE(ORDER)\
    BigGolem_UpdateParts(entity, ORDER);\
    if ((s8)--entity->timer > 4)\
        break;\
    entity->timer = 1;\
    entity->subState++;

#define FINISH(NEXT)\
    entity->timer = 1;\
    entity->subState = NEXT;\
    entity->state = 0;\
    BigGolem_RestoreParts(entity);

/**
 * @brief Attack state machine (state 1): punch (substates 0-19, two variants
 * chosen at random) and rock throw (substates 20-39).
 */
void BigGolem_UpdateAttack(struct EwramData_EntityData *entity)
{
    s8 orderA[14];
    s8 orderB[14];
    s8 orderC[14];
    s32 x;
    s32 vx;

    memcpy(orderA, sBigGolemOrderA, sizeof(orderA));
    memcpy(orderB, sBigGolemOrderB, sizeof(orderB));
    memcpy(orderC, sBigGolemOrderC, sizeof(orderC));

    switch (entity->subState)
    {
    case 0:
        BigGolem_SaveParts(entity);
        if ((RandomNumberGenerator() & 0x7F) < 0x2A)
            entity->ext.bigGolemAttack.variant = 0;
        else if ((RandomNumberGenerator() & 0x7F) < 0x2A)
            entity->ext.bigGolemAttack.variant = 1;
        else
        {
            entity->subState = 0x14;
            break;
        }
        if ((s8)ANIM == 3)
        {
            entity->subState = 1;
            entity->ext.jointAttack.phase = 0;
            entity->timer = 0x78;
            WINDUP_LEGS(6, 8, 7, 1, 2)
            A(4, 3, 0x19000, 0x80000);
            A(5, 3, 0x19400, 0x3A0000);
            A(9, 3, 0x1A000, 0x260000);
            if (!VARIANT)
            {
                ANIM = 6;
                ARMS(0x12000)
                A(12, 13, 0x17000, 0x2C0000);
            }
            else
            {
                ANIM = 0xE;
                ARMS(0x13000)
                A(12, 13, 0x1000, 0x2C0000);
            }
            entity->timer = 0x28;
        }
        else
        {
            entity->subState = 0xB;
            entity->ext.jointAttack.phase = 0;
            entity->timer = 0x78;
            WINDUP_LEGS(0, 2, 1, 7, 8)
            A(4, 3, 0x19000, 0x80000);
            A(5, 3, 0x19400, 0x3A0000);
            A(9, 3, 0x1A000, 0x260000);
            if (!VARIANT)
            {
                ANIM = 0xA;
                ARMS(0x12000)
                A(12, 13, 0x17000, 0x2C0000);
            }
            else
            {
                ANIM = 0x12;
                ARMS(0x13000)
                A(12, 13, 0x6000, 0x2C0000);
            }
            entity->timer = 0x28;
        }
        break;
    case 1:
        WAIT(orderA)
        entity->subState++;
        entity->timer = 0x28;
        AT(5, 3, 0x19400, 0x3A0000, 0x50);
        if (!VARIANT)
            ANIM = 7;
        else
            ANIM = 0xF;
        break;
    case 2:
        WAIT(orderA)
        entity->timer = 0x28;
        entity->subState++;
        AT(5, 3, 0x19400, 0x3A0000, 0x3C);
        if (!VARIANT)
            ANIM = 6;
        else
            ANIM = 0xE;
        break;
    case 3:
        WAIT(orderA)
        entity->timer = 0x18;
        entity->subState++;
        PlaySong(0x86);
        if (!VARIANT)
        {
            ANIM = 8;
            PUNCH_START()
        }
        else
        {
            ANIM = 8;
            PUNCH_START_LOW()
        }
        entity->timer = 0xF;
        break;
    case 4:
        WAIT(orderA)
        entity->timer = 6;
        entity->subState++;
        if (!VARIANT)
        {
            ANIM = 9;
            J(12, 13, 0x17000, 0x2C0000);
        }
        else
        {
            ANIM = 0x11;
            J(12, 13, 0x200, 0x2C0000);
        }
        break;
    case 5:
        WAIT(orderA)
        entity->timer = 0x20;
        entity->subState++;
        if (!VARIANT)
        {
            PUNCH_BACK_HIGH()
        }
        else
        {
            PUNCH_BACK(0x19E00)
        }
        entity->timer = 0x18;
        break;
    case 6:
        WAIT(orderA)
        entity->timer = 8;
        entity->subState++;
        if (!VARIANT)
            ANIM = 9;
        else
            ANIM = 0x11;
        break;
    case 7:
        WAIT(orderA)
        entity->timer = 0x3C;
        entity->subState++;
        ANIM = 3;
        LEAN(6, 8, 7, 1, 2, 0, 0x1A400, 0x1B000, 0x15000)
        if (!VARIANT)
            A(12, 13, 0x18000, 0x2C0000);
        else
            A(12, 13, -0x1000, 0x2C0000);
        break;
    case 8:
        SETTLE(orderA)
        break;
    case 11:
        WAIT(orderB)
        entity->subState++;
        entity->timer = 0x28;
        AT(5, 3, 0x19400, 0x3A0000, 0x50);
        if (!VARIANT)
            ANIM = 0xB;
        else
            ANIM = 0x13;
        break;
    case 12:
        WAIT(orderB)
        entity->timer = 0x28;
        AT(5, 3, 0x19400, 0x3A0000, 0x3C);
        entity->subState++;
        if (!VARIANT)
            ANIM = 0xA;
        else
            ANIM = 0x12;
        break;
    case 13:
        WAIT(orderB)
        entity->timer = 0x18;
        entity->subState++;
        PlaySong(0x86);
        if (!VARIANT)
        {
            ANIM = 0xC;
            PUNCH_START()
        }
        else
        {
            ANIM = 0x14;
            PUNCH_START_LOW()
        }
        entity->timer = 0xF;
        break;
    case 14:
        WAIT(orderB)
        entity->timer = 6;
        entity->subState++;
        if (!VARIANT)
        {
            ANIM = 0xD;
            J(12, 13, 0x17000, 0x2C0000);
        }
        else
        {
            ANIM = 0x15;
            J(12, 13, 0x200, 0x2C0000);
        }
        break;
    case 15:
        WAIT(orderB)
        entity->timer = 0x20;
        entity->subState++;
        if (!VARIANT)
        {
            PUNCH_BACK_HIGH()
        }
        else
        {
            PUNCH_BACK(0x19C00)
        }
        entity->timer = 0x18;
        break;
    case 16:
        WAIT(orderB)
        entity->timer = 8;
        entity->subState++;
        if (!VARIANT)
            ANIM = 0xD;
        else
            ANIM = 0x15;
        break;
    case 17:
        WAIT(orderB)
        entity->timer = 0x64;
        entity->subState++;
        ANIM = 5;
        LEAN(0, 2, 1, 7, 8, 6, 0x1B400, 0x1C000, 0x12000)
        if (!VARIANT)
            A(12, 13, 0x13000, 0x2C0000);
        else
            A(12, 13, -0x6000, 0x2C0000);
        break;
    case 19:
        WAIT(orderB)
        FINISH(6)
        break;
    case 20:
        BigGolem_SaveParts(entity);
        if ((s8)ANIM == 3)
        {
            entity->subState = 0x15;
            entity->ext.jointAttack.phase = 0;
            entity->timer = 0x78;
            WINDUP_LEGS(6, 8, 7, 1, 2)
            A(4, 3, 0x1B000, 0x80000);
            AT(5, 3, 0x1B400, 0x3A0000, 0x78);
            A(9, 3, 0x1C000, 0x260000);
            ANIM = 0x16;
            ARMS(0x15000)
            A(12, 13, 0x2800, 0x2C0000);
            entity->timer = 0x1E;
        }
        else
        {
            entity->subState = 0x1F;
            entity->ext.jointAttack.phase = 0;
            entity->timer = 0x78;
            WINDUP_LEGS(0, 2, 1, 7, 8)
            A(4, 3, 0x1B000, 0x80000);
            A(5, 3, 0x1B400, 0x3A0000);
            A(9, 3, 0x1C000, 0x260000);
            ANIM = 0x1A;
            ARMS(0x15000)
            A(12, 13, 0x7800, 0x2C0000);
            entity->timer = 0x50;
        }
        break;
    case 21:
        WAIT(orderA)
        entity->subState++;
        entity->timer = 0x5A;
        A(5, 3, 0x1B400, 0x3A0000);
        ANIM = 0x17;
        break;
    case 22:
        BigGolem_UpdateParts(entity, orderA);
        if (entity->timer <= 0x2C && !(entity->timer & 0xF))
        {
            ROCK(0, Effect_SpawnFallingPiece(entity->posX.whole + x, entity->posY.whole - 0x680000, vx, 0x1000, entity->spriteLayer + 1, 1))
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x14;
        entity->subState++;
        break;
    case 23:
        WAIT(orderA)
        entity->timer = 0x14;
        entity->subState++;
        ANIM = 0x18;
        THROW_PREPARE(8, 6, 7, 1, 2, 0x16000)
        entity->timer = 0xC;
        break;
    case 24:
        WAIT(orderA)
        entity->timer = 0x28;
        entity->subState++;
        ANIM = 0x19;
        PlaySong(0x71);
        break;
    case 25:
        BigGolem_UpdateParts(entity, orderA);
        if (!(entity->timer & 7))
        {
            ROCK(0x18000, BigGolem_SpawnRock(entity, entity->posX.whole + x, entity->posY.whole - 0x680000, vx, 0x1000))
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 8;
        entity->subState++;
        break;
    case 26:
        WAIT(orderA)
        entity->timer = 8;
        entity->subState++;
        ANIM = 0x19;
        break;
    case 27:
        WAIT(orderA)
        entity->timer = 0x3C;
        entity->subState++;
        ANIM = 3;
        LEAN(6, 8, 7, 1, 2, 0, 0x1A400, 0x1B000, 0x15000)
        A(12, 13, 0x18000, 0x2C0000);
        break;
    case 28:
        SETTLE(orderA)
        break;
    case 9:
    case 29:
        WAIT(orderA)
        FINISH(3)
        break;
    case 31:
        WAIT(orderB)
        entity->subState++;
        entity->timer = 0x5A;
        A(5, 3, 0x1B400, 0x3A0000);
        ANIM = 0x1B;
        break;
    case 32:
        BigGolem_UpdateParts(entity, orderB);
        if (entity->timer <= 0x2C && !(entity->timer & 0xF))
        {
            ROCK(0, Effect_SpawnFallingPiece(entity->posX.whole + x, entity->posY.whole - 0x680000, vx, 0x1000, entity->spriteLayer + 1, 1))
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x14;
        entity->subState++;
        break;
    case 33:
        WAIT(orderB)
        entity->timer = 0x14;
        entity->subState++;
        ANIM = 0x1C;
        THROW_PREPARE(2, 0, 1, 7, 8, 0x15C00)
        entity->timer = 0xC;
        break;
    case 34:
        WAIT(orderB)
        entity->timer = 0x28;
        entity->subState++;
        ANIM = 0x1D;
        PlaySong(0x71);
        break;
    case 35:
        BigGolem_UpdateParts(entity, orderB);
        if (!(entity->timer & 7))
        {
            ROCK(0x18000, BigGolem_SpawnRock(entity, entity->posX.whole + x, entity->posY.whole - 0x680000, vx, 0x1000))
        }
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 8;
        entity->subState++;
        break;
    case 36:
        WAIT(orderB)
        entity->timer = 8;
        entity->subState++;
        ANIM = 0x1D;
        break;
    case 37:
        WAIT(orderB)
        entity->timer = 0x3C;
        entity->subState++;
        ANIM = 5;
        LEAN(0, 2, 1, 7, 8, 6, 0x1B400, 0x1C000, 0x12000)
        A(12, 13, 0x13000, 0x2C0000);
        break;
    case 18:
    case 38:
        SETTLE(orderB)
        break;
    case 39:
        WAIT(orderB)
        FINISH(6)
        break;
    }
}

/* Big Golem joint step: like the golem Golem_UpdatePart, but every part has its
 * own countdown interpolationTimer as divisor and a swing state easingState (1: wait for half
 * of interpolationDuration, 2: slow down until interpolationDuration reaches interpolationDurationLimit). Parts 5 and 12 also
 * turn toward their resting angle. */
void BigGolem_UpdatePart(struct EwramData_EntityData *entity, s32 index)
{
    struct BigGolemPart *parts = entity->ext.bigGolem.parts;
    const s16 *pose = sBigGolemPartPoses[(s8)entity->ext.joint.pose];
    const s32 *angles = sBigGolemPartAngles[(s8)entity->ext.joint.pose];
    s32 x, y, dx, dy, timer;
    struct BigGolemPart *p = (struct BigGolemPart *)(index * sizeof(struct BigGolemPart) + (u32)parts);
    timer = p->interpolationTimer;
    if (timer == 0) timer = 1;
    p->interpolationTimer--;
    if (p->targetSwingAngle == 0 && p->swingAngle == 0)
    {
        struct BigGolemPart *part;
        if (entity->flipX) x = -(pose[index * 2] * 0x10000);
        else x = pose[index * 2] * 0x10000;
        y = pose[index * 2 + 1] * 0x10000;
        part = (struct BigGolemPart *)(index * sizeof(struct BigGolemPart) + (u32)parts);
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / timer;
        part->offsetX = dx;
        dy += y / timer;
        part->offsetY = dy;
        if (index == 5 || index == 12)
        {
            s32 angle;
            if (index == 5) x = angles[0];
            else if (index == 12) x = angles[1];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / timer;
            parts[index].rotationAngle = angle;
        }
    }
    else
    {
        struct BigGolemPart *part = (struct BigGolemPart *)(index * sizeof(struct BigGolemPart) + (u32)parts);
        s32 swing;
        if (part->easingState == 0)
        {
            x = part->targetSwingAngle;
            swing = part->swingAngle;
            x -= swing;
            swing += x / timer;
            part->swingAngle = swing;
            swing += 0x4000;
            x = Sin(swing);
            y = Sin(part->swingAngle);
            x = FixedMultiply(x, part->linkLength);
            y = FixedMultiply(y, part->linkLength);
            if (part->anchorXOrSentinel == (s32)0x80000000)
            {
                part->offsetX = parts[part->anchorYOrPartIndex].offsetX + x;
                part->offsetY = parts[part->anchorYOrPartIndex].offsetY + y;
            }
            else
            {
                part->offsetX = part->anchorXOrSentinel + x;
                part->offsetY = part->anchorYOrPartIndex + y;
            }
            if (index == 5 || index == 12)
            {
                s32 angle;
                if (index == 5) x = angles[0];
                else if (index == 12) x = angles[1];
                if (entity->flipX) x = 0x20000 - x;
                angle = parts[index].rotationAngle;
                x -= angle;
                angle += x / timer;
                parts[index].rotationAngle = angle;
            }
        }
        else
        {
            if (part->easingState == 1)
            {
                if (part->interpolationTimer <= part->interpolationDuration / 2)
                    part->easingState = 2;
            }
            else if (part->easingState == 2)
            {
                part->interpolationTimer++;
                part->interpolationDuration++;
                timer = part->interpolationTimer;
                if (part->interpolationDuration >= part->interpolationDurationLimit)
                    part->easingState = 3;
            }
            part = (struct BigGolemPart *)(index * sizeof(struct BigGolemPart) + (u32)parts);
            x = part->targetSwingAngle;
            swing = part->swingAngle;
            x -= swing;
            swing += x / timer;
            part->swingAngle = swing;
            swing += 0x4000;
            x = Sin(swing);
            y = Sin(part->swingAngle);
            x = FixedMultiply(x, part->linkLength);
            y = FixedMultiply(y, part->linkLength);
            if (part->anchorXOrSentinel == (s32)0x80000000)
            {
                part->offsetX = parts[part->anchorYOrPartIndex].offsetX + x;
                part->offsetY = parts[part->anchorYOrPartIndex].offsetY + y;
            }
            else
            {
                part->offsetX = part->anchorXOrSentinel + x;
                part->offsetY = part->anchorYOrPartIndex + y;
            }
            if (index == 5 || index == 12)
            {
                s32 angle;
                if (index == 5) x = angles[0];
                else if (index == 12) x = angles[1];
                if (entity->flipX) x = 0x20000 - x;
                angle = parts[index].rotationAngle;
                x -= angle;
                angle += x / timer;
                parts[index].rotationAngle = angle;
            }
        }
    }
}

/* BigGolem_FaceTarget (Big Golem turning around). */
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);

/**
 * @brief AC298 | Face the direction given by Entity_IsTargetRight; when turning
 * around mirror the x offsets of all 14 parts and the angle unk_8 of parts
 * 2, 4, 5, 8, 12 and 13.
 */
void BigGolem_FaceTarget(struct EwramData_EntityData *entity)
{
    u32 flipX = entity->flipX;
    struct BigGolemPart *parts;
    s32 i;

    if (flipX != Entity_IsTargetRight(entity))
    {
        entity->flipX ^= 1;
        parts = entity->ext.bigGolem.parts;
        for (i = 0; i < 14; i++)
        {
            parts[i].offsetX = -parts[i].offsetX;
            if (i == 2 || i == 4 || i == 5 || i == 8 || i == 12 || i == 13)
                parts[i].rotationAngle = 0x20000 - parts[i].rotationAngle;
        }
    }
}

/* BigGolem_AttachPartAbsolute (Big Golem joint set-up, variant of CreakingSkull_AttachJointSetAngle on
 * struct BigGolemPart). */

/**
 * @brief AC314 | Hang part partIndex from part anchorIndex: store the parent,
 * the distance (length, or measured when 0), interpolationTimer = interpolationDuration = duration,
 * easingState = 0, the angle to the parent + 0x10000
 * (swingAngle) and the swing base targetSwingAngle from angleSpec (mirrored when facing
 * right); a angleSpec with |angleSpec| > 0xFFFF is an absolute angle, also copied to swingAngle.
 */
void BigGolem_AttachPartAbsolute(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration)
{
    struct BigGolemPart *parts = entity->ext.bigGolem.parts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct BigGolemPart *part = (struct BigGolemPart *)(partIndex * sizeof(struct BigGolemPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct BigGolemPart *anglePart;

    part->easingState = 0;
    part->interpolationTimer = duration;
    part->interpolationDuration = duration;
    part->anchorXOrSentinel = 0x80000000;
    part->anchorYOrPartIndex = anchorIndex;
    if (length != 0)
    {
        part->linkLength = length;
    }
    else
    {
        dx = parts[anchorIndex].offsetX - part->offsetX;
        dy = parts[anchorIndex].offsetY - part->offsetY;
        dx = FixedMultiply(dx, dx);
        dy = FixedMultiply(dy, dy);
        part->linkLength = (u32)(Sqrt(dx + dy) << 16) >> 8;
    }
    dx = parts[partIndex].offsetX - parts[anchorIndex].offsetX;
    dy = parts[partIndex].offsetY - parts[anchorIndex].offsetY;
    angle = ArcTan2(dx < 0 ? -(-dx >> 16) : dx >> 16, dy < 0 ? -(-dy >> 16) : dy >> 16);
    anglePart = (struct BigGolemPart *)(partIndex * sizeof(struct BigGolemPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if ((angleSpec < 0 ? -angleSpec : angleSpec) > 0xFFFF)
    {
        if (entity->flipX)
        {
            if (angleSpec > 0x18000)
                anglePart->targetSwingAngle = 0x38000 - angleSpec;
            else
                anglePart->targetSwingAngle = 0x28000 - angleSpec;
        }
        else
        {
            anglePart->targetSwingAngle = angleSpec;
        }
        {
            struct BigGolemPart *absolutePart = (struct BigGolemPart *)(partIndex * sizeof(struct BigGolemPart) + (u32)parts);

            absolutePart->swingAngle = absolutePart->targetSwingAngle;
        }
    }
    else if (entity->flipX)
    {
        anglePart->targetSwingAngle = baseAngle - angleSpec;
    }
    else
    {
        anglePart->targetSwingAngle = baseAngle + angleSpec;
    }
}

/* BigGolem_AttachPart (Big Golem joint set-up, like CreakingSkull_AttachJoint on struct
 * BigGolemPart). */

/**
 * @brief AC468 | Hang part partIndex from part anchorIndex: store the parent,
 * the distance (length, or measured when 0), interpolationTimer = interpolationDuration = duration,
 * easingState = 0, the angle to the parent + 0x10000
 * (swingAngle) and the swing base targetSwingAngle from angleSpec (mirrored when facing
 * right).
 */
void BigGolem_AttachPart(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration)
{
    struct BigGolemPart *parts = entity->ext.bigGolem.parts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct BigGolemPart *part = (struct BigGolemPart *)(partIndex * sizeof(struct BigGolemPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct BigGolemPart *anglePart;

    part->easingState = 0;
    part->interpolationTimer = duration;
    part->interpolationDuration = duration;
    part->anchorXOrSentinel = 0x80000000;
    part->anchorYOrPartIndex = anchorIndex;
    if (length != 0)
    {
        part->linkLength = length;
    }
    else
    {
        dx = parts[anchorIndex].offsetX - part->offsetX;
        dy = parts[anchorIndex].offsetY - part->offsetY;
        dx = FixedMultiply(dx, dx);
        dy = FixedMultiply(dy, dy);
        part->linkLength = (u32)(Sqrt(dx + dy) << 16) >> 8;
    }
    dx = parts[partIndex].offsetX - parts[anchorIndex].offsetX;
    dy = parts[partIndex].offsetY - parts[anchorIndex].offsetY;
    angle = ArcTan2(dx < 0 ? -(-dx >> 16) : dx >> 16, dy < 0 ? -(-dy >> 16) : dy >> 16);
    anglePart = (struct BigGolemPart *)(partIndex * sizeof(struct BigGolemPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if ((angleSpec < 0 ? -angleSpec : angleSpec) > 0xFFFF)
    {
        if (entity->flipX)
        {
            if (angleSpec > 0x18000)
                anglePart->targetSwingAngle = 0x38000 - angleSpec;
            else
                anglePart->targetSwingAngle = 0x28000 - angleSpec;
        }
        else
        {
            anglePart->targetSwingAngle = angleSpec;
        }
    }
    else if (entity->flipX)
    {
        anglePart->targetSwingAngle = baseAngle - angleSpec;
    }
    else
    {
        anglePart->targetSwingAngle = baseAngle + angleSpec;
    }
}

/* Big Golem damage and contact callbacks. */
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void BossDefeatEffect_Spawn(s32, s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);

/* Flash and reduce the shared HP; end the boss encounter at zero HP. */
void BigGolem_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage)
{
    entity->hitFlashTimer = 8;
    entity->hp -= damage;
    if ((s8)entity->ext.bigGolem.hitCount < 10)
        entity->ext.bigGolem.hitCount++;
    entity->ext.bigGolem.hit = 1;
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        Anim_StartIndexed(entity, 3, 0, 1);
        PlaySong(0x6E);
        entity->palette = entity->basePalette;
        entity->timer = 60;
        entity->state = 2;
        entity->subState = 0;
        if (entity->spawnParam0 != 0)
        {
            BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
            BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 32, 12, 96, 0);
            Player_SetSelectedTarget(NULL);
        }
    }
}

/* Contact from parts 12/13 during the parent's early attack substates
 * also pushes the player horizontally away and upwards. */
void BigGolemPart_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contact;
    struct EwramData_EntityData *parent = entity->ext.jointPart.owner;
    contact.attack = entity->attack;
    contact.attributes = 0;
    contact.unused = 0;
    if (parent->state == 1 && parent->subState <= 19 && (u8)(entity->ext.jointPart.index - 12) <= 1)
    {
        s32 vx;
        contact.reaction = 3;
        /* Original call occurs before the contact coordinates are assigned
         * below. Preserve this order; revisit when making a portable runtime. */
        Player_ApplyContactDamage(&contact);
        vx = -0x40000;
        if (gEwramData->playerState.player->posX.whole > entity->posX.whole)
            vx = -vx;
        Player_SetKnockbackVelocity(vx, -0x24000);
    }
    else
        contact.reaction = 0;
    contact.sourceX = entity->posX.whole;
    contact.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contact);
}

/* Big Golem part update: joint following, weapon collision and death motion. */
struct GolemBox { s8 x, y, w, h; };
extern void *memcpy(void *, const void *, unsigned long);
extern const u8 sBigGolemPartTimers[14];
extern const s16 sBigGolemPartOffsets[14];
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
#define JOINT(e) ((s8)e->ext.jointPart.index)
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)
#define HAS_SPRITE(e) (JOINT(e) == 2 || JOINT(e) == 4 || JOINT(e) == 5 || JOINT(e) == 8 || JOINT(e) == 12 || JOINT(e) == 13)
#define HAD_SPRITE(e) (JOINT(e) == 2 || JOINT(e) == 5 || JOINT(e) == 8 || JOINT(e) == 11 || JOINT(e) == 12 || JOINT(e) == 13)

/**
 * @brief Part entity update: follow its joint of the parent golem (palette,
 * flip, position), handle the weapon/contact boxes and the death motion.
 */
void BigGolemPart_Update(struct EwramData_EntityData *entity)
{
    u8 hit[16];
    u8 timers[14];
    s16 offsets[14];
    struct GolemBox box;
    struct EwramData_EntityData *parent = entity->ext.bigGolemPart.owner;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct BigGolemPart *parts;
    s32 x, y, size;

    memcpy(timers, sBigGolemPartTimers, sizeof(timers));
    memcpy(offsets, sBigGolemPartOffsets, sizeof(offsets));
    if (entity->state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->enemyId == entity->enemyId)
        {
            parts = parent->ext.bigGolem.parts;
            if (parent->palette == 4)
                entity->palette = 4;
            else
                entity->palette = entity->basePalette;
            entity->flipX = parent->flipX;
            entity->timer++;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (JOINT(entity) == 2 || JOINT(entity) == 4 || JOINT(entity) == 8 || JOINT(entity) == 13)
            {
                u16 angle;

                x = parts[(s8)entity->ext.jointPart.anchor].offsetX;
                x -= parts[JOINT(entity)].offsetX;
                y = parts[(s8)entity->ext.jointPart.anchor].offsetY;
                y -= parts[JOINT(entity)].offsetY;
                angle = ArcTan2(FIX_INT(x), FIX_INT(y));
                if (JOINT(entity) == 4)
                {
                    if (entity->flipX)
                        entity->velX.whole = angle + 0x2700;
                    else
                        entity->velX.whole = angle + 0x5900;
                }
                else
                    entity->velX.whole = angle - 0x4000;
                parts[JOINT(entity)].rotationAngle = entity->velX.whole;
            }
            else if (JOINT(entity) == 5 || JOINT(entity) == 12)
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->ext.bigGolemPart.angle = entity->velX.whole;
                entity->ext.bigGolemPart.spin = (RandomNumberGenerator() & 0xFF) - 0x80;
                entity->ext.bigGolemPart.floorY = parent->posY.part16.integer + (s16)camera->yPos.part16.integer + offsets[JOINT(entity)];
                entity->accelY.whole = 0x1800;
                entity->accelX.whole = 0;
                entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) - 0x2000;
                entity->velY.whole = 0;
                entity->timer = timers[JOINT(entity)];
                return;
            }
            if ((parent->ext.joint.hurtMask >> JOINT(entity)) & 1)
            {
                if (JOINT(entity) == 2 || JOINT(entity) == 8 || JOINT(entity) == 13)
                {
                    size = ((entity->timer & 1) + 1) * 21;
                    if (entity->flipX)
                    {
                        x = Sin(0x8000 - parts[JOINT(entity)].rotationAngle);
                        y = Sin(0x4000 - parts[JOINT(entity)].rotationAngle);
                    }
                    else
                    {
                        x = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                        y = Sin(parts[JOINT(entity)].rotationAngle + 0x4000);
                    }
                    box.x = FIX_INT(size * x) - 10;
                    box.y = FIX_INT(size * y) - 10;
                    box.w = 20;
                    box.h = 20;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    HitBox_SetBody(parent, &box);
                    entity->hit.flags &= 0xFB;
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 3)
                {
                    box.x = -16;
                    box.y = -64;
                    box.w = 32;
                    box.h = 90;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    HitBox_SetBody(parent, &box);
                    entity->hit.flags &= 0xFB;
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 5)
                {
                    box.x = -12;
                    box.y = -10;
                    box.w = 24;
                    box.h = 48;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    HitBox_SetBody(parent, &box);
                    entity->hit.flags &= 0xFB;
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 12)
                {
                    size = 21;
                    if (entity->flipX)
                    {
                        x = Sin(0x8000 - parts[JOINT(entity)].rotationAngle);
                        y = Sin(0x4000 - parts[JOINT(entity)].rotationAngle);
                    }
                    else
                    {
                        x = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                        y = Sin(parts[JOINT(entity)].rotationAngle + 0x4000);
                    }
                    box.x = FIX_INT(size * x) - 9;
                    box.y = FIX_INT(size * y) - 9;
                    box.w = 18;
                    box.h = 18;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    HitBox_SetBody(parent, &box);
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
            }
            else
            {
                entity->hit.flags |= 4;
                entity->hit.flags |= 2;
            }
            if (HAS_SPRITE(entity))
                ObjAffine_Set(entity->velX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            return;
        }
        if (HAD_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
            if (JOINT(entity) == 13) entity->subState = 10;
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->timer <= 60)
        {
            EntityAccelerateAndMove(entity, 0x60000);
            entity->ext.bigGolemPart.angle += entity->ext.bigGolemPart.spin;
            if (entity->subState == 1
                && entity->posY.part16.integer + (s16)camera->yPos.part16.integer >= entity->ext.bigGolemPart.floorY)
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->ext.bigGolemPart.spin = 0;
                entity->subState++;
                if ((u8)(entity->ext.jointPart.index - 1) > 2
                    && entity->ext.jointPart.index != 7)
                    PlaySong(0x71);
            }
        }
        if (entity->subState == 2 && parent->spawnParam0 == 0 && (entity->timer & 7) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x40000, 0x100000, 0x100000, entity->spriteLayer - 1);
        if (HAS_SPRITE(entity))
            ObjAffine_Set(entity->ext.bigGolemPart.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if (--entity->timer <= 1)
        {
            if (HAS_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
            if (JOINT(entity) == 5) PlaySong(0x6E);
            EntityDelete(entity);
        }
        else if (!entity->hidden)
        {
            if (entity->posY.part16.integer < -24) entity->hidden = 1;
            else if (entity->posY.part16.integer > 240) entity->hidden = 1;
        }
    }
}

/* Big Golem: falling projectile with a hit callback owned by the boss. */
extern void BigGolemRock_Update(struct EwramData_EntityData *);
extern void BigGolemRock_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void BigGolemRock_OnKilled(struct EwramData_EntityData *, s32, u16 *);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Spawn a thrown rock at (x, y) with velocity (vx, vy): BigGolemRock_Update,
 * shared object graphics and a contact box (BigGolemRock_DamagePlayerOnContact);
 * deleted again when the graphics are missing.
 */
void BigGolem_SpawnRock(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy)
{
    u32 collision = 0x0808FCFC;
    struct EwramData_EntityData *shot = EntityCreateInRange(0x1B, 0x48, (u32 *)BigGolemRock_Update);
    if (shot != NULL)
    {
        s32 tiles = ObjGfx_Find(sCommonObjectGfx);
        if (tiles < 0)
        {
            EntityDelete(shot);
            return;
        }
        Sprite_Attach(shot, sCommonObjectSpriteSheet, tiles, 6);
        shot->animFrame = 0x42;
        shot->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(shot, 10, BigGolemRock_DamagePlayerOnContact, (s32)BigGolemRock_OnKilled);
        HitBox_SetAttack(shot, &collision);
        HitBox_SetBody(shot, &collision);
        shot->hit.flags &= ~4;
        shot->hit.flags &= ~2;
        shot->scrollWithBg = 1;
        shot->spriteLayer = parent->spriteLayer + 1;
        ENTITY_OWNER(shot) = parent;
        shot->posX.whole = x;
        shot->posY.whole = y;
        shot->velX.whole = vx;
        shot->velY.whole = vy;
        shot->accelY.whole = 0x2000;
        shot->flipX = parent->flipX;
        shot->attack = parent->attack;
    }
}

/* Big Golem body-part helpers 0x080ACEC4..0x080AD00B. */
#define BIG_GOLEM_PART_COUNT 14

/**
 * @brief ACEC4 | Move the entity by the mean x of parts partIndex0 and partIndex1
 * and shift all parts back by the same amount.
 */
void BigGolem_RecenterOnParts(struct EwramData_EntityData *entity, s32 partIndex0, s32 partIndex1)
{
    struct BigGolemPart *parts = entity->ext.bigGolem.parts;
    s32 shift = (parts[partIndex0].offsetX + parts[partIndex1].offsetX) / 2;
    s32 i;

    entity->posX.whole += shift;
    for (i = BIG_GOLEM_PART_COUNT - 1; i >= 0; parts++, i--)
        parts->offsetX -= shift;
}

/**
 * @brief ACF00 | Set the y offset of part partIndex.
 */
void BigGolem_SetPartYOffset(struct EwramData_EntityData *entity, s32 partIndex, s32 offsetY)
{
    entity->ext.bigGolem.parts[partIndex].offsetY = offsetY;
}

/**
 * @brief ACF10 | Unless timer is 0, run BigGolem_UpdatePart on parts 0..13.
 */
void BigGolem_UpdateAllParts(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < BIG_GOLEM_PART_COUNT; i++)
            BigGolem_UpdatePart(entity, i);
    }
}

/**
 * @brief ACF30 | Unless timer is 0, run BigGolem_UpdatePart on the 14 parts
 * listed in order.
 */
void BigGolem_UpdateParts(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < BIG_GOLEM_PART_COUNT; i++)
            BigGolem_UpdatePart(entity, order[i]);
    }
}

/**
 * @brief Clear the swing base targetSwingAngle and angle swingAngle of part index.
 */
void BigGolem_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct BigGolemPart *parts = entity->ext.bigGolem.parts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief ACF6C | BigGolem_AttachPartAbsolute with half of duration, then set easingState = 1 and
 * interpolationDurationLimit = duration of part partIndex.
 */
void BigGolem_AttachPartAbsoluteEased(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration)
{
    struct BigGolemPart *parts;

    BigGolem_AttachPartAbsolute(entity, partIndex, anchorIndex, angleSpec, length, duration / 2);
    parts = entity->ext.bigGolem.parts;
    parts[partIndex].easingState = 1;
    parts[partIndex].interpolationDurationLimit = duration;
}

/**
 * @brief ACFA0 | Same as BigGolem_AttachPartAbsoluteEased with BigGolem_AttachPart.
 */
void BigGolem_AttachPartEased(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration)
{
    struct BigGolemPart *parts;

    BigGolem_AttachPart(entity, partIndex, anchorIndex, angleSpec, length, duration / 2);
    parts = entity->ext.bigGolem.parts;
    parts[partIndex].easingState = 1;
    parts[partIndex].interpolationDurationLimit = duration;
}

/**
 * @brief ACFD4 | Save the offsets of all parts (offsetX/offsetY -> savedOffsetX/savedOffsetY).
 */
void BigGolem_SaveParts(struct EwramData_EntityData *entity)
{
    struct BigGolemPart *parts = entity->ext.bigGolem.parts;
    s32 i;

    for (i = BIG_GOLEM_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->savedOffsetX = parts->offsetX;
        parts->savedOffsetY = parts->offsetY;
    }
}

/**
 * @brief ACFF0 | Restore the offsets saved by BigGolem_SaveParts.
 */
void BigGolem_RestoreParts(struct EwramData_EntityData *entity)
{
    struct BigGolemPart *parts = entity->ext.bigGolem.parts;
    s32 i;

    for (i = BIG_GOLEM_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = parts->savedOffsetX;
        parts->offsetY = parts->savedOffsetY;
    }
}

/* Big Golem damage and contact callbacks. */
/* Route damage through the parent at this part's position, restoring its
 * coordinates afterwards. Parts 12/13 play a guarded-hit sound. */
void BigGolemPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *parent = entity->ext.jointPart.owner;
    s32 x, y, damage;
    if ((u8)(entity->ext.jointPart.index - 12) <= 1)
    {
        x = parent->posX.whole;
        y = parent->posY.whole;
        parent->posX.whole = entity->posX.whole;
        parent->posY.whole = entity->posY.whole;
        Player_CalculateAttackPower(hit[2]);
        Hit_ResolveDamage(parent, attacker, hit, 0);
        parent->posX.whole = x;
        parent->posY.whole = y;
        PlaySong(0x69);
    }
    else
    {
        x = parent->posX.whole;
        y = parent->posY.whole;
        parent->posX.whole = entity->posX.whole;
        parent->posY.whole = entity->posY.whole;
        damage = Player_CalculateAttackPower(hit[2]);
        damage = Hit_ResolveDamage(parent, attacker, hit, (s16)damage);
        parent->posX.whole = x;
        parent->posY.whole = y;
        damage = (s16)damage;
        Entity_ShowDamage(entity, damage, 0);
        BigGolem_OnDamaged(entity->ext.jointPart.owner, attacker, hit, damage);
    }
}

/* BigGolemRock_Update. */
extern void Hit_CheckWithPlayerSharedHistory(void *res, struct EwramData_EntityData *entity, struct EwramData_EntityData *owner);

/**
 * @brief AD0B8 | State 0: fall (max 0x40000) with a hitbox owned by
 * unk_4F8_entity; delete when off screen.
 */
void BigGolemRock_Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];
    struct EwramData_EntityData *golem = ENTITY_OWNER(entity);

    if (entity->state == 0)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        if (EntityIsOutsideScreen(entity, 1))
            EntityDelete(entity);
        else
            Hit_CheckWithPlayerSharedHistory(collision, entity, golem);
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BigGolemRock_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

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
 * @brief AD0F8 | Same as Enemy_DamagePlayerOnContact.
 */
void BigGolemRock_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BigGolemRock_OnKilled = GolemDebris_OnKilled */

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern void Effect_SpawnFallingPiece(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 piece);

/**
 * @brief Same as GolemDebris_OnKilled: hit effect, two falling pieces, delete.
 */
void BigGolemRock_OnKilled(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 jitterX;
    s32 jitterY;

    Player_CalculateAttackPower(hit[2]);
    HitEffect_Spawn((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0);
    jitterX = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    jitterY = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + jitterX,
                 entity->velY.whole + jitterY, entity->spriteLayer, 0);
    jitterX = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    jitterY = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
    Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, entity->velX.whole + jitterX,
                 entity->velY.whole + jitterY, entity->spriteLayer, 0);
    EntityDelete(entity);
}

extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern u8 sBigGolemGfx[], sBigGolemPalette[], sBigGolemSpriteSheet[];
/* Big Golem graphics: allocate tiles and two palette variants. */
u8 BigGolem_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 tiles = ObjGfx_LoadResource(sBigGolemGfx, 0);
    if (tiles < 0)
        return 0;
    entity->ext.bigGolem.palette0 = ObjPalette_FindOrLoadVariant(sBigGolemPalette, 0);
    entity->ext.bigGolem.palette1 = ObjPalette_FindOrLoadVariant(sBigGolemPalette, 1);
    entity->basePalette = entity->ext.bigGolem.palette0;
    Sprite_Attach(entity, sBigGolemSpriteSheet, tiles, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BigGolem_DamagePlayerOnContact = Enemy_DamagePlayerOnContact
 *   BigGolem_OnHit = Manticore_OnHit */


/**
 * @brief AD234 | Same as Enemy_DamagePlayerOnContact.
 */
void BigGolem_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief AD260 | Same as Manticore_OnHit.
 */
void BigGolem_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    BigGolem_OnDamaged(entity, attacker, hit, damage);
}
