/* Scripted movement along a step table. */
#include "code/code_08039340.h"
#include "nemesis_entity.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/* One movement step of sNemesisScriptedAttackSteps; dx == 0x7FFF ends the table. */
struct Unk_C70F0_Step {
    s16 dx;       // pixels over the whole step, mirrored with flag bit 1
    s16 dy;
    s16 rotation; // total affine angle change
    u16 frames;
};

extern struct Unk_C70F0_Step sNemesisScriptedAttackSteps[];
extern void Nemesis_UpdateRotatedBoxes(struct EwramData_EntityData *);

#define FLAGS(e) (NEMESIS_FLAGS((e)))
#define STEP(e) ((e)->unk_4F4.part8.unk_4F7)

/**
 * @brief Distribute each movement-table step over its frame count, update rotation and boxes, and return to positioning after the terminator and pause.
 */
void Nemesis_UpdateScriptedAttack(struct EwramData_EntityData *entity)
{
    struct Unk_C70F0_Step *table = sNemesisScriptedAttackSteps;
    struct Unk_C70F0_Step *step = &table[STEP(entity)];
    u8 frames;
    s32 mirrored;
    s32 dx;

    Nemesis_UpdateRotatedBoxes(entity);
    switch (entity->subState)
    {
    case 0:
        STEP(entity) = 0;
        step = table;
        PlaySong(0x85);
        entity->subState++;
    case 1:
        frames = entity->timer = step->frames;
        dx = step->dx;
        mirrored = FLAGS(entity) & 2;
        if (mirrored)
            dx = -dx;
        dx <<= 16;
        entity->velX.whole = dx / frames;
        entity->velY.whole = (step->dy << 16) / frames;
        NEMESIS_TURN(entity) = step->rotation / frames;
        if (!mirrored)
            NEMESIS_TURN(entity) = -NEMESIS_TURN(entity);
        entity->subState++;
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        NEMESIS_ANGLE(entity) += NEMESIS_TURN(entity);
        ObjAffine_Set(NEMESIS_ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
        if (--entity->timer == 0)
        {
            STEP(entity)++;
            if (table[STEP(entity)].dx == 0x7FFF)
                entity->subState = 5;
            else
                entity->subState = 1;
            entity->timer = 0;
            PlaySong(0x85);
        }
        break;
    case 5:
        if (entity->timer++ > 0x28)
        {
            entity->timer = 0;
            if (entity->state != 1)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            STEP(entity) = 0;
        }
        break;
    }
}
