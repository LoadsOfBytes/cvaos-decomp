#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateView
{
    void (*create)(struct EwramData_EntityData *);
    u32 *update;
    u8 pad_8[0xC - 0x8];
    u16 hp;
    u8 pad_E[0x24 - 0xE];
};

extern const struct EnemyTemplateView sEnemyData[];
extern s32 Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);
extern s32 RandomSigned(s32);
extern void Object18_SpawnDebris(struct EwramData_EntityData *parent);
extern struct EwramData_EntityData *Pickup_CreateGold(s32 x, s32 y, s32 index);
extern struct EwramData_EntityData *Pickup_CreateMana(s32 x, s32 y, s32 large, s32 jump);
extern void Pickup_UpdateBouncing(struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 p2, s32 p3, s32 layer);
extern void EntityUpdateNothing(void);

/**
 * @brief 53968 | Breakable object (object 0x18): hidden off screen; when hit
 * (Hit_CheckWithPlayer flags 0x1E) it breaks (Object18_SpawnDebris, sound 0xA2) and drops
 * up to eight coins (Pickup_CreateGold, Soma only; Julius gets Pickup_CreateMana), then
 * smokes for 40 frames; with unk_514 set it instead releases enemy 0x4A
 * (sEnemyData[0x4A]) every 4th frame for 32 frames.
 */
void Object18Update(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    u16 hit;
    s32 x;
    s32 y;
    s32 i;
    struct EwramData_EntityData *e;
    const struct EnemyTemplateView *info;

    hit = Hit_CheckWithPlayer(buffer, entity);
    x = (s16)entity->posX.part16.integer;
    y = (s16)entity->posY.part16.integer - 8;
    if (!Entity_IsInRect(entity, -0x20, -0x20, 0x130, 0xE0))
    {
        entity->hidden = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->hidden = 0;
        if (!(hit & 0x1E))
            break;
        if (entity->unk_514.part16.spawnParam0)
        {
            entity->state = 2;
            entity->timer = 0x20;
            Object18_SpawnDebris(entity);
            PlaySong(0xA2);
            entity->hidden = 1;
            break;
        }
        if (!gEwramData->playerStatus.currentCharacter)
        {
            for (i = 0; i <= 7; i++)
            {
                e = Pickup_CreateGold(x + RandomSigned(0x20), y - (RandomNumberGenerator() & 0xF), i / 2);
                if (e == NULL)
                    break;
                e->updateFunc = (u32 *)Pickup_UpdateBouncing;
                e->velX.whole = RandomSigned(0x100) << 10;
                e->velY.whole = RandomSigned(0x8000) - 0x20000;
                e->accelY.whole = 0x2000;
            }
        }
        else
        {
            Pickup_CreateMana(x, y, 1, 0);
        }
        Object18_SpawnDebris(entity);
        PlaySong(0xA2);
        entity->hidden = 1;
        entity->timer = 0x28;
        entity->state++;
        break;
    case 1:
        if (entity->timer > 0x1E && !(entity->timer & 3))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x100000, 0x80000, 0x100000, entity->spriteLayer - 1);
        goto dec;
    case 2:
        if (!(entity->timer & 3))
        {
            info = &sEnemyData[0x4A];
            e = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (e != NULL)
            {
                e->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x20);
                {
                    s32 dy = RandomSigned(0x10);
                    dy -= 0x18;
                    e->posY.part16.integer = dy + entity->posY.part16.integer;
                }
                e->unk_518.part8.enemyId = 0x4A;
                e->unk_518.part16.hp = info->hp;
                e->updateFunc = info->update;
                e->scrollWithBg = 1;
                info->create(e);
            }
        }
    dec:
        if (--entity->timer == 0xFF)
            entity->deleteRequested = 1;
        break;
    }
}
