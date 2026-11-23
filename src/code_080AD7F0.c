/* Giant Worm / Poison Worm: create the root and four linked affine segments. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void Worm_SettleOnGround(struct EwramData_EntityData *);
extern void WormSegment_Update(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Worm_Draw(struct EwramData_EntityData *);
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern void Worm_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void PoisonWorm_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Worm_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern u8 sWormGfx[], sWormPalette[], sWormSpriteSheet[];
/**
 * @brief Shared create of Giant Worm (variant 0) and Poison Worm (variant 1):
 * graphics, affine head, body segments (WormSegment_Update), hit boxes
 * (Poison Worm contact with attribute 0x40) and Worm_Draw.
 */
void Worm_Init(struct EwramData_EntityData *root, s32 variant)
{
    u16 matrix;
    struct EwramData_EntityData *previous; register struct EwramData_EntityData *part asm("r5"); /* FAKEMATCH: loop pointers */
    s32 spacing, tiles;
    root->scrollWithBg = 1;
    matrix = ObjAffine_Alloc(1);
    if ((s16)matrix < 0)
    {
        EntityDelete(root);
        return;
    }
    root->affineIndex = matrix;
    root->affine = 1;
    Worm_SettleOnGround(root);
    previous = root;
    root->ext.worm.root = root;
    spacing = 0xA0000;
    if (root->flipX)
        spacing = -spacing;
    {
    s32 i = 0;
    for (; i <= 3;)
    {
        part = Entity_CreateObject(WormSegment_Update);
        if (part == NULL)
            goto fail;
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
            goto fail;
        {
            u8 slot = *(u8 *)&part->ext.raw.unk_4F4;
            *(u8 *)&previous->ext.raw.unk_504 = slot;
        }
        part->affineIndex = matrix;
        part->affine = 1;
        i++;
        part->ext.worm.wavePhase = i << 12;
        part->ext.worm.root = root;
        part->ext.worm.previous = previous;
        part->ext.worm.segmentIndex = i;
        part->posX.whole = previous->posX.whole + spacing;
        part->posY.whole = previous->posY.whole;
        part->scrollWithBg = 1;
        Worm_SettleOnGround(part);
        { u8 t_ = root->enemyId; register u8 *d_ asm("r1") = &part->enemyId; *d_ = t_; } /* FAKEMATCH: address in r1 */
        Enemy_ResetFromTemplate(part);
        previous = part;
    }
    }
    Enemy_ResetFromTemplate(root);
    root->drawFunc = (u32 *)Worm_Draw;
    tiles = ObjGfx_LoadResource(sWormGfx, 0);
    root->basePalette = ObjPalette_FindOrLoadVariant(sWormPalette, (u8)variant);
    Sprite_Attach(root, sWormSpriteSheet, tiles, root->basePalette);
    if (variant == 0)
        Anim_StartIndexed(root, 3, 0, 1);
    else
        Anim_StartIndexed(root, 0, 0, 1);
    Anim_Update(root);
    ObjAffine_Set(0, 0x10000, 0x10000, root->flipX, 0, root->affineIndex);
    root->ext.worm.variant = variant;
    root->spriteLayer = 17;
    if (gEwramData->gameState.bestiaryPreview)
    {
        root->state = 6;
        return;
    }
    if (root->ext.worm.variant == 0)
        Hit_Enable(root, 8, Worm_DamagePlayerOnContact, (s32)Worm_OnHit);
    else
    {
        if (0)
        {
        fail:
            root->ext.worm.aggressive = 1;
            root->state = 4;
            return;
        }
        Hit_Enable(root, 8, PoisonWorm_DamagePlayerOnContact, (s32)Worm_OnHit);
    }
    HitBox_SetBodyRaw(root, 0x1010F8F8);
    HitBox_SetAttackRaw(root, 0x1010F8F8);
}


