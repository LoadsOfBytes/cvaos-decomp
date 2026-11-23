#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "macros.h"
struct GraphicsSet { void *graphics; void *palette; void *sprites; };
extern const struct GraphicsSet sSkillGraphicsSets[];
extern u8 sSkillStudentWitchWalkProfile[];
#define CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))

static inline u8 Enable(u8 value) { return value | 4; }
/* Student Witch soul projectile: fall, slide, wait after collision, then
 * release its active-soul slot. */
void SkillStudentWitchUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 collision;
    switch (entity->state)
    {
    case 0:
        {
            const struct GraphicsSet *sets = sSkillGraphicsSets;
            const struct GraphicsSet *set = sets + 4;
            void *graphics = sets[4].graphics;
            void *palette = set->palette;
            void *sprites = set->sprites;
            s32 animation;
            CONTEXT->redSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
            ObjPalette_LoadRange(palette, 0, 1, 2);
            Sprite_Attach(entity, sprites, CONTEXT->redSoulGfx, 2);
            animation = 0;
            if ((RandomNumberGenerator() & 7) == 0) animation = 1;
            animation = (u32)animation << 16;
            Anim_StartIndexed(entity, animation >> 16, 0, 1);
            entity->spriteLayer = 13;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            Entity_PlaceRelativeTo(entity, parent, 8, -26);
            entity->accelY.whole = 0x2000;
            entity->velY.whole = 0;
            entity->scrollWithBg = 1;
            Hit_Enable(entity, 5, NULL, 0);
            HitBox_SetAttackRaw(entity, 0x1018F0F4);
            entity->hit.flags = Enable(entity->hit.flags);
            Hit_SetTimerOverride(1, 10);
            CONTEXT->nbrActiveRedSoulEntities++;
            entity->state++;
        }
        /* fall through */
    case 1:
        collision = Entity_Walk(entity, -0x20000, sSkillStudentWitchWalkProfile, 8);
        if (collision & 8)
        {
            if (!entity->subState)
            {
                entity->subState = 1;
                PlaySong(0xD0);
            }
            if (collision & 3)
            {
                entity->state = 2;
                entity->timer = 16;
                entity->drawSkip = 1;
            }
        }
        break;
    case 2:
        if (entity->timer) entity->timer--;
        else entity->state = 3;
        break;
    case 3:
        CONTEXT->activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        CONTEXT->nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 4) || !entity->roomActive)
    {
        entity->roomActive = 1;
        entity->state = 3;
        entity->hidden = 1;
    }
    entity->roomActive = 1;
    Anim_Update(entity);
}
