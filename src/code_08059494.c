/* Objects 0x32..0x34 0x08059494..0x0805A18B. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/* Two hit boxes of object 0x32 (by spawnParam1). */
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
 * when that fails), hit box by spawnParam1, remember the world position
 * (BG 1 relative) in ext.layerObject.x/Y and the shuttle fields (unused by 0x32).
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
    HitBox_SetBody(entity, &boxes.box[entity->spawnParam1]);
    HitBox_SetAttack(entity, &boxes.box[entity->spawnParam1]);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    if (entity->spawnParam1 == 0)
        entity->animFrame = 0;
    else
        entity->animFrame = 7;
    entity->spriteLayer = 0x12;
    entity->state = 0;
    roomY = entity->posY.whole;
    entity->ext.layerObject.shuttleStart = roomY + bg->yPos.whole;
    entity->ext.layerObject.shuttleEnd = entity->spawnParam0 << 16;
    roomX = entity->posX.whole + bg->xPos.whole;
    entity->ext.layerObject.x = roomX;
    roomY += bg->yPos.whole;
    entity->ext.layerObject.y = roomY;
    entity->ext.layerObject.drawX = roomX - bg->xPos.whole;
    entity->ext.layerObject.drawY = roomY - bg->yPos.whole;
    entity->scrollWithBg = 0;
    if ((s32)entity->ext.layerObject.shuttleStart > entity->ext.layerObject.shuttleEnd)
        entity->ext.layerObject.shuttleDir = 1;
    else
        entity->ext.layerObject.shuttleDir = 0;
}

/**
 * @brief 59580 | Object 0x32, state 0: keep the world position (ext.layerObject.x /
 * ext.layerObject.y) on screen; when it becomes standingPlatform crumble in four timed steps
 * (frames animFrame + 1, sounds 0x152, 0x151, 0x150) and break into debris.
 */
void Object32_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    struct BgLayerState *bg = &(*ewramRef)->bgInfo[1];

    entity->ext.layerObject.drawX = entity->ext.layerObject.x - bg->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - bg->yPos.whole;
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
            if (entity->spawnParam1 == 0)
            {
                Object32_SpawnDebris(entity, 4, 0);
                Object32_SpawnDebris(entity, 5, 4);
            }
            break;
    }
    entity->posX.whole = entity->ext.layerObject.x - bg->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - bg->yPos.whole;
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
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sObject32SpriteSheet, gfx, child->basePalette);
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
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject32Object34Palette, 1);
    Sprite_Attach(entity, sObject32SpriteSheet, gfx, entity->basePalette);
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
    if (entity->spawnParam0 != 1)
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
    if (entity->spawnParam0 == 0)
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
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sObject33SpriteSheet, gfx, child->basePalette);
    child->animFrame = RandomNumberGenerator() % 4 + 0xB;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    child->flipY = RandomNumberGenerator() % 2;
    child->objMode = 1;
    if (entity->spawnParam0 == 2)
        child->velX.whole = RandomNumberGenerator() % 0x40000 - 0x20000;
    else
        child->velX.whole = -0x10000 - RandomNumberGenerator() % 0x10000;
    if (entity->spawnParam0 == 0)
        child->velY.whole = -0x10000 - RandomNumberGenerator() % 0x10000;
    else
        child->velY.whole = -0x20000 - RandomNumberGenerator() % 0x10000;
    child->accelY.whole = 0x2800;
    child->ext.raw.unk_4FC.whole = bg->xPos.whole;
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
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sObject33SpriteSheet, gfx, child->basePalette);
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
    u32 collision[4];

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
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject33Palette, 0);
    Sprite_Attach(entity, sObject33SpriteSheet, gfx, entity->basePalette);
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
    if (entity->spawnParam0 != 0)
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
 * acceleration unk_500 = spawnParam1, start direction by variant, world
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
    entity->ext.object34.angle = 0x4000;
    entity->ext.object34.accel = entity->spawnParam1;
    if (entity->spawnParam0 == 0)
    {
        entity->subState = 0;
        entity->ext.object34.speed = 0x100;
    }
    else
    {
        entity->subState = 1;
        entity->ext.object34.speed = -0x100;
    }
    roomX = entity->posX.whole + bg->xPos.whole;
    entity->ext.layerObject.x = roomX;
    roomY = entity->posY.whole + bg->yPos.whole;
    entity->ext.layerObject.y = roomY;
    entity->ext.layerObject.drawX = roomX - bg->xPos.whole;
    entity->ext.layerObject.drawY = roomY - bg->yPos.whole;
    entity->scrollWithBg = 0;
    Object34_SpawnPart(entity, 0);
    Object34_SpawnPart(entity, 1);
    Object34_SpawnPart(entity, 2);
}

