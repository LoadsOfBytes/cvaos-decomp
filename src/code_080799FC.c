#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Arachne_LoadGraphics(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(void *update);
extern void ArachneLegs_Update(struct EwramData_EntityData *entity);
extern void Arachne_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Arachne_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *info);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern u8 sArachneGfx[], sArachneSpriteSheet[];

/**
 * @brief 799FC | Arachne create: six legs Entity_CreateObject(ArachneLegs_Update) with
 * graphics sArachneSpriteSheet, odd ones in front (palette +1, animFrame 0x19) and
 * even ones behind (0x15), leg index in unk_504 and velX (i / 2 * 7) << 16
 * as the spread; frozen time gives state 4, otherwise box slot 8
 * (Hit_EnableContact), Entity_UpdateBoxes / Entity_SnapToGround and state 0. A failed
 * allocation deletes it.
 */
void EnemyArachneCreate(struct EwramData_EntityData *entity)
{
    s32 i;

    if ((u8)Arachne_LoadGraphics(entity) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    for (i = 5; i >= 0; i--)
    {
        struct EwramData_EntityData *leg = Entity_CreateObject(ArachneLegs_Update);
        s32 tiles;
        if (leg != NULL)
        {
        if (i & 1)
            leg->spriteLayer = entity->spriteLayer + 1;
        else
            leg->spriteLayer = entity->spriteLayer - 1;
        tiles = ObjGfx_Find(sArachneGfx);
        Sprite_Attach(leg, sArachneSpriteSheet, tiles, entity->unk_510.part8.basePalette);
        leg->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
        leg->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
        leg->scrollWithBg = 0;
        leg->drawFunc = (u32 *)Sprite_DrawEntity;
        ENTITY_OWNER(leg) = entity;
        leg->ext.arachne.legIndex = i;
        leg->posX.whole = entity->posX.whole;
        leg->posY.whole = entity->posY.whole;
        leg->velX.whole = (i / 2 * 7) << 16;
        if (i & 1)
            leg->animFrame = 0x19;
        else
            leg->animFrame = 0x15;
        continue;
        }
        EntityDelete(entity);
        return;
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        entity->ext.arachne.gait = 0;
        return;
    }
    Hit_EnableContact(entity, 8, Arachne_DamagePlayerOnContact, (s32)Arachne_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}
