/* Objects 0x32..0x34 0x08059494..0x0805A18B. */
#include "code_08009A0.h"
#include "layer_object.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/*
 * Object 0x34 (swinging platform): swing angle, angular speed (+-0x100) and
 * the acceleration applied at the turning points (spawnParam1). Its parts
 * (0, 1 chain links, 2 the platform) keep the pivot entity and their index.
 */
#define SWING_ANGLE(e) e->unk_4FC.part16.unk_4FC
#define SWING_SPEED(e) e->unk_4FC.part16.unk_4FE[0]
#define SWING_ACCEL(e) e->unk_500.part16.unk_500
#define SWING_PART_PIVOT(e) e->unk_504.unk_504_entity
#define SWING_PART_INDEX(e) e->unk_500.part8.unk_502

/* Two hit boxes of object 0x32 (by unk_516). */
struct Object32Boxes {
    u32 box[2];
};
extern const struct Object32Boxes sObject32Boxes;

u8 Object32_LoadGraphics(struct EwramData_EntityData *entity);
void Object32_UpdateState(struct EwramData_EntityData *entity);
void Object32_SpawnDebris(struct EwramData_EntityData *entity, s32 frame, s32 index);
void Object32Debris_Update(struct EwramData_EntityData *entity);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern s32 Entity_GetVisibilityPattern(s32 step);
u8 Object33_LoadGraphics(struct EwramData_EntityData *entity);
void Object33_SpawnFlyingSpark(struct EwramData_EntityData *entity);
void Object33_SpawnFallingSpark(struct EwramData_EntityData *entity);
void Object33FallingSpark_Update(struct EwramData_EntityData *entity);
void Object33_UpdateState(struct EwramData_EntityData *entity);
void Object33FlyingSpark_Update(struct EwramData_EntityData *entity);
u8 Object34_LoadGraphics(struct EwramData_EntityData *entity);
void Object34_SpawnPart(struct EwramData_EntityData *entity, s32 part);
void Object34Part_Update(struct EwramData_EntityData *entity);
void Object34Part_Draw(struct EwramData_EntityData *entity);
extern u8 sObject34Gfx[];
extern u8 sObject34SpriteSheet[];

/* Graphics of objects 0x32 / 0x33 and their pieces. */
extern u8 sObject33Gfx[];
extern u8 sObject33Palette[];
extern u8 sObject33SpriteSheet[];
extern u8 sObject32Gfx[];
extern u8 sObject32Object34Palette[];
extern u8 sObject32SpriteSheet[];

/**
 * @brief 59494 | Object 0x32 create: load the graphics (delete the object
 * when that fails), hit box by unk_516, remember the world position
 * (BG 1 relative) in BG1_OBJECT_X/Y and the shuttle fields (unused by 0x32).
 */
void Object32Create(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct Object32Boxes boxes = sObject32Boxes;
    s32 roomY;
    s32 roomX;

    if (!Object32_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, NULL, 0);
    HitBox_SetBody(entity, &boxes.box[entity->unk_514.part16.spawnParam1]);
    HitBox_SetAttack(entity, &boxes.box[entity->unk_514.part16.spawnParam1]);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    if (entity->unk_514.part16.spawnParam1 == 0)
        entity->animFrame = 0;
    else
        entity->animFrame = 7;
    entity->spriteLayer = 0x12;
    entity->state = 0;
    roomY = entity->posY.whole;
    SHUTTLE_START(entity) = roomY + bg->yPos.whole;
    SHUTTLE_END(entity) = entity->unk_514.part16.spawnParam0 << 16;
    roomX = entity->posX.whole + bg->xPos.whole;
    BG1_OBJECT_X(entity) = roomX;
    roomY += bg->yPos.whole;
    BG1_OBJECT_Y(entity) = roomY;
    BG1_OBJECT_DRAW_X(entity) = roomX - bg->xPos.whole;
    BG1_OBJECT_DRAW_Y(entity) = roomY - bg->yPos.whole;
    entity->scrollWithBg = 0;
    if ((s32)SHUTTLE_START(entity) > SHUTTLE_END(entity))
        SHUTTLE_DIR(entity) = 1;
    else
        SHUTTLE_DIR(entity) = 0;
}

/**
 * @brief 59580 | Object 0x32, state 0: keep the world position (BG1_OBJECT_X /
 * BG1_OBJECT_Y) on screen; when it becomes standingPlatform crumble in four timed steps
 * (frames animFrame + 1, sounds 0x152, 0x151, 0x150) and break into debris.
 */
