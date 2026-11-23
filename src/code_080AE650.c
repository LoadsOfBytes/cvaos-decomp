/* Dryad death: palette flashes, smoke, shrinking affine sprite. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);

/**
 * @brief Death state (state 3): effects with sound 0x6E every 8 frames for 64
 * frames, then free the affine slot and request deletion.
 */
void Dryad_UpdateDeath(struct EwramData_EntityData *entity)
{
    u16 slot;
    switch (entity->subState)
    {
    case 0:
        entity->timer = 0x40;
        entity->subState++;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot >= 0)
        {
            entity->affineIndex = slot;
            entity->affine = 1;
        }
        break;
    case 1:
        if (entity->timer == 0x10) entity->drawSkip = 1;
        if (!(entity->timer & 7))
        {
            PlaySong(0x6E);
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x100000, 0x100000, 0x100000, entity->spriteLayer - 1);
        }
        if (--entity->timer == 0)
        {
            entity->deleteRequested = 1;
            entity->hidden = 1;
            if (entity->affine)
            {
                ObjAffine_Free(entity->affineIndex);
                entity->affine = 0;
            }
            return;
        }
        break;
    }
    if (Entity_GetVisibilityPattern(entity->timer)) entity->palette = 5;
    else entity->palette = entity->unk_510.part8.basePalette;
    if (entity->affine)
        ObjAffine_Set(0, entity->timer << 10, 0x10000, entity->flipX, 0, entity->affineIndex);
}

void DryadSeed_OnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    if ((hit[0] & 1) && entity->state <= 1)
    {
        entity->state = 2;
        entity->timer = 0xB4;
        entity->accelX.part16.integer = entity->posX.part16.integer - attacker->posX.part16.integer;
        entity->accelY.part16.integer = entity->posY.part16.integer - attacker->posY.part16.integer;
        entity->ext.dryadSeed.unusedAttacker = attacker;
        entity->hit.flags |= 4;
    }
}
