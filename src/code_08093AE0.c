/* MedusaHead_OnHit (hit with HP / death), generated with a template script from the
 * template of WingedSkeleton_OnHit and checked byte for byte. */
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

extern u8 sMedusaHeadGfx[];
extern u8 sMedusaHeadPalette[];
extern u8 sMedusaHeadSpriteSheet[];

/**
 * @brief 93AE0 | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, song 0x72, animation 3, palette into
 * 0x53E bits 3-6, timer 0x28, state 2). Same template as WingedSkeleton_OnHit.
 */
void MedusaHead_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x72);
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/**
 * @brief 93B68 | Load the graphics (sMedusaHeadGfx), palette 0 / 1 of
 * sMedusaHeadPalette by bit 0 of spawnParam1 and animation set sMedusaHeadSpriteSheet;
 * returns 0 when ObjGfx_LoadResource fails.
 */
s32 MedusaHead_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sMedusaHeadGfx, 0);

    if (gfx < 0)
        return 0;

    if (entity->spawnParam1 & 1)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sMedusaHeadPalette, 1);
    else
        entity->basePalette = ObjPalette_FindOrLoadVariant(sMedusaHeadPalette, 0);
    Sprite_Attach(entity, sMedusaHeadSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