void Object32_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    struct BgLayerState *bg = &(*ewramRef)->bgInfo[1];

    BG1_OBJECT_DRAW_X(entity) = BG1_OBJECT_X(entity) - bg->xPos.whole;
    BG1_OBJECT_DRAW_Y(entity) = BG1_OBJECT_Y(entity) - bg->yPos.whole;
    switch (entity->subState)
    {
        case 0:
            if ((*ewramRef)->playerState.standingPlatform != entity)
                break;
            entity->subState++;
            entity->timer = 0;
            PlaySong(0x152);
            break;
        case 1:
            if (entity->timer++ < 10)
                break;
            entity->subState++;
            entity->timer = 0;
            entity->animFrame++;
            break;
        case 2:
            if (entity->timer++ < 0x14)
                break;
            entity->subState++;
            entity->timer = 0;
            entity->animFrame++;
            PlaySong(0x151);
            break;
        case 3:
            if (entity->timer++ < 10)
                break;
            entity->subState++;
            entity->timer = 0;
            entity->animFrame++;
            PlaySong(0x150);
            break;
        case 4:
            if (entity->timer++ < 6)
                break;
            entity->deleteRequested = 1;
            Object32_SpawnDebris(entity, 5, 1);
            Object32_SpawnDebris(entity, 6, 2);
            Object32_SpawnDebris(entity, 4, 3);
            if (entity->unk_514.part16.spawnParam1 == 0)
            {
                Object32_SpawnDebris(entity, 4, 0);
                Object32_SpawnDebris(entity, 5, 4);
            }
            break;
    }
    entity->posX.whole = BG1_OBJECT_X(entity) - bg->xPos.whole;
    entity->posY.whole = BG1_OBJECT_Y(entity) - bg->yPos.whole;
}

/**
 * @brief 596E0 | Spawn debris piece index (frame frame) of object 0x32:
 * x offset (index * 6 - 12) px, flying sideways by its index with random
 * upward speed and flips, gravity 0x1800.
 */
void Object32_SpawnDebris(struct EwramData_EntityData *entity, s32 frame, s32 index)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object32Debris_Update);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole + (s16)(index * 6 - 12) * 0x10000;
    child->posY.whole = entity->posY.whole + 0xC0000;
    child->spriteLayer = entity->spriteLayer + 1;
    gfx = ObjGfx_Find(sObject32Gfx);
    child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
    Sprite_Attach(child, sObject32SpriteSheet, gfx, child->unk_510.part8.basePalette);
    child->animFrame = frame;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    child->flipY = RandomNumberGenerator() % 2;
    child->velX.whole = (index << 13) - 0x4000;
    child->velY.whole = (RandomNumberGenerator() & 0x3FFF) + 0x4000;
    child->accelY.whole = 0x1800;
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief 597DC | Object 0x32 update: Player_QueueEntity(1) while the player is
 * within 0x20 px.
 */
void Object32Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    s32 dx;
    u32 hitInfo[4];

    if (entity->state == 0)
        Object32_UpdateState(entity);
    player = gEwramData->playerState.player;
    dx = player->posX.whole - entity->posX.whole;
    if (dx < 0)
        dx = -dx;
    if (dx <= 0x1FFFFF)
    {
        s32 dy = player->posY.whole - entity->posY.whole;
        if (dy < 0)
            dy = -dy;
        if (dy <= 0x1FFFFF)
            Player_QueueEntity(entity, 1);
    }
    Hit_CheckWithPlayer(hitInfo, entity);
}

/**
 * @brief 59840 | Debris piece: fall with gravity 0x40000 max, deleted when
 * outside the screen.
 */
void Object32Debris_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x40000);
    if (EntityIsOutsideScreen(entity, 3))
        EntityDelete(entity);
}

/**
 * @brief 59864 | Load the graphics of object 0x32; 0 when no slot is free.
 */
