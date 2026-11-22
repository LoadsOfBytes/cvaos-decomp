/* SoulTypeSelector_SetDirection; only
 * matches without -f2003-patch (NO_2003_PATCH_SRCS in ../Makefile). The
 * caller passes entity slot 21 (gEwramData + 0xFB8). */
#include "structs/ewram.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))
/* Bytes 0x51C (turned around) and 0x51D (direction) of this entity. */
#define ENTITY_TURNED(e) (((s8 *)&(e)->unk_51C)[0])
#define ENTITY_DIR(e) (((s8 *)&(e)->unk_51C)[1])
#define ENTITY_DIR_BYTE(e) ((u8 *)&e->unk_51C)[1]
/* Signed icon rotation step per frame (SoulTypeIcons.dir). */
#define SOUL_SELECTOR_SPIN(e) e->unk_504.whole

/**
 * @brief 486E0 | Set the direction byte 0x51D to dir (-1 or 1) and
 * point unk_4F6 and the rotation step that way (negative for -1); in state 3 a
 * change of a set direction marks byte 0x51C. Next state.
 */
void SoulTypeSelector_SetDirection(struct EwramData_EntityData *entity, s8 dir)
{
    if (ENTITY_DIR(entity) != 0 && entity->state == 3 && ENTITY_DIR(entity) != dir)
        ENTITY_TURNED(entity) = 1;
    if (dir == -1)
    {
        ENTITY_DIR_BYTE(entity) = 0xFF; // -1, stored as an unsigned byte
        entity->unk_4F4.part16.unk_4F6 = -ABS(entity->unk_4F4.part16.unk_4F6);
        SOUL_SELECTOR_SPIN(entity) = -ABS(SOUL_SELECTOR_SPIN(entity));
    }
    else
    {
        ENTITY_DIR(entity) = 1;
        entity->unk_4F4.part16.unk_4F6 = ABS(entity->unk_4F4.part16.unk_4F6);
        SOUL_SELECTOR_SPIN(entity) = ABS(SOUL_SELECTOR_SPIN(entity));
    }
    entity->state++;
}
