/* Spawn a body part of a multi-part enemy (Legion_CreateCorePart). The do/while (0)
 * wrappers only weight references for global-alloc (kind r6, entity r7,
 * data r4); they do not change the code. */
#include "code_08009A0.h"
#include "legion_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern struct EwramData_EntityData *LegionCorePart_CreateDetail(struct EwramData_EntityData *entity, s32 x, s32 y, s32 kind, s32 tiles);
extern void LegionCorePart_Update(struct EwramData_EntityData *entity);
extern void Legion_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void LegionCorePart_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
extern u8 sLegionSpriteSheet[];

#define CORE(k) LegionCorePart_CreateDetail(part, -LEGION_PART_OFFSET_X(part), -LEGION_PART_OFFSET_Y(part), k, *data)

struct EwramData_EntityData *Legion_CreateCorePart(struct EwramData_EntityData *entity, s32 x, s32 y, s32 kind)
{
    struct EwramData_EntityData *part = Entity_CreateObject((u32 *)LegionCorePart_Update);
    u32 box = 0x1010F0F0;
    s32 *data = (s32 *)LEGION_DATA(entity);
    u16 palette;
    s32 tiles;

    if (part == NULL)
        goto fail;
    LEGION_PART_OFFSET_X(part) = x;
    LEGION_PART_OFFSET_Y(part) = y;
    part->posX.whole = entity->posX.whole + x;
    part->posY.whole = entity->posY.whole + y;
    do { do { LEGION_PART_KIND(part) = kind; } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    if ((u8)kind == 3)
    {
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
    }
    part->spriteLayer = entity->spriteLayer - 2;
    do { do { tiles = *data; } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    LEGION_PART_TILES(part) = tiles;
    part->unk_510.part8.basePalette = LEGION_ANIM_PALETTE(entity);
    Sprite_Attach(part, sLegionSpriteSheet, tiles, part->unk_510.part8.basePalette);
    do { part->unk_518.part8.enemyId = entity->unk_518.part8.enemyId; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    switch (kind)
    {
    case 0:
        part->animFrame = 0;
        CORE(0);
        CORE(1);
        CORE(2);
        break;
    case 1:
        part->animFrame = 1;
        CORE(0);
        CORE(2);
        CORE(3);
        break;
    case 2:
        part->animFrame = 2;
        CORE(1);
        CORE(4);
        break;
    case 3:
        part->animFrame = 0x36;
        part->velX.whole = 0xC000;
        part->velY.whole = 0xC000;
        CORE(3);
        CORE(4);
        palette = ObjAffine_Alloc(1);
        if ((s16)palette < 0)
        {
            EntityDelete(part);
            goto fail;
        }
        part->affineIndex = palette;
        part->affine = 1;
        ObjAffine_SetBiased(0, part->velX.whole, part->velY.whole, part->flipX, part->flipY, part->affineIndex);
        break;
    case 10:
        part->animFrame = 3;
        CORE(3);
        CORE(4);
        break;
    }
    part->animMode = 2;

    do { LEGION_PART_OWNER(part) = entity; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    part->scrollWithBg = 0;
    part->flipX = 0;
    do { part->unk_518.part16.hp = entity->unk_518.part16.hp; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    do { part->attack = entity->attack; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    if (!(*((u8 *)gEwramData + 0x8E) & 0x40))
    {
        Hit_Enable(part, 8, Legion_DamagePlayerOnContact, (s32)LegionCorePart_OnHit);
        HitBox_SetBody(part, &box);
        HitBox_SetAttack(part, &box);
        part->hit.flags &= 0xFB;
        part->hit.flags &= 0xFD;
    }
    part->drawFunc = (u32 *)Sprite_DrawEntity;
    return part;
fail:
    return NULL;
}
