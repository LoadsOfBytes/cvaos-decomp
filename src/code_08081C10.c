/* Headhunter (entity type 0x6A): update and create. */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"

struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sHeadhunterFrameAnim;
extern u8 sHeadhunterSpriteSheet[];
extern struct AnimScript sHeadhunterAnimScript0;
extern struct AnimScript sHeadhunterAnimScript1;
extern struct AnimScript sHeadhunterAnimScript2;
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void Headhunter_UpdateMovement(struct EwramData_EntityData *);
extern void Headhunter_UpdateAttack(struct EwramData_EntityData *);
extern void Headhunter_UpdateDeath(struct EwramData_EntityData *);
extern u8 Headhunter_LoadGraphics(struct EwramData_EntityData *);
extern void HeadhunterTransformationPart_Update(struct EwramData_EntityData *);
extern void Headhunter_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Headhunter_OnHit(struct EwramData_EntityData *);

#define KIND(e) (*(s8 *)&e->ext.headhunterChild.kind)

/* Create the Headhunter (unless already defeated) with its three
 * transformation parts (HeadhunterTransformationPart_Update, kinds 2..0) at fixed room positions;
 * after the fight (bestiaryPreview) only the final defeated form is shown. */
/* The final global flag update keeps the address in r2 and loaded word
 * in r0 to reproduce agbcc allocation. A byte match does not establish
 * original source spelling or organization. */
void EnemyHeadhunterCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *part;
    s32 kind;
    if (entity->spawnParam0 != 0 && gEwramData->gameState.bossRushMode == 0
        && (gEwramData->gameState.bossEventFlags & 0x10))
        goto delete;
    if (!Headhunter_LoadGraphics(entity)) goto delete;
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bossRushMode) entity->attack += 20;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        Anim_StartIndexed(entity, 9, 2, 1);
        Anim_Update(entity);
        entity->basePalette = entity->ext.headhunter.palette2;
        entity->palette = entity->basePalette;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Headhunter_DamagePlayerOnContact, Headhunter_OnHit);
    else
        Hit_Enable(entity, 8, Headhunter_DamagePlayerOnContact, (s32)Headhunter_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    for (kind = 2; kind >= 0; kind--)
    {
        s32 graphics;
        part = Entity_CreateObject((u32 *)HeadhunterTransformationPart_Update);
        if (part == NULL) goto delete;
        part->spriteLayer = entity->spriteLayer + 1;
        graphics = entity->ext.headhunter.graphics;
        switch (kind)
        {
        case 0:
            Sprite_Attach(part, sHeadhunterSpriteSheet, graphics, *(s8 *)&entity->ext.headhunter.palette0);
            part->basePalette = entity->ext.headhunter.palette0;
            Anim_Start(part, &sHeadhunterAnimScript0, 2, 1);
            part->posX.part16.integer = 0xF0 - camera->xPos.part16.integer;
            part->posY.part16.integer = 0x7C - camera->yPos.part16.integer;
            break;
        case 1:
            Sprite_Attach(part, sHeadhunterSpriteSheet, graphics, *(s8 *)&entity->ext.headhunter.palette1);
            part->basePalette = entity->ext.headhunter.palette1;
            Anim_Start(part, &sHeadhunterAnimScript1, 2, 1);
            part->posX.part16.integer = 0x110 - camera->xPos.part16.integer;
            part->posY.part16.integer = 0x80 - camera->yPos.part16.integer;
            break;
        case 2:
            Sprite_Attach(part, sHeadhunterSpriteSheet, graphics, *(s8 *)&entity->ext.headhunter.palette2);
            part->basePalette = entity->ext.headhunter.palette2;
            Anim_Start(part, &sHeadhunterAnimScript2, 2, 1);
            part->posX.part16.integer = 0x100 - camera->xPos.part16.integer;
            part->posY.part16.integer = 0x60 - camera->yPos.part16.integer;
            break;
        }
        part->enemyId = entity->enemyId;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        part->flipX = entity->flipX;
        ENTITY_OWNER(part) = entity;
        KIND(part) = kind;
        Anim_Update(part);
    }
    entity->state = 0;
    Anim_StartIndexed(entity, 0x17, 2, 1);
    Anim_Update(entity);
    Player_SetSelectedTarget(entity);
    { register u32 *flags asm("r2") = &gEwramData->gameState.stateFlags; /* FAKEMATCH: pinned register */
      u32 value = *flags;
      value |= 2;
      *flags = value; }
    Music_SetCurrent(0x10);
    return;
delete:
    EntityDelete(entity);
}
