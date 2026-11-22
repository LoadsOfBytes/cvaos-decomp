/* Pickup_TrySpawnOnHit. */
#include "code_08009A0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/* Same callee as Enemy_PreUpdate's, whose result is ignored there; here the
 * low halfword is tested against 0x1E. */
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Pickup_CreateMana(s32 x, s32 y, s32 large, s32 jump);
extern void Pickup_CreateGold(s32 x, s32 y, s32 index);

/**
 * @brief On hit-mask 0x1E select and spawn the existing mana/gold/item reward, with character and full-MP substitutions; return one for the hit even if allocation fails.
 */
s32 Pickup_TrySpawnOnHit(struct EwramData_EntityData *entity, s32 offsetY, u8 kind, s32 amount)
{
    u8 collision[16];
    s32 x = (s16)entity->posX.part16.integer;
    s32 y = (s16)entity->posY.part16.integer + offsetY;
    u16 touch;

    touch = Hit_CheckWithPlayer(collision, entity);
    if (touch & 0x1E)
    {
        if (gEwramData->playerStatus.currentCharacter != 0 && kind != 0)
        {
            kind = 0;
            amount = 0;
        }
        switch (kind)
        {
            case 0:
                if (gEwramData->playerStatus.currentMP != gEwramData->playerStatus.maxMP || gEwramData->playerStatus.currentCharacter != 0)
                    Pickup_CreateMana(x, y, amount, 0);
                else
                    Pickup_CreateGold(x, y, (RandomNumberGenerator() & 1) + amount);
                break;
            case 1:
                Pickup_CreateGold(x, y, amount);
                break;
            case 2:
                Pickup_CreateItem(x, y, 2, amount, 0);
                break;
        }
        return 1;
    }
    return 0;
}
