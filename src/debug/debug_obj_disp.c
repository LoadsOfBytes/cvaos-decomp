#include "debug_obj_disp.h"
#include "core.h"
#include "bg_and_collision.h"
// #include "bg_effects.h" // TODO: Bg_UpdateScrollModes implicitly called with arguments (it should have none)
#include "debug_menu.h"
#include "gba.h"

#include "constants/main.h"

#include "data/interrupt_table.h"
#include "data/debug_font.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "palette.h"
#include "data/data_118B48.h"

static struct DebugObjTestState sDebugObjTestState;

struct DebugObjTestCallbacks {
    void (*updateFunc)(struct DebugObjTestState *state);
    void (*initFunc)(struct DebugObjTestState *state);
    void (*exitFunc)(struct DebugObjTestState *state);
}; /* size = 0xC */
struct DebugObjTestCallbacks sDebugObjTestCallbacks[] = {
    [0] = {
        .updateFunc = DebugObjDispTest_Update,
        .initFunc = DebugObjDispTest_Init,
        .exitFunc = DebugObjDispTest_Exit
    }
};

/**
 * @brief Run the OBJ display test through its init/update/exit callbacks;
 * Select+B exits. Return 1 when done.
 */
s32 DebugObjDispTest_Run(s32 test)
{
    s32 done;
    struct DebugObjTestCallbacks *def;

    done = 0;
    def = &sDebugObjTestCallbacks[test];

    switch (DebugTest_GetStage())
    {
        case 0:
            DMA_FILL_32(3, 0, &sDebugObjTestState, 0x8);
            BgCmdBuffer_WriteString(2, 0x13, 1, "PUSH SELECT + B TO EXIT.");
            def->initFunc(&sDebugObjTestState);
            DebugTest_SetStage(1);
            break;

        case 1:
            def->updateFunc(&sDebugObjTestState);
            Entity_UpdateAll();
            if ((gEwramData->inputData.heldInput & (4 | 2)) == (4 | 2))
            {
                DebugTest_SetStage(2);
            }
            break;

        case 2:
            def->exitFunc(&sDebugObjTestState);
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            done = 1;
            break;
    }

    return done;
}

/**
 * @brief Show the animated test sprite at (0x96, 0x50).
 */
void DebugObjDispTestSprite_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_LoadResource((u8 *)sDebugJitterSpriteGfx, 0);
            if (gfx >= 0)
            {
                ObjPalette_LoadRange((u8 *)sDebugJitterSpritePalette, 0, 1, 2);
                Sprite_Attach(entity, (void *)sDebugJitterSpriteSheet, gfx, 2);
                Anim_SelectScript(entity, 0, 1);
                entity->drawFunc = Sprite_DrawEntity;
                entity->posX.part16.integer = 0x96;
                entity->posY.part16.integer = 0x50;
                entity->scrollWithBg = 1;
                entity->state = 1;
            }
            break;

        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Empty update callback of the OBJ display test.
 */
void DebugObjDispTest_Update(struct DebugObjTestState *state)
{
    return;
}

/**
 * @brief Create the two test entities.
 */
void DebugObjDispTest_Init(struct DebugObjTestState *state)
{
    state->entities[0] = EntityCreate(DebugObjDispTestDummy_Update);
    state->entities[1] = EntityCreate(DebugObjDispTestSprite_Update);
}

/**
 * @brief Delete the two test entities.
 */
void DebugObjDispTest_Exit(struct DebugObjTestState *state)
{
    s32 i;
    struct EwramData_EntityData *entity;

    for (i = 0; i < 2; i++)
    {
        entity = state->entities[i];
        if (entity != NULL)
        {
            EntityDelete(entity);
        }
    }
}

/**
 * @brief Empty update function of the first test entity.
 */
void DebugObjDispTestDummy_Update(struct EwramData_EntityData *entity)
{
    return;
}
