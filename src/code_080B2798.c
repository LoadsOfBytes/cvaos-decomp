#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

void PlaySong(u32 song);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, void *graphics, void *sprite, u8 palette, void *paletteData);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity);
extern void RockArmorRock_DamagePlayerOnContact(void);
extern void ArmorProjectile_OnHit(void);
extern void RockArmorRockPiece_Update(struct EwramData_EntityData *entity);

/**
 * @brief B2798 | Thrown boss projectile: load its graphics (deleted when
 * that fails), hit box 0x2020F0F0, arc towards the player via Entity_Walk;
 * on landing or when its HP runs out burst into eight RockArmorRockPiece_Update pieces.
 * Frozen while timeStopTimer is set; deleted when far outside the screen.
 */
void RockArmorRock_Update(struct EwramData_EntityData *entity)
{
    u8 hit[0x10];
    s32 i;
    struct EwramData_EntityData *piece;
    s32 box;

    Hit_CheckWithPlayer(hit, entity);
    if (entity->state != 0)
    {
        if ((s16)entity->unk_518.part16.hp == 0 && entity->state != 2)
            entity->state = 2;
        if (entity->state != 2 && gEwramData->gameState.timeStopTimer != 0)
            return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)0x081E41BC, (void *)0x0820B83C, 1, (void *)0x0823FFD8))
            goto delete;
        Anim_StartIndexed(entity, 3, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = 0x13;
        Hit_Enable(entity, 10, RockArmorRock_DamagePlayerOnContact, (s32)ArmorProjectile_OnHit);
        box = 0x2020F0F0;
        HitBox_SetAttackRaw(entity, box);
        HitBox_SetBodyRaw(entity, box);
        entity->unk_4FC.whole = -0x2000 - (Entity_GetTargetDistance(entity) << 10);
        entity->posY.whole += -0x3A0000;
        if (entity->flipX)
            entity->unk_4FC.whole = -entity->unk_4FC.whole;
        entity->velY.whole = -0x20000;
        entity->accelY.whole = 0x2000;
        entity->attack += entity->attack >> 2;
        entity->scrollWithBg = 1;
        entity->state = 1;
    case 1:
        if (Entity_Walk(entity, entity->unk_4FC.whole, (void *)0x08528738, 2))
            entity->state = 2;
        break;
    case 2:
        PlaySong(0x80);
        for (i = 0; i <= 7; i++)
        {
            piece = EntityCreateInRange(0x49, 0xCF, (u32 *)RockArmorRockPiece_Update);
            if (piece == NULL)
                break;
            piece->posX.whole = entity->posX.whole + RandomSigned(0x100000);
            piece->posY.whole = entity->posY.whole + RandomSigned(0x100000) - 0x80000;
            piece->flipX = RandomNumberGenerator() & 1;
            piece->spriteLayer = 0x12;
            piece->scrollWithBg = 1;
        }
    delete:
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3))
        EntityDelete(entity);
}
