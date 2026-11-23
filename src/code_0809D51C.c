/* Legion_UpdateDeath: matched (shared dx/dy/dvx for the four DEBRIS calls so the
   masks are not local-allocated; byte mask 0xFB, s16 coordinate arguments; one FAKEMATCH pin). */
/* Legion core (state == 2): stops, falls, breaks apart and dies. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct LegionShell {
    s32 state;
    s32 actionState;
    s32 radiusSet;
    s32 unk_C;
    s32 angleOffset;
    s32 timer;
};

struct LegionState {
    u32 unk_0;
    s32 rotationAngle;
    s32 angularVelocity;
    s32 roomX;
    s32 roomY;
    s32 bobPhase;
    s32 shellActionTimer;
    s32 unk_1C[3];
    struct LegionShell shells[3];
};

extern s32 Entity_MoveWithFilteredBoxCollision(struct EwramData_EntityData *, s32, s32, s32, s32, s32, s32);
extern void Legion_SpawnDebris(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 frame);
extern void EntityMove(struct EwramData_EntityData *);
extern void SoulPickup_SpawnFromEnemyDefinition(s32, s32, s32, s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern u8 sLegionAnimScript0[];
#define ANIM(P) Anim_Start(entity, (struct AnimScript *)(sLegionAnimScript0 + (P)), 2, 0)
#define DEBRIS(F) \
    { dx = entity->posX.whole + (RandomNumberGenerator() & 0x1FFFFF) - 0x100000; \
      dy = entity->posY.whole + (RandomNumberGenerator() & 0x1FFFFF) - 0x100000; \
      dvx = (RandomNumberGenerator() & 0xFFFF) - 0x8000; \
      Legion_SpawnDebris(entity, dx, dy, dvx, -0x10000 - (RandomNumberGenerator() & 0xFFFF), F); }
#define SPARK() \
    { \
        s32 x = entity->posX.whole + (RandomSigned(0x10) << 16); \
        s32 y = entity->posY.whole + (RandomSigned(0x10) << 16); \
        struct EwramData_EntityData *spark = Effect_CreateFromTable(x, y, RandomSigned(0x1000), 0, 4, 7); \
        if (spark != NULL) \
        { \
            spark->velY.whole = RandomSigned(0x4000) - 0x10000; \
            spark->accelY.whole = -spark->velY.whole / 32; \
            spark->spriteLayer = 10; \
        } \
    }

/**
 * @brief Death state (state 2): debris (Legion_SpawnDebris), sounds 0x6E / 0x6F,
 * then request deletion (the update frees the state block).
 */
void Legion_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct LegionState *data = (struct LegionState *)entity->ext.legion.data.value;
    s32 r;
    s32 dx, dy, dvx;

    switch (entity->subState)
    {
    case 0:
        if (entity->stage == 0 && entity->animStep == 0)
        {
            entity->stage = 1;
            ANIM(0x78);
        }
        if (data->angularVelocity > 0x7F)
            data->angularVelocity -= 3;
        if (({ register s32 v_ asm("r1") = data->rotationAngle; /* FAKEMATCH: value in r1, reload of data in r0 */ v_ &= 0x7FFF; v_; }) > 0x7FF)
            break;
        data->angularVelocity = 0;
        entity->subState++;
        break;
    case 1:
        if (entity->stage == 0 && entity->animStep == 0)
        {
            entity->stage = 1;
            ANIM(0x78);
        }
        entity->stage = 1;
        if (!entity->animDone)
            break;
        entity->subState++;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1800;
        break;
    case 2:
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
        r = Entity_MoveWithFilteredBoxCollision(entity, 0, -16, -16, 32, 32, 0x40000);
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        if (!(r & 0x304))
            break;
        if ((entity->ext.legion.brokenCores & 0xF) == 0xF)
            entity->subState++;
        else
        {
            entity->subState = 10;
            entity->timer = 60;
            PlaySong(0x6E);
        }
        ANIM(0xAC);
        *(u8 *)&entity->ext.legion.flags &= 0xFB;
        ObjAffine_Free(entity->affineIndex);
        entity->affine = 0;
        DEBRIS(0x33)
        DEBRIS(0x33)
        DEBRIS(0x32)
        DEBRIS(0x31)
        PlaySong(0x6F);
        break;
    case 3:
        if (!entity->animDone)
            break;
        entity->subState++;
        entity->timer = 0x40;
        entity->velX.whole = 0;
        entity->velY.whole = -0x8000;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0;
        break;
    case 4:
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
        EntityMove(entity);
        data->roomX = entity->posX.whole + bg->xPos.whole;
        data->roomY = entity->posY.whole + bg->yPos.whole;
        if (!(entity->timer & 3))
            SPARK()
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0;
        ANIM(0xBC);
        break;
    case 5:
        if (!(entity->timer++ & 3))
            SPARK()
        if (!entity->animDone)
            break;
        entity->subState++;
        entity->timer = 0x40;
        SoulPickup_SpawnFromEnemyDefinition((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0x6C, 5);
        BossRecoveryPickup_Spawn(0x100 - (s16)bg->xPos.part16.integer, 0x170 - (s16)bg->yPos.part16.integer, 5, 0x20, 1);
        break;
    case 10:
        if (!(entity->timer & 3))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x60000, 0x100000, 0xC0000, entity->spriteLayer - 1);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        BossRecoveryPickup_Spawn(0x100 - (s16)bg->xPos.part16.integer, 0x170 - (s16)bg->yPos.part16.integer, 5, 0x20, 0);
        break;
    case 6:
    case 11:
        Enemy_DropRewards(entity);
        Player_SetSelectedTarget(NULL);
        entity->deleteRequested = 1;
        entity->subState++;
        break;
    case 12:
        break;
    }
    data->shells[0].angleOffset += 0x87;
    data->shells[1].angleOffset -= 0x76;
    data->shells[2].angleOffset += 0xC3;
    if (entity->ext.legion.flags & 4)
    {
        data->rotationAngle += data->angularVelocity;
        ObjAffine_Set(data->rotationAngle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
    if (entity->ext.legion.flags & 2)
    {
        data->bobPhase += 0x180;
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole + Sin(data->bobPhase) * 3;
    }
    else
    {
        entity->posX.whole = data->roomX - bg->xPos.whole;
        entity->posY.whole = data->roomY - bg->yPos.whole;
    }
}
