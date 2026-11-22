/* Death_UpdateFirstPhase, decompiled from the original assembly. The second player
 * read goes through the inline Player() (base kept, offset constant reloaded
 * as in the target); case 3 uses its own block variable for the radius. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void DeathPillar_Update(struct EwramData_EntityData *);
extern void DeathBlade_Update(struct EwramData_EntityData *);
extern u8 sDeathFirstPhaseAnims[];

static inline void SpawnChild(struct EwramData_EntityData *e, void *func)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)func);
    if (child != NULL)
    {
        child->unk_4F8.part8.unk_4F8 = e->unk_4F4.part8.unk_4F4;
        child->scrollWithBg = 1;
    }
}

static inline struct EwramData_EntityData *Player(void) { return gEwramData->playerState.player; }

/**
 * @brief First combat phase (1): hover near the player, command the
 * floating weapon, summon blades and optionally pillars. Enter the
 * weapon transition when flag 1 reports the floating weapon defeated.
 */
void Death_UpdateFirstPhase(struct EwramData_EntityData *e)
{
    s32 r, x, vx, vy;

    switch (e->subState)
    {
    case 0:
        e->subState = 1;
    case 1:
        vy = gEwramData->playerState.player->posY.whole - 0x300000;
        if (vy <= 0x3FFFFF)
            vy = 0x400000;
        e->posY.whole += Sin(e->timer << 7);
        {
            s32 px = Player()->posX.whole;
            r = 0x400;
            if (px < e->posX.whole - 0x400000)
                e->accelX.whole = -0x400;
            else if (px > e->posX.whole + 0x400000)
                e->accelX.whole = r;
            else
            {
                e->accelX.whole = 0;
                EntityDampVelocityX(e, 0x28);
            }
            if (vy < e->posY.whole - 0x100000)
                e->accelY.whole = -0x400;
            else if (vy > e->posY.whole + 0x100000)
                e->accelY.whole = r;
            else
            {
                e->accelY.whole = 0;
                EntityDampVelocityY(e, 0x28);
            }
        }
        EntityAccelerateAndMove(e, 0x10000);
        if ((++e->unk_4F8.part16.unk_4FA & 0xFF) != 0)
            break;
        if ((e->unk_4F8.part16.unk_4F8 & 0x40) && (RandomNumberGenerator() & 7))
        {
            e->subState = 2;
            Anim_StartIndexed(e, 3, 2, 0);
            break;
        }
        e->subState = 7;
        e->stage = 0;
        Anim_StartIndexed(e, sDeathFirstPhaseAnims[0], 2, 0);
        e->unk_4F8.part16.unk_4F8 |= 4;
        break;
    case 2:
        e->accelX.whole = 0;
        e->accelY.whole = 0;
        EntityDampVelocityX(e, 0x28);
        EntityDampVelocityY(e, 0x28);
        EntityAccelerateAndMove(e, 0x10000);
        if (!(e->animFlags & 4))
            break;
        e->unk_4F8.part16.unk_4FA = 0x40;
        PlaySong(0x111);
        e->subState = 3;
        Anim_StartIndexed(e, 4, 2, 1);
        break;
    case 3:
    {
        s16 angle = RandomNumberGenerator();
        s32 rr = RandomNumberGenerator();
        do { rr &= 0xF; } while (0);
        do { rr += 0x20; } while (0);
        vx = rr * Sin(angle + 0x4000);
        vy = rr * Sin(angle);
        x = (((s16)e->posX.part16.integer - 0xC) << 16) + vx;
        { s32 y = (((s16)e->posY.part16.integer - 0x30) << 16) + vy;
        Effect_CreateFromTable(x, y, -vx / 0xC0, -vy / 0xC0, 4, 6); }
        if (e->unk_4F8.part16.unk_4FA == 0)
        {
            e->subState = 4;
            Anim_StartIndexed(e, 5, 2, 0);
            break;
        }
        e->unk_4F8.part16.unk_4FA--;
        break;
    }
    case 4:
        if (!(e->animFlags & 4))
            break;
        Anim_StartIndexed(e, 0x12, 2, 1);
        e->unk_4F8.part16.unk_4FA = 0x40;
        e->subState = 5;
        break;
    case 5:
        if (!(e->unk_4F8.part16.unk_4FA & 0xF))
            SpawnChild(e, DeathPillar_Update);
        if (e->unk_4F8.part16.unk_4FA != 0)
        {
            e->unk_4F8.part16.unk_4FA--;
            break;
        }
        e->subState = 6;
        Anim_StartIndexed(e, 0x13, 2, 0);
        break;
    case 6:
        if (!(e->animFlags & 4))
            break;
        e->subState = 1;
        Anim_StartIndexed(e, 1, 2, 1);
        break;
    case 7:
        e->accelX.whole = 0;
        e->accelY.whole = 0;
        EntityDampVelocityX(e, 0x28);
        EntityDampVelocityY(e, 0x28);
        EntityAccelerateAndMove(e, 0x10000);
        if (!(e->animFlags & 4))
            break;
        e->stage++;
        if (sDeathFirstPhaseAnims[e->stage] == 0xFF)
        {
            e->subState = 1;
            Anim_StartIndexed(e, 1, 2, 1);
            break;
        }
        Anim_StartIndexed(e, sDeathFirstPhaseAnims[e->stage], 2, 0);
        e->unk_4F8.part16.unk_4F8 &= ~4;
        break;
    }
    {
        s32 t = e->timer + 1;
        e->timer = t;
        if (!(t & (gEwramData->gameState.difficultyMode == 0 ? 0x7F : 0x3F)))
            SpawnChild(e, DeathBlade_Update);
    }
    if (e->unk_4F8.part16.unk_4F8 & 1)
    {
        e->state = 2;
        e->stage = 0;
        e->subState = 0;
    }
}