u8 Object32_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject32Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sObject32Object34Palette, 1);
    Sprite_Attach(entity, sObject32SpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 598B0 | Object 0x33 create (spark source): load the graphics
 * (delete the object when that fails), animation 1 for variant 1, else 0;
 * variant 0 is hidden.
 */
void Object33Create(struct EwramData_EntityData *entity)
{
    u16 script;

    if (!Object33_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    if (entity->unk_514.part16.spawnParam0 != 1)
        script = 0;
    else
        script = 1;
    Anim_StartIndexed(entity, script, 0, 1);
    Anim_Update(entity);
    entity->objMode = 1;
    entity->spriteLayer = 9;
    entity->flipX = 0;
    entity->flipY = 0;
    entity->state = 0;
    if (entity->unk_514.part16.spawnParam0 == 0)
        entity->hidden = 1;
}

/**
 * @brief 59928 | Spawn a spark (Object33FlyingSpark_Update) flying up-left (variant 2:
 * random left/right) with random frame and flips, gravity 0x2800, 0x32
 * frames.
 */
void Object33_SpawnFlyingSpark(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object33FlyingSpark_Update);
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole;
    child->posY.whole = entity->posY.whole;
    child->spriteLayer = entity->spriteLayer + 1;
    gfx = ObjGfx_Find(sObject33Gfx);
    child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
    Sprite_Attach(child, sObject33SpriteSheet, gfx, child->unk_510.part8.basePalette);
    child->animFrame = RandomNumberGenerator() % 4 + 0xB;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    child->flipY = RandomNumberGenerator() % 2;
    child->objMode = 1;
    if (entity->unk_514.part16.spawnParam0 == 2)
        child->velX.whole = RandomNumberGenerator() % 0x40000 - 0x20000;
    else
        child->velX.whole = -0x10000 - RandomNumberGenerator() % 0x10000;
    if (entity->unk_514.part16.spawnParam0 == 0)
        child->velY.whole = -0x10000 - RandomNumberGenerator() % 0x10000;
    else
        child->velY.whole = -0x20000 - RandomNumberGenerator() % 0x10000;
    child->accelY.whole = 0x2800;
    child->unk_4FC.whole = bg->xPos.whole;
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->timer = 0x32;
}

/**
 * @brief 59A8C | Spawn a spark (Object33FallingSpark_Update) within +-16 px with random
 * frame 6..10, flips and a small upward speed.
 */
void Object33_SpawnFallingSpark(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object33FallingSpark_Update);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole + RandomNumberGenerator() % 0x200000 - 0x100000;
    child->posY.whole = entity->posY.whole;
    child->spriteLayer = entity->spriteLayer + 1;
    gfx = ObjGfx_Find(sObject33Gfx);
    child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
    Sprite_Attach(child, sObject33SpriteSheet, gfx, child->unk_510.part8.basePalette);
    child->animFrame = RandomNumberGenerator() % 5 + 6;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    child->flipY = RandomNumberGenerator() % 2;
    child->objMode = 1;
    child->velX.whole = RandomNumberGenerator() % 0xC000 - 0x6000;
    child->velY.whole = -0x4000 - RandomNumberGenerator() % 0x2000;
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    child->timer = 0x32;
}

/**
 * @brief 59B9C | Object 0x33 update.
 */
void Object33Update(struct EwramData_EntityData *entity)
{
    u32 var_0[4];

    if (entity->state == 0)
        Object33_UpdateState(entity);
    Anim_Update(entity);
}

/**
 * @brief 59BBC | Spark (Object33_SpawnFallingSpark): fall, blink, end after timer.
 */
void Object33FallingSpark_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x60000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
        entity->deleteRequested = 1;
}

/**
 * @brief 59C04 | Load the graphics of object 0x33; 0 when no slot is free.
 */
u8 Object33_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject33Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sObject33Palette, 0);
    Sprite_Attach(entity, sObject33SpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 59C50 | Object 0x33, state 0: every 0x10..0x1F frames spawn a spark
 * (and for variants != 0 a second kind).
 */
void Object33_UpdateState(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0)
        return;
    if (entity->timer-- > 1)
        return;
    entity->timer = RandomNumberGenerator() % 16 + 0x10;
    Object33_SpawnFlyingSpark(entity);
    if (entity->unk_514.part16.spawnParam0 != 0)
        Object33_SpawnFallingSpark(entity);
}

/**
 * @brief 59C90 | Spark (Object33_SpawnFlyingSpark): slow down sideways, fall, blink,
 * end after timer.
 */
void Object33FlyingSpark_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x40);
    EntityAccelerateAndMove(entity, 0x60000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
        entity->deleteRequested = 1;
}

/**
 * @brief 59CE0 | Object 0x34 create (swinging platform): load the graphics
 * (delete the object when that fails), angle unk_4FC = 0x4000, swing
 * acceleration unk_500 = unk_516, start direction by variant, world
 * position in unk_4F4/unk_4F8; spawn its three parts.
 */
