/* Dryad projectile: ballistic launch, animation sequence, then a timed
 * player-following effect with contact damage every sixteen frames. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *, s32);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void DryadSeed_OnContact(struct EwramData_EntityData *);
extern void DryadSeed_OnHit(struct EwramData_EntityData *);
extern s32 Player_GetOverheadY(void);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern u8 sDryadGfx[], sDryadPalette[], sDryadSpriteSheet[], sUnk_08528604[];
static inline u8 AdvanceDryadEffect(struct EwramData_EntityData *entity)
{
    if (sUnk_08528604[entity->subState] == 0xFF)
        return 1;
    if (entity->subState == 0 || (entity->animFlags & 4))
    {
        u8 index = entity->subState++;
        Anim_StartIndexed(entity, sUnk_08528604[index], 0, 0);
    }
    return 0;
}
static inline s16 PlayerEffectX(void)
{
    return gEwramData->playerState.player->posX.part16.integer;
}
void DryadSeed_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct PlayerContactDamage contact;
    if (EntityIsOutsideScreen(entity, 1))
        entity->deleteRequested = 1;
    switch (entity->state)
    {
    case 0:
        Entity_LoadGraphics(entity, sDryadGfx, sDryadPalette, 0, sDryadSpriteSheet);
        Hit_Enable(entity, 8, DryadSeed_OnContact, (s32)DryadSeed_OnHit);
        HitBox_SetBodyRaw(entity, 0x0404FEFE);
        HitBox_SetAttackRaw(entity, 0x0404FEFE);
        entity->spriteLayer = 17;
        Anim_StartIndexed(entity, 8, 0, 1);
        entity->scrollWithBg = 1;
        entity->state++;
        /* fall through */
    case 1:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x1000;
        break;
    case 2:
        if (AdvanceDryadEffect(entity))
        {
            entity->state = 3;
            entity->timer = 120;
        }
        /* fall through */
    case 3:
        entity->posX.part16.integer = PlayerEffectX();
        entity->posY.part16.integer = Player_GetOverheadY();
        if (entity->state == 3)
        {
            if ((entity->timer & 15) == 0)
            {
                contact.attributes = 0;
                contact.unk_4 = 0;
                contact.attack = entity->attack;
                contact.reaction = 2;
                contact.sourceX = entity->posX.whole;
                contact.sourceY = entity->posY.whole;
                Player_ApplyContactDamage(&contact);
            }
            if (--entity->timer == 0)
                entity->deleteRequested = 1;
        }
        break;
    }
    Anim_Update(entity);
    Hit_CheckWithPlayer(collision, entity);
}
