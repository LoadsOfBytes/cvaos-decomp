#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

void PlaySong(u32 song);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void BaloreSpark_DamagePlayerOnContact(struct EwramData_EntityData *entity);

/**
 * @brief B5224 | Boss part with a hit box (BaloreSpark_DamagePlayerOnContact): load its own
 * palette, wait timer frames, then move sideways; when its animation ends
 * spawn falling sparks (Effect_SpawnShared) every fourth frame and delete itself.
 */
void BaloreSpark_Update(struct EwramData_EntityData *entity)
{
    u8 hit[0x10];
    struct EwramData_EntityData *parent = BALORE_BODY(entity);
    struct EwramData_EntityData *spark;
    u8 timer;

    switch (entity->state)
    {
    case 0:
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant((void *)0x0820B92C, 0x10);
        Sprite_Attach(entity, (void *)0x08244220, BALORE_GFX_BYTE(parent), entity->unk_510.part8.basePalette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Anim_StartIndexed(entity, 0x33, 2, 0);
        entity->spriteLayer = 0x13;
        Hit_Enable(entity, 8, BaloreSpark_DamagePlayerOnContact, 0);
        HitBox_SetAttackRaw(entity, 0x2810D8F8);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        entity->hidden = 1;
        entity->state = 1;
    case 1:
        timer = entity->timer;
        if (timer != 0)
            goto count;
        PlaySong(0x6E);
        entity->hidden = 0;
        entity->state = 2;
    case 2:
        EntityAccelerateAndMoveX(entity, 0x80000);
        Hit_CheckWithPlayer(hit, entity);
        if (entity->animStep == 3)
            entity->hit.flags &= ~2;
        else
            entity->hit.flags |= 2;
        if (Anim_Update(entity) != 3)
            break;
        spark = Effect_SpawnShared(entity->posX.whole + RandomSigned(0x100000), entity->posY.whole + RandomSigned(0x40000) - 0x40000, 1, 7);
        if (spark != NULL)
        {
            spark->velY.whole = RandomSigned(0x2000) - 0x8000;
            spark->accelY.whole = -spark->velY.whole / 64;
        }
        entity->state = 3;
        entity->timer = 8;
        break;
    case 3:
        if ((entity->timer & 3) == 0)
        {
            spark = Effect_SpawnShared(entity->posX.whole + RandomSigned(0x200000), entity->posY.whole + RandomSigned(0x40000) - 0x40000, 5, 6);
            if (spark != NULL)
            {
                spark->drawSkip = 1;
                spark->velY.whole = RandomSigned(0x2000) - 0x8000;
                spark->accelY.whole = -spark->velY.whole / 64;
            }
        }
        timer = entity->timer;
        if (timer != 0)
        {
    count:
            entity->timer = timer - 1;
            break;
        }
        entity->deleteRequested = 1;
        break;
    }
}
