/* Spawn a fragment at (x, y) with velocity (vx, vy), gravity 0x1000, a
 * random frame 0x2C-0x30 and random flips, in the owner palette (graphics
 * slot passed in, sprites 082258FC); lifetime 60, updated by LegionFragment_Update. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void LegionFragment_Update(struct EwramData_EntityData *);
extern u8 sLegionSpriteSheet[];
extern void LegionDestructibleFragment_Update(struct EwramData_EntityData *);
extern void LegionDestructibleFragment_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void LegionDestructibleFragment_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern struct AnimScript sLegionAnimScript3[];

void Legion_SpawnFragment(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 graphics)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)LegionFragment_Update);
    if (entity == NULL)
        return;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    entity->accelX.whole = 0;
    entity->accelY.whole = 0x1000;
    entity->spriteLayer = owner->spriteLayer + 1;
    entity->unk_510.part8.basePalette = owner->unk_510.part8.basePalette;
    Sprite_Attach(entity, sLegionSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    entity->animFrame = RandomNumberGenerator() % 5 + 0x2C;
    entity->animMode = 2;
    entity->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
    entity->unk_4F4.unk_4F4_entity = owner;
    entity->scrollWithBg = 1;
    entity->flipX = RandomNumberGenerator() % 2;
    entity->flipY = RandomNumberGenerator() % 2;
    entity->timer = 0x3C;
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
}

/* Spawn a destructible fragment (HP 200, hit callbacks LegionDestructibleFragment_DamagePlayerOnContact /
 * LegionDestructibleFragment_OnHit) at (x, y) with velocity (vx, vy), animation 08528328,
 * palette from the owner unk_504, facing the direction of vx. */
void Legion_SpawnDestructibleFragment(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, s32 graphics)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)LegionDestructibleFragment_Update);
    if (entity == NULL)
        return;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    entity->accelX.whole = 0;
    entity->accelY.whole = 0x1000;
    entity->spriteLayer = owner->spriteLayer - 3;
    entity->unk_510.part8.basePalette = owner->unk_504.part8.unk_504;
    Sprite_Attach(entity, sLegionSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    Anim_Start(entity, sLegionAnimScript3, 2, 1);
    Anim_Update(entity);
    entity->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
    entity->unk_4F4.unk_4F4_entity = owner;
    entity->scrollWithBg = 1;
    if (vx < 0)
        entity->flipX = 1;
    else
        entity->flipX = 0;
    entity->timer = 0x3C;
    entity->attack = owner->attack;
    entity->unk_518.part16.hp = 200;
    entity->enemyFlags |= 1;
    Hit_Enable(entity, 8, LegionDestructibleFragment_DamagePlayerOnContact, (s32)LegionDestructibleFragment_OnHit);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
}
