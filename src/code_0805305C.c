/* Object17Create: spawns the object and its child effects; matched. */
#include "structs/ewram.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"

extern u8 sObject17Gfx[], sUnk_08209FD8[], sObject17SpriteSheet[];
extern const s16 sUnk_08118590[];
extern const u16 sObject17Xs[], sObject17Ys[];
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern s16 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);
extern void Player_RegisterSolidObject(struct EwramData_EntityData *, void *);
extern void Object17Follower_Update(struct EwramData_EntityData *);
extern void Object17Overlay_Update(struct EwramData_EntityData *);
extern void Object17Effect_Update(struct EwramData_EntityData *);

static inline struct BgLayerState *Bg1(void)
{
    return &gEwramData->bgInfo[1];
}

void Object17Create(struct EwramData_EntityData *entity)
{
    s32 i;
    s32 j;
    u32 py;
    s32 t;
    const s16 *tbl;
    struct EwramData_EntityData *e;
    struct BgLayerState *bg;

    if (!Entity_LoadGraphics(entity, sObject17Gfx, sUnk_08209FD8, 4, sObject17SpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->spriteLayer = 0x11;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    t = GetEntityRoomYPositionInteger(gEwramData->playerState.player) << 16;
    i = 0;
    if (sUnk_08118590[0] + 0x20 >= ({ py = (u32)t >> 16; t >> 16; }))
    {
        entity->subState = i;
        do { tbl = sUnk_08118590; } while (0);
    }
    else
    {
    next:
        i++;
        do { tbl = sUnk_08118590; } while (0);
        if (i <= 2)
        {
            if (tbl[i] + 0x20 < (s16)py)
                goto next;
            entity->subState = i;
        }
    }
    bg = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0x78 - bg->xPos.part16.integer;
    entity->posY.part16.integer = tbl[entity->subState] - bg->yPos.part16.integer;
    Player_RegisterSolidObject(entity, (void *)0x08118598);
    entity->roomActive = 1;
    e = EntityCreateInRange(0x49, 0xCF, (u32 *)Object17Follower_Update);
    if (e != NULL)
    {
        if (!Entity_LoadGraphics(e, sObject17Gfx, sUnk_08209FD8, 4, sObject17SpriteSheet))
        {
        delete:
            EntityDelete(e);
            return;
        }
        Anim_StartIndexed(e, 1, 0, 1);
        Anim_Update(e);
        e->unk_4F8.unk_4F8_entity = entity;
        e->scrollWithBg = 1;
        e->posX.whole = entity->posX.whole;
        e->posY.whole = entity->posY.whole;
        e->spriteLayer = 0x12;
    }
    for (i = 0; i <= 1; i++)
    {
        e = EntityCreateInRange(0x49, 0xCF, (u32 *)Object17Overlay_Update);
        if (e == NULL)
            continue;
        if (!Entity_LoadGraphics(e, sObject17Gfx, sUnk_08209FD8, 4, sObject17SpriteSheet))
            goto delete;
        Anim_StartIndexed(e, 3, 0, 0);
        Anim_Update(e);
        e->unk_4F8.unk_4F8_entity = entity;
        e->scrollWithBg = 1;
        e->flipX = i;
        e->hidden = 1;
        e->posX.whole = entity->posX.whole;
        e->posY.whole = entity->posY.whole;
        e->spriteLayer = 0x11;
    }
    for (i = 0; i <= 1; i++)
    {
        for (j = 0; j <= 7; j++)
        {
            e = EntityCreateInRange(0x49, 0xCF, (u32 *)Object17Effect_Update);
            if (e == NULL)
                break;
            e->scrollWithBg = 1;
            {
                const u16 *tx = sObject17Xs;
                e->posX.part16.integer = tx[i] - ({ Bg1(); })->xPos.part16.integer;
            }
            {
                const u16 *ty = sObject17Ys;
                e->posY.part16.integer = ty[j] - ({ Bg1(); })->yPos.part16.integer;
            }
            e->flipX = i;
            e->subState = i + 4;
        }
    }
}
