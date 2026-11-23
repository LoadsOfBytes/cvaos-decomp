/* Chaos: mark on the player (3D40). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
};
struct MarkFrames {
    s32 v[3];
};
struct MarkOffsets {
    struct {
        s32 x;
        s32 y;
    } v[3];
};

extern u8 sChaosSpriteSheet[];
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern struct MarkFrames sChaosMarkFrames;
extern struct MarkOffsets sChaosMarkOffsets;
extern s32 Entity_GetVisibilityPattern(s32);
extern void ChaosMark_Update(struct EwramData_EntityData *);
void ChaosHalo_Update(struct EwramData_EntityData *entity);
extern void ChaosAura_Update(struct EwramData_EntityData *);

#define KIND(e) (*(u8 *)&e->ext.chaosOrb.kind)

/* Create the mark of orb kind `kind` next to the player. */
void Chaos_CreateMark(struct EwramData_EntityData *core, s32 kind)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *mark = EntityCreateInRange(0x49, 0xCF, (u32 *)ChaosMark_Update);
    struct MarkFrames frames;
    struct ChaosData *data;

    frames = sChaosMarkFrames;
    data = (struct ChaosData *)core->ext.chaos.data.value;
    if (mark != NULL)
    {
        s32 graphics;

        mark->posX.part16.integer = 0xFF80;
        mark->posY.part16.integer = 0xFF80;
        mark->spriteLayer = (*ewram)->playerState.player->spriteLayer - 1;
        graphics = data->graphics;
        mark->unk_510.part8.basePalette = data->palette2;
        Sprite_Attach(mark, sChaosSpriteSheet, graphics, mark->unk_510.part8.basePalette);
        mark->animMode = 2;
        mark->animFrame = frames.v[kind] + 0x14;
        mark->unk_518.part8.enemyId = core->unk_518.part8.enemyId;
        mark->ext.chaosChild.link = core;
        mark->scrollWithBg = 0;
        KIND(mark) = kind;
        mark->timer = 40;
        mark->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
