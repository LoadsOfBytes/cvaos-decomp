/* Chronomage casting and transition sequence. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Chronomage_UpdateWait(struct EwramData_EntityData *);
extern void Chronomage_UpdateCast(struct EwramData_EntityData *);
extern void Chronomage_UpdateEscape(struct EwramData_EntityData *);
extern void Chronomage_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Chronomage_OnHit(struct EwramData_EntityData *);
extern u8 sChronomageGfx[], sChronomagePalette[], sChronomageSpriteSheet[];

#include "code_0800B700.h"
#include "code_080109F4.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
extern u8 sChronomageDestinationRoom[];
extern void ChronomageCastEffect_Update(struct EwramData_EntityData *);

/**
 * @brief Cast state (1): sequence animations, sound and palette changes.
 * Branch on the global stop timer and Player_HasAbilityOneBased(6); either start
 * the transition through RoomJump_SetDestination or spawn the affine effect and escape.
 * The meaning of the player flags and transition data remains unresolved.
 */
void Chronomage_UpdateCast(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect;
    switch (entity->subState)
    {
    case 0:
        if (gEwramData->playerState.player->unk_4F4.whole & 0x360)
        {
            entity->subState = 2;
            break;
        }
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        /* fall through */
    case 1:
        if (entity->animFlags & 4) entity->subState = 2;
        break;
    case 2:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState = 3;
        entity->timer = 0x18;
        break;
    case 3:
        if (entity->timer != 0)
        {
            if (--entity->timer == 8) PlaySong(0xEC);
        }
        else
        {
            entity->timer = 0;
            entity->subState = 4;
        }
        break;
    case 4:
        if (gEwramData->gameState.timeStopTimer == 0)
        {
            gEwramData->paletteInvert = entity->timer << 12;
            if (entity->timer <= 0xF)
            {
                entity->timer++;
                break;
            }
            if (!Player_HasAbilityOneBased(6)) goto menu;
        }
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x8000, 0x10000, 0x10);
        entity->subState = 6;
        break;
menu:
        entity->subState = 5;
        Game_SetMenusEnabled(0);
        break;
    case 5:
        RoomJump_SetDestination(sChronomageDestinationRoom, 0, 0x100, 0x20, 0x70);
        RoomJump_Begin(&gEwramData->gameState);
        break;
    case 6:
        gEwramData->paletteInvert = 1;
        if (!(entity->animFlags & 4)) break;
        effect = EntityCreateInRange(0x49, 0xCF, (u32 *)ChronomageCastEffect_Update);
        if (effect != NULL)
        {
            effect->posX.part16.integer = entity->posX.part16.integer;
            effect->posY.part16.integer = (u16)entity->posY.part16.integer - 0x20;
            effect->scrollWithBg = 1;
        }
        entity->subState = 7;
        entity->timer = 0x30;
        /* fall through */
    case 7:
        if (entity->timer != 0) entity->timer--;
        else
        {
            entity->state = 2;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
