#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "entity_hit.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern s32 RandomSigned(s32);
extern struct EwramData_EntityData *Effect_CreateFromTable(s32, s32, s32, s32, s32, u8);
extern void DeathPillar_DamagePlayerOnContact(struct EwramData_EntityData *);

/* Pillar rising under the player (x of the player, y 0x90): grows, then
 * shrinks along a sine while throwing debris; hits during frames 15..17. */
void DeathPillar_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ew = &gEwramData;
    struct EwramData_EntityData *parent = &(*ew)->entityData[(u8)entity->ext.deathHelper.ownerIndex];
    struct HitResult res;
    u16 slot;

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)0x08119498, (void *)0x0820BC1C, 2, (void *)0x08249C48))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->posX.whole = (*ew)->playerState.player->posX.whole;
        entity->posY.part16.integer = 0x90;
        entity->attack = parent->attack + (parent->attack >> 2);
        entity->unk_518.part16.hp = 1;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x11;
        Anim_StartIndexed(entity, 0xD, 2, 0);
        Anim_Update(entity);
        entity->ext.deathPillar.scaleX = 0x1000;
        Hit_Enable(entity, 8, DeathPillar_DamagePlayerOnContact, 0);
        HitBox_SetAttackRaw(entity, 0x800880FC);
        entity->hit.flags |= 4;
        PlaySong(0x11F);
        entity->state = 1;
    case 1:
        ObjAffine_Set(0, entity->ext.deathPillar.scaleX, entity->ext.deathPillar.scaleY, entity->flipX, 0, entity->affineIndex);
        if (entity->ext.deathPillar.scaleY <= 0xFFFF)
        {
            entity->ext.deathPillar.scaleY += 0x1000;
            break;
        }
        entity->timer = 0x20;
        entity->state = 2;
        break;
    case 2:
        entity->ext.deathPillar.scaleX = Sin(entity->timer << 10);
        ObjAffine_Set(0, entity->ext.deathPillar.scaleX, 0x10000, entity->flipX, 0, entity->affineIndex);
        if (!(entity->timer & 7))
        {
            s32 x = entity->posX.whole + RandomSigned(0x100000);
            s32 vx = -0x800 - (RandomNumberGenerator() & 0xFFF);
            struct EwramData_EntityData *debris = Effect_CreateFromTable(x, entity->posY.whole - 0x40000, 0, vx, 4, 6);
            if (debris != NULL)
                debris->velY.whole = -0x2000 - ((RandomNumberGenerator() & 0x1FF) << 8);
        }
        if (entity->timer == 0)
            goto destroy;
        if ((u8)(entity->timer - 0xF) <= 2)
            Hit_CheckWithPlayer(&res, (u8 *)entity);
        entity->timer--;
        break;
    case 3:
    destroy:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3))
        entity->state = 3;
}
