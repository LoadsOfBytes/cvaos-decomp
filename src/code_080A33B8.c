/* Chaos seed shots (33B8/3504) and sparks (35A0). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "syscalls.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
};

extern u8 sChaosSpriteSheet[];
extern u16 Player_CalculateAttackPower(u16);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern s32 Entity_GetVisibilityPattern(s32);
void ChaosShellPiece_Update(struct EwramData_EntityData *entity);
void ChaosSeedShot_Update(struct EwramData_EntityData *entity);
void ChaosSeedShot_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
extern void ChaosSpark_Update(struct EwramData_EntityData *);
extern void ChaosSpark_Draw(struct EwramData_EntityData *);
extern void ChaosSeedShot_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void ChaosSpark_OnContact(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *, s32, s32, s32);

#define OFFSET_X(e) (e->ext.chaosShot.offsetX)
#define OFFSET_Y(e) (e->ext.chaosShot.offsetY)
#define REFLECTED(e) (e->ext.chaosShot.reflected)
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)

/* Seed shot: follow the orb, appear after the delay, then fly at the
 * target (enemyTarget) when its animation ends; it dies with the orb. */
void ChaosSeedShot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;
    struct EwramData *ewram = gEwramData;
    struct BgLayerState *camera = &ewram->bgInfo[1];

    if (entity->state == 0)
    {
        if (orb == NULL || orb->updateFunc == NULL
            || orb->enemyId != entity->enemyId)
            goto check;
        if (entity->stage == 0)
        {
            if ((s8)--entity->timer <= 0)
            {
                entity->hidden = 0;
                entity->stage++;
                Anim_StartIndexed(entity, 4, 2, 0);
                PlaySong(0x144);
            }
        }
        else if (entity->stage == 1 && entity->animDone)
        {
            struct EwramData_EntityData *target;
            entity->state++;
            target = ewram->playerState.enemyTarget;
            Entity_AimVelocityAtPoint(entity, target->posX.whole + camera->xPos.whole,
                         target->posY.whole + camera->yPos.whole - 0x100000, 0x20000);
            Anim_StartIndexed(entity, 5, 2, 1);
            REFLECTED(entity) = 0;
            PlaySong(0x143);
        }
        entity->posX.whole = orb->posX.whole + OFFSET_X(entity);
        entity->posY.whole = orb->posY.whole + OFFSET_Y(entity);
        Anim_Update(entity);
    }
    else if (entity->state == 1)
    {
        entity->scrollWithBg = 1;
        EntityAccelerateAndMove(entity, 0x40000);
        Anim_Update(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            EntityDelete(entity);
            return;
        }
        if (REFLECTED(entity) == 0)
            Hit_CheckWithPlayer(collision, entity);
        Enemy_UpdateHitFlash(entity);
    }
check:
    if (orb->state == 10)
        EntityDelete(entity);
}

/* Seed shot hit by the player: bounce away from the attacker. */
void ChaosSeedShot_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 dx, dy;
    u16 angle;

    Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    entity->hitFlashTimer = 8;
    REFLECTED(entity) = 1;
    dx = entity->posX.whole - attacker->posX.whole;
    dy = entity->posY.whole - attacker->posY.whole;
    angle = ArcTan2(TO_PIXELS(dx), TO_PIXELS(dy));
    dx = Sin(angle + 0x4000);
    dy = Sin(angle);
    entity->velX.whole = FixedMultiply(dx, 0x30000);
    entity->velY.whole = FixedMultiply(dy, 0x30000);
}

/* Create a spark at the orb + (x, y) (box 0x0404FEFE = x -2, y -2, w 4,
 * h 4, harmless to the player, drawn by ChaosSpark_Draw). */
void ChaosOrb_SpawnSpark(struct EwramData_EntityData *orb, s32 x, s32 y)
{
    u32 box;
    struct EwramData_EntityData *owner = orb->ext.chaosChild.link;
    struct EwramData_EntityData *spark = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosSpark_Update);
    struct ChaosData *data;

    box = 0x0404FEFE;
    data = (struct ChaosData *)owner->ext.chaos.data.value;
    if (spark != NULL)
    {
        s32 graphics;

        spark->posX.part16.integer = 0xFF80;
        spark->posY.part16.integer = 0xFF80;
        OFFSET_X(spark) = x;
        OFFSET_Y(spark) = y;
        spark->spriteLayer = orb->spriteLayer - 1;
        graphics = data->graphics;
        spark->basePalette = data->palette2;
        Sprite_Attach(spark, sChaosSpriteSheet, graphics, spark->basePalette);
        Anim_StartIndexed(spark, 6, 2, 0);
        Anim_Update(spark);
        Hit_Enable(spark, 8, ChaosSpark_OnContact, 0);
        HitBox_SetAttack(spark, &box);
        spark->hit.flags |= 4;
        spark->hit.flags &= ~2;
        spark->enemyId = orb->enemyId;
        spark->ext.chaosChild.link = orb;
        spark->scrollWithBg = 0;
        spark->ext.chaosSoulShot.spread = 0x1000000;
        spark->drawFunc = (u32 *)ChaosSpark_Draw;
    }
}
