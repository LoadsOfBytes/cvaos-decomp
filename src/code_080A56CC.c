/* Chaos second form: spark burst (56CC). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern struct AnimScript sChaos2SparkAnimScript;
extern void Chaos2Spark_Update(struct EwramData_EntityData *);

#define OFFSET_X(e) ((e)->unk_4F8.whole)
#define OFFSET_Y(e) ((e)->unk_4FC.whole)

/* Create a spark (animation sChaos2SparkAnimScript) at a random angle on a circle
 * of radius 48 around the core, flying inwards and slowing down. */
void Chaos2_SpawnSpark(struct EwramData_EntityData *core)
{
    struct EwramData_EntityData *spark = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2Spark_Update);

    if (spark != NULL)
    {
        s32 angle, c, s, vx, vy, graphics;

        spark->posX.whole = -0x800000;
        spark->posY.whole = -0x800000;
        spark->spriteLayer = core->spriteLayer - 1;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(spark, sCommonObjectSpriteSheet, graphics, 7);
        Anim_Start(spark, &sChaos2SparkAnimScript, 0, 0);
        spark->hit.flags |= 4;
        spark->hit.flags |= 2;
        spark->drawFunc = (u32 *)Sprite_DrawEntity;
        spark->timer = 20;
        angle = (RandomNumberGenerator() % 256) << 8;
        c = Sin(angle + 0x4000);
        s = Sin(angle);
        OFFSET_X(spark) = c * 48;
        OFFSET_Y(spark) = s * 48 - 0x100000;
        vx = -(c * 20) / 16;
        spark->velX.whole = vx;
        vy = -(s * 20) / 16;
        spark->velY.whole = vy;
        spark->accelX.whole = vx / 16;
        spark->accelY.whole = vy / 16;
    }
}
