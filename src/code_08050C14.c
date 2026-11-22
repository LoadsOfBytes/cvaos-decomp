#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Object0E_Draw(struct EwramData_EntityData *);
extern void Object0E_OnHit(void);
extern void HitBox_SetBodyRaw(struct EwramData_EntityData *, u32);
extern void Entity_SnapToGround(struct EwramData_EntityData *);

struct Object0EGraphics {
    void *graphics;
    void *palette;
    void *sprites;
};

struct Object0EVariant {
    u8 graphics;
    u8 palette;
    u8 animation;
    u8 pad_3[3];
    u16 skipGroundSnap;
    u32 box;
};

extern struct Object0EGraphics sObject0EGraphics[];
extern struct Object0EVariant sObject0EVariants[];

void Object0ECreate(struct EwramData_EntityData *entity)
{
    struct Object0EVariant *variant = &sObject0EVariants[entity->unk_514.part16.spawnParam0];
    struct Object0EGraphics *graphics = &sObject0EGraphics[variant->graphics];
    if (!Entity_LoadGraphics(entity, graphics->graphics, graphics->palette, variant->palette, graphics->sprites))
    {
        EntityDelete(entity);
        return;
    }
    entity->spriteLayer = 0x14;
    Anim_StartIndexed(entity, variant->animation, 0, 1);
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Object0E_Draw;
    Hit_Enable(entity, 0, NULL, Object0E_OnHit);
    entity->hit.flags |= 2;
    HitBox_SetBodyRaw(entity, variant->box);
    if (variant->skipGroundSnap == 0) Entity_SnapToGround(entity);
    entity->scrollWithBg = 1;
}

#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08060B98.h"
extern u8 sUnk_08526370[][4];
extern s32 Pickup_TrySpawnOnHit(struct EwramData_EntityData *, s32, s32, s32);
extern void Object0E_SpawnPieces(struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);

void Object0EUpdate(struct EwramData_EntityData *entity)
{
    u8 *entry;
    if (!Entity_IsInRect(entity, -0x20, -0x20, 0x130, 0x120))
    {
        entity->hidden = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->hidden = 0;
        entry = sUnk_08526370[entity->unk_514.part16.spawnParam1];
        if (Pickup_TrySpawnOnHit(entity, -20, entry[0], entry[1]))
        {
            Object0E_SpawnPieces(entity);
            PlaySong(0xA2);
            entity->hidden = 1;
            entity->timer = 0x28;
            entity->state++;
        }
        break;
    case 1:
        if (entity->timer > 30 && (entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x100000, 0x80000, 0x100000, entity->spriteLayer - 1);
        if (--entity->timer == 0xFF) entity->deleteRequested = 1;
        break;
    }
}