/**
 * @brief 59D98 | Object 0x34, state 0: swing the angle ext.object34.angle by the
 * speed (+-0x100, accelerated by ext.object34.accel)->ext.object34.speed between 0x3FFF and
 * 0x4000 turning points; keep the world position on screen.
 */
void Object34_UpdateSwing(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u16 speed;

    entity->ext.layerObject.drawX = entity->ext.layerObject.x - bg->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - bg->yPos.whole;
    switch (entity->subState)
    {
        case 0:
            {
                u16 speed2 = entity->ext.object34.speed;
                if (entity->ext.object34.speed > -0x100)
                {
                    entity->ext.object34.speed = speed2 - entity->ext.object34.accel;
                    if (entity->ext.object34.speed < -0x100)
                        entity->ext.object34.speed = -0x100;
                }
            }
            entity->ext.object34.angle += entity->ext.object34.speed;
            if (entity->ext.object34.angle > 0x3FFF)
                break;
            entity->subState = 1;
            break;
        case 1:
            speed = entity->ext.object34.speed;
            if (entity->ext.object34.speed <= 0xFF)
            {
                entity->ext.object34.speed = speed + entity->ext.object34.accel;
                if (entity->ext.object34.speed > 0x100)
                    entity->ext.object34.speed = 0x100;
            }
            entity->ext.object34.angle += entity->ext.object34.speed;
            if (entity->ext.object34.angle <= 0x4000)
                break;
            entity->subState = 0;
            break;
    }
    entity->posX.whole = entity->ext.layerObject.x - bg->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - bg->yPos.whole;
}

/**
 * @brief 59E4C | Spawn part part (0, 1: chain links, 2: the platform
 * with a hit box) of object 0x34 at its position; the links get an affine
 * slot rotated by the parent's angle. ext.object34Part.pivot = parent, ext.object34Part.index = index.
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
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sObject34SpriteSheet, gfx, child->basePalette);
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
        ObjAffine_SetBiased(entity->ext.object34.angle, 0x10000, 0x10000, child->flipX, child->flipY, child->affineIndex);
    }
    roomX = child->posX.whole + bg->xPos.whole;
    child->ext.layerObject.x = roomX;
    roomY = child->posY.whole + bg->yPos.whole;
    child->ext.layerObject.y = roomY;
    child->ext.layerObject.drawX = roomX - bg->xPos.whole;
    child->ext.layerObject.drawY = roomY - bg->yPos.whole;
    child->ext.object34Part.pivot = entity;
    child->ext.object34Part.index = part;
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
    struct EwramData_EntityData *pivot = entity->ext.object34Part.pivot;
    struct EwramData_EntityData *player;
    s32 targetX;
    s32 targetY;
    s32 dist;

    entity->ext.layerObject.x += entity->velX.whole;
    entity->ext.layerObject.y += entity->velY.whole;
    if (entity->subState == 0)
    {
        targetX = Sin(pivot->ext.object34.angle + 0x4000);
        targetX = FixedMultiply(targetX, ((s8)entity->ext.object34Part.index * 15 << 17) + 0x100000);
        targetY = Sin(pivot->ext.object34.angle);
        targetY = FixedMultiply(targetY, ((s8)entity->ext.object34Part.index * 15 << 17) + 0x100000);
        targetX += pivot->ext.layerObject.x;
        targetY += pivot->ext.layerObject.y;
        entity->velX.whole = targetX - entity->ext.layerObject.x;
        entity->velY.whole = targetY - entity->ext.layerObject.y;
    }
    if ((s8)entity->ext.object34Part.index != 2)
        ObjAffine_SetBiased(pivot->ext.object34.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    entity->posX.whole = entity->ext.layerObject.x - bg->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - bg->yPos.whole;
    entity->ext.layerObject.drawX = entity->ext.layerObject.x - bg->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - bg->yPos.whole;
    if ((s8)entity->ext.object34Part.index == 2)
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
    u32 collision[4];

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
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject32Object34Palette, 2);
    Sprite_Attach(entity, sObject34SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 5A16C | Object 0x34 part draw: draw (Sprite_Draw) at the
 * position saved in ext.layerObject.drawX / _Y, then restore posX / posY.
 */
void Object34Part_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;

    entity->posX.whole = entity->ext.layerObject.drawX;
    entity->posY.whole = entity->ext.layerObject.drawY;
    Sprite_Draw(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
