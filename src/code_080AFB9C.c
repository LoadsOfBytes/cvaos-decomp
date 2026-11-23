/* Enemy helpers decompiled from assembly. */
#include "structs/ewram.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *, s32, u16 *);
extern void EntityMoveX(struct EwramData_EntityData *);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Entity_SetVelocityXFromFacing(struct EwramData_EntityData *, s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, u16, u16);
extern s32 sUnk_08528614[];
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void SkullArcherArrow_OnHit(struct EwramData_EntityData *);
extern u8 sSkullArcherGfx[], sSkullArcherSpriteSheet[];
extern u8 sSkullArcherWalkProfile[], sSkullArcherPalette[];
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *);
extern void Effect_UpdateSpriteDebris(struct EwramData_EntityData *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void SkullArcher_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);

/* Seven animated fragments when the archer dies. */
void SkullArcher_SpawnDeathPieces(struct EwramData_EntityData *entity)
{
    s32 i;
    PlaySong(0x6B);
    for (i = 5; i <= 11; i++)
    {
        struct EwramData_EntityData *fragment = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateSpriteDebris);
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
            fragment->drawFunc = (u32 *)Sprite_DrawEntity;
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
        entity->state = 4;
        return;
    }
    Enemy_ResetFromTemplate(entity);
    Entity_EnableBoxes(entity, Enemy_OnContact, (s32)SkullArcher_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    if (entity->spawnParam0 == 0)
        entity->state = 0;
    else
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 1, 0, 1);
    }
}
