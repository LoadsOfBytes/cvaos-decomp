/* OBJ-window sprite bound to an owner slot. */
#include "code_08009A0.h"
#include "death_entity.h"
#include "code/code_08039340.h"
#include "structs/main.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief BD20C | Sprite used as OBJ window, owned by entity slot unk_4F8.
 * Keeps DISPCNT bit 15 set every frame.  State 0 sets WINOUT to 0x2F3F,
 * loads the graphics (sCommonObjectGfx / sCommonObjectSpriteSheet, palette 6), priority
 * 0x12, frame 0x3F, draw callback Sprite_DrawEntity, objMode = 2 and places
 * itself at the owner + (0x20, 0x18).  It is deleted once the owner's
 * state exceeds 2.
 */
void DeathFloatingWeaponWindow_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)DEATH_OWNER_INDEX(entity)];
    s32 slot;

    gUnk_03002CB0.dispCnt |= 0x8000; /* OBJ window on */
    switch (entity->state)
    {
    case 0:
        gDisplayRegisters.winOut_L = 0x3F;
        gDisplayRegisters.winOut_H = 0x2F;
        slot = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, slot, 6);
        entity->spriteLayer = 0x12;
        entity->animFrame = 0x3F;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->objMode = 2;
        entity->posX.part16.integer = owner->posX.part16.integer + 0x20;
        entity->posY.part16.integer = owner->posY.part16.integer + 0x18;
        entity->state = 1;
    case 1:
        if (owner->state > 2)
            EntityDelete(entity);
        break;
    }
}