void Object34Create(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 roomX;
    s32 roomY;

    if (!Object34_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->animFrame = 3;
    entity->spriteLayer = 0x12;
    entity->state = 0;
    SWING_ANGLE(entity) = 0x4000;
    SWING_ACCEL(entity) = entity->unk_514.part16.spawnParam1;
    if (entity->unk_514.part16.spawnParam0 == 0)
    {
        entity->subState = 0;
        SWING_SPEED(entity) = 0x100;
    }
    else
    {
        entity->subState = 1;
        SWING_SPEED(entity) = -0x100;
    }
    roomX = entity->posX.whole + bg->xPos.whole;
    BG1_OBJECT_X(entity) = roomX;
    roomY = entity->posY.whole + bg->yPos.whole;
    BG1_OBJECT_Y(entity) = roomY;
    BG1_OBJECT_DRAW_X(entity) = roomX - bg->xPos.whole;
    BG1_OBJECT_DRAW_Y(entity) = roomY - bg->yPos.whole;
    entity->scrollWithBg = 0;
    Object34_SpawnPart(entity, 0);
    Object34_SpawnPart(entity, 1);
    Object34_SpawnPart(entity, 2);
}

/**
 * @brief 59D98 | Object 0x34, state 0: swing the angle SWING_ANGLE by the
 * speed SWING_SPEED (+-0x100, accelerated by SWING_ACCEL) between 0x3FFF and
 * 0x4000 turning points; keep the world position on screen.
 */
void Object34_UpdateSwing(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u16 speed;

    BG1_OBJECT_DRAW_X(entity) = BG1_OBJECT_X(entity) - bg->xPos.whole;
    BG1_OBJECT_DRAW_Y(entity) = BG1_OBJECT_Y(entity) - bg->yPos.whole;
    switch (entity->subState)
    {
        case 0:
            {
                u16 speed2 = SWING_SPEED(entity);
                if (SWING_SPEED(entity) > -0x100)
                {
                    SWING_SPEED(entity) = speed2 - SWING_ACCEL(entity);
                    if (SWING_SPEED(entity) < -0x100)
                        SWING_SPEED(entity) = -0x100;
                }
            }
            SWING_ANGLE(entity) += SWING_SPEED(entity);
            if (SWING_ANGLE(entity) > 0x3FFF)
                break;
            entity->subState = 1;
            break;
        case 1:
            speed = SWING_SPEED(entity);
            if (SWING_SPEED(entity) <= 0xFF)
            {
                SWING_SPEED(entity) = speed + SWING_ACCEL(entity);
                if (SWING_SPEED(entity) > 0x100)
                    SWING_SPEED(entity) = 0x100;
            }
            SWING_ANGLE(entity) += SWING_SPEED(entity);
            if (SWING_ANGLE(entity) <= 0x4000)
                break;
            entity->subState = 0;
            break;
    }
    entity->posX.whole = BG1_OBJECT_X(entity) - bg->xPos.whole;
    entity->posY.whole = BG1_OBJECT_Y(entity) - bg->yPos.whole;
}

/**
 * @brief 59E4C | Spawn part part (0, 1: chain links, 2: the platform
 * with a hit box) of object 0x34 at its position; the links get an affine
 * slot rotated by the parent's angle. SWING_PART_PIVOT = parent, SWING_PART_INDEX = index.
 */
void Object34_SpawnPart(struct EwramData_EntityData *entity, s32 part)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object34Part_Update);
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u32 box = 0x1C20F4F0;
    s32 gfx;
    u16 affineIndex;
    s32 roomX;
    s32 roomY;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole;
    child->posY.whole = entity->posY.whole;
    child->spriteLayer = entity->spriteLayer + 1;
    gfx = ObjGfx_Find(sObject34Gfx);
    child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
    Sprite_Attach(child, sObject34SpriteSheet, gfx, child->unk_510.part8.basePalette);
    if (part == 0)
        child->animFrame = 2;
    else if (part == 1)
        child->animFrame = part;
    else
        child->animFrame = 0;
    if (part == 2)
    {
        Hit_Enable(child, 8, NULL, 0);
        HitBox_SetBody(child, &box);
        HitBox_SetAttack(child, &box);
    }
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    if (part != 2)
    {
        affineIndex = ObjAffine_Alloc(1);
        if ((s16)affineIndex < 0)
        {
            EntityDelete(child);
            return;
        }
        child->affineIndex = affineIndex;
        child->affine = 1;
        ObjAffine_SetBiased(SWING_ANGLE(entity), 0x10000, 0x10000, child->flipX, child->flipY, child->affineIndex);
    }
    roomX = child->posX.whole + bg->xPos.whole;
    BG1_OBJECT_X(child) = roomX;
    roomY = child->posY.whole + bg->yPos.whole;
    BG1_OBJECT_Y(child) = roomY;
    BG1_OBJECT_DRAW_X(child) = roomX - bg->xPos.whole;
    BG1_OBJECT_DRAW_Y(child) = roomY - bg->yPos.whole;
    SWING_PART_PIVOT(child) = entity;
    SWING_PART_INDEX(child) = part;
    child->scrollWithBg = 0;
    child->drawFunc = (u32 *)Object34Part_Draw;
}

