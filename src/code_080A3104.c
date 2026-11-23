/* Chaos orb damage (3104) and the shell pieces (3178/3210). */
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
extern void ChaosSeedShot_Update(struct EwramData_EntityData *);
extern void ChaosSeedShot_OnHit(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *);
extern void ChaosSeedShot_DamagePlayerOnContact(struct EwramData_EntityData *);
struct HitBox {
    s8 x;
    s8 y;
    u8 w;
    u8 h;
};
#define OFFSET_X(e) (e->ext.chaosShot.offsetX)
#define OFFSET_Y(e) (e->ext.chaosShot.offsetY)
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

/* Orb hit: lose HP; at 0 switch to the breaking state 10. */
void ChaosOrb_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->unk_510.part8.hitFlashTimer = 8;
    damage <<= 16;
    damage >>= 16;
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)(entity->unk_518.part16.hp -= damage) <= 0)
    {
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 40;
        entity->unk_510.part8.hitFlashTimer = 0;
        entity->state = 10;
        entity->subState = 0;
        entity->stage = 0;
    }
}

/* Create a shell piece (frame 3 + frame) over the orb for `timer` frames. */
void ChaosOrb_CreateShellPiece(struct EwramData_EntityData *orb, s32 frame, s32 timer)
{
    struct EwramData_EntityData *owner = orb->ext.chaosChild.link;
    struct EwramData_EntityData *piece = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosShellPiece_Update);
    struct ChaosData *data = (struct ChaosData *)owner->ext.chaos.data.value;

    if (piece != NULL)
    {
        s32 graphics;

        piece->posX.part16.integer = 0xFF80;
        piece->posY.part16.integer = 0xFF80;
        piece->spriteLayer = orb->spriteLayer;
        graphics = data->graphics;
        piece->unk_510.part8.basePalette = data->palette;
        Sprite_Attach(piece, sChaosSpriteSheet, graphics, piece->unk_510.part8.basePalette);
        piece->animMode = 2;
        piece->animFrame = frame + 3;
        piece->unk_518.part8.enemyId = orb->unk_518.part8.enemyId;
        piece->ext.chaosChild.link = orb;
        piece->scrollWithBg = 0;
        piece->timer = timer;
        piece->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Shell piece: stick to the orb, then fall blinking for 60 frames. */
void ChaosShellPiece_Update(struct EwramData_EntityData *entity)
{
    u8 unused[16];
    struct EwramData_EntityData *orb = entity->ext.chaosChild.link;

    if (entity->state == 0)
    {
        if (orb == NULL || orb->updateFunc == NULL
            || orb->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
            return;
        entity->posX.whole = orb->posX.whole;
        entity->posY.whole = orb->posY.whole;
        if ((s8)--entity->timer <= 0)
        {
            entity->state++;
            entity->accelY.whole = 0x2000;
            entity->timer = 60;
        }
    }
    else if (entity->state == 1)
    {
        entity->scrollWithBg = 1;
        EntityAccelerateAndMove(entity, 0x40000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            EntityDelete(entity);
    }
}

/* Create a seed shot at the orb + (x, y) that waits `delay` frames (box
 * 0x0A06FDFD = x -3, y -3, w 6, h 10, harmed by ChaosSeedShot_DamagePlayerOnContact, bounced by
 * ChaosSeedShot_OnHit). */
void ChaosOrb_SpawnSeedShot(struct EwramData_EntityData *orb, s32 x, s32 y, s32 delay)
{
    struct EwramData_EntityData *owner = orb->ext.chaosChild.link;
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosSeedShot_Update);
    struct HitBox box = { -3, -3, 6, 10 };
    struct ChaosData *data = (struct ChaosData *)owner->ext.chaos.data.value;
    if (shot != NULL)
    {
        s32 graphics;

        shot->posX.part16.integer = 0xFF80;
        shot->posY.part16.integer = 0xFF80;
        OFFSET_X(shot) = x;
        OFFSET_Y(shot) = y;
        shot->spriteLayer = orb->spriteLayer - 1;
        graphics = data->graphics;
        shot->unk_510.part8.basePalette = data->palette2;
        Sprite_Attach(shot, sChaosSpriteSheet, graphics, shot->unk_510.part8.basePalette);
        Anim_StartIndexed(shot, 4, 2, 0);
        Anim_Update(shot);
        Hit_Enable(shot, 8, ChaosSeedShot_DamagePlayerOnContact, (s32)ChaosSeedShot_OnHit);
        HitBox_SetBody(shot, &box);
        HitBox_SetAttack(shot, &box);
        shot->hit.flags &= ~4;
        shot->hit.flags &= ~2;
        shot->unk_518.part8.enemyId = orb->unk_518.part8.enemyId;
        shot->ext.chaosChild.link = orb;
        shot->scrollWithBg = 0;
        shot->timer = delay;
        shot->hidden = 1;
        shot->attack = 100;
        shot->enemyFlags |= 1;
        shot->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