/**
 * @brief 59FB4 | Object 0x34 part: hang at distance (index * 30 + 16) px
 * from the parent along its swing angle (moving by the difference so that
 * riders follow); links rotate with the parent; the platform (index 2)
 * calls Player_QueueEntity(1) near the player and carries the player
 * (Player_SetXFraction / Player_SetYFraction) while being standingPlatform.
 */
void Object34Part_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    struct BgLayerState *bg = &(*ewramRef)->bgInfo[1];
    struct EwramData_EntityData *pivot = SWING_PART_PIVOT(entity);
    struct EwramData_EntityData *player;
    s32 targetX;
    s32 targetY;
    s32 dist;

    BG1_OBJECT_X(entity) += entity->velX.whole;
    BG1_OBJECT_Y(entity) += entity->velY.whole;
    if (entity->subState == 0)
    {
        targetX = Sin(SWING_ANGLE(pivot) + 0x4000);
        targetX = FixedMultiply(targetX, ((s8)SWING_PART_INDEX(entity) * 15 << 17) + 0x100000);
        targetY = Sin(SWING_ANGLE(pivot));
        targetY = FixedMultiply(targetY, ((s8)SWING_PART_INDEX(entity) * 15 << 17) + 0x100000);
        targetX += BG1_OBJECT_X(pivot);
        targetY += BG1_OBJECT_Y(pivot);
        entity->velX.whole = targetX - BG1_OBJECT_X(entity);
        entity->velY.whole = targetY - BG1_OBJECT_Y(entity);
    }
    if ((s8)SWING_PART_INDEX(entity) != 2)
        ObjAffine_SetBiased(SWING_ANGLE(pivot), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    entity->posX.whole = BG1_OBJECT_X(entity) - bg->xPos.whole;
    entity->posY.whole = BG1_OBJECT_Y(entity) - bg->yPos.whole;
    BG1_OBJECT_DRAW_X(entity) = BG1_OBJECT_X(entity) - bg->xPos.whole;
    BG1_OBJECT_DRAW_Y(entity) = BG1_OBJECT_Y(entity) - bg->yPos.whole;
    if ((s8)SWING_PART_INDEX(entity) == 2)
    {
        player = (*ewramRef)->playerState.player;
        dist = player->posX.whole - entity->posX.whole;
        if (dist < 0)
            dist = -dist;
        if (dist <= 0x1FFFFF)
        {
            dist = player->posY.whole - entity->posY.whole;
            if (dist < 0)
                dist = -dist;
            if (dist <= 0x1FFFFF)
                Player_QueueEntity(entity, 1);
        }
    }
    if (gEwramData->playerState.standingPlatform == entity)
    {
        // Low halves of posX / posY, as in the original.
        Player_SetXFraction((s16)entity->posX.part16.fraction);
        Player_SetYFraction((s16)entity->posY.part16.fraction);
    }
}

/**
 * @brief 5A108 | Object 0x34 update.
 */
void Object34Update(struct EwramData_EntityData *entity)
{
    u32 var_0[4];

    if (entity->state == 0)
        Object34_UpdateSwing(entity);
}

/**
 * @brief 5A120 | Load the graphics of object 0x34; 0 when no slot is free.
 */
u8 Object34_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject34Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sObject32Object34Palette, 2);
    Sprite_Attach(entity, sObject34SpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 5A16C | Object 0x34 part draw: draw (Sprite_Draw) at the
 * position saved in BG1_OBJECT_DRAW_X / _Y, then restore posX / posY.
 */
void Object34Part_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;

    entity->posX.whole = BG1_OBJECT_DRAW_X(entity);
    entity->posY.whole = BG1_OBJECT_DRAW_Y(entity);
    Sprite_Draw(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
