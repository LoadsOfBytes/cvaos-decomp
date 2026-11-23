/*
 * Damage numbers and number watches (player / entity damage indicators, HUD HP number).
 */
#include "damage_numbers.h"
#include "types.h"
#include "structs/ewram.h"
#include "structs/entity_boxes.h"
#include "core.h"
#include "obj_gfx.h"

/* Small accessors (2A54, 2A6C). */
/* Set byte index of the table at EWRAM 0x1307C. */
void Hit_SetTimerOverride(s32 index, s32 value)
{
    gEwramData->hitTimerOverride[index] = value;
}

/* Box record of the entity's slot (boxSlot). */
/**
 * @brief Return the EntityBoxes record indexed by the entity hit boxSlot, retaining unchecked indexing.
 */
struct EntityBoxes *Hit_GetEntityBoxes(struct EwramData_EntityData *entity)
{
    return &gEntityBoxes[entity->hit.boxSlot];
}

/* Damage/status indicator; palette and vertical position depend on kind. */
extern s32 Player_GetOverheadY(void);
extern void PlayerDamageIndicator_Update(struct EwramData_EntityData *);
extern void NumberIndicator_Format(struct EwramData_EntityData *, s32);
extern void NumberIndicator_Draw(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectGfxLanguage2[], sCommonObjectGfxLanguage3[], sCommonObjectSpriteSheet[];

/**
 * @brief Create the localized player damage/healing indicator with kind-dependent position and palette; value zero in kind 0 selects frame 0x58.
 */
void Player_ShowDamage(s32 value, u8 kind)
{
    s32 useFrame = 0;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)PlayerDamageIndicator_Update);
    s32 graphics, palette;
    entity->state = kind;
    switch (gEwramData->gameState.language)
    {
    case 2: graphics = ObjGfx_LoadResource(sCommonObjectGfxLanguage2, 0); break;
    case 3: graphics = ObjGfx_LoadResource(sCommonObjectGfxLanguage3, 0); break;
    default: graphics = ObjGfx_LoadResource(sCommonObjectGfx, 0); break;
    }
    entity->posX.whole = gEwramData->playerState.cachedPlayerX;
    entity->posY.part16.integer = Player_GetOverheadY();
    switch (entity->state)
    {
    case 0:
        palette = 0;
        entity->posY.whole = gEwramData->playerState.player->posY.whole - 0x1A0000;
        if (value == 0) { useFrame = 1; palette = 6; }
        break;
    case 1:
        palette = 7;
        break;
    case 2:
        palette = 8;
        break;
    default: palette = 9; break;
    }
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, palette);
    if (useFrame == 0)
    {
        NumberIndicator_Format(entity, value);
        entity->drawFunc = (u32 *)NumberIndicator_Draw;
    }
    else
    {
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->animFrame = 0x58;
    }
    entity->spriteLayer = 2;
    entity->scrollWithBg = 1;
}

/* Damage/status indicator; palette and vertical position depend on kind. */
extern void EntityDamageIndicator_Update(struct EwramData_EntityData *);

void Entity_ShowDamage(struct EwramData_EntityData *owner, s32 value, u8 kind)
{
    s32 useFrame = 0;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)EntityDamageIndicator_Update);
    u8 *sprite;
    s32 graphics, palette;
    entity->state = kind;
    switch (gEwramData->gameState.language)
    {
    case 2: sprite = sCommonObjectGfxLanguage2; break;
    case 3: sprite = sCommonObjectGfxLanguage3; break;
    default: sprite = sCommonObjectGfx; break;
    }
    graphics = ObjGfx_LoadResource(sprite, 0);
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    switch (entity->state)
    {
    case 0:
        palette = 6;
        if (value == 0) useFrame = 1;
        break;
    case 1:
        palette = 7;
        entity->posY.whole -= 0x80000;
        break;
    case 2:
        palette = 9;
        entity->posY.whole -= 0x80000;
        break;
    default: palette = 9; break;
    }
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, palette);
    if (useFrame == 0)
    {
        NumberIndicator_Format(entity, value);
        entity->drawFunc = (u32 *)NumberIndicator_Draw;
    }
    else
    {
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->animFrame = 0x58;
    }
    entity->spriteLayer = 2;
    entity->scrollWithBg = 1;
}

/* Format a signed value in eleven digit slots, with leading blanks.
 * Frame 10 is minus, 11 is the kind-1 prefix, and 12 is blank. */

/* Preserve the lane, base and group temporaries: folding the fixed offsets
 * into one displacement changes agbcc's addressing and register allocation. */
static inline u8 *NumberSlot(struct EwramData_EntityData *entity, u8 i)
{
    s32 lane = i & 3;
    u8 *base = (u8 *)&entity->ext.raw.unk_4F4;
    s32 group = (i >> 2) + 2;
    s32 offset = lane + group * 4;
    return base + offset;
}
#define DIGIT(e,i) (*NumberSlot(e, i))
/**
 * @brief Format a signed value in eleven packed glyph slots with leading blanks and optional minus/kind-1 prefix, preserving the original zero and minimum-digit behavior.
 */
void NumberIndicator_Format(struct EwramData_EntityData *entity, s32 value)
{
    s32 started = 0;
    s32 divisor = 1000000000;
    u8 i;
    if (value == 0)
    {
        for (i = 0; i <= 10; i++) DIGIT(entity,i) = 12;
        entity->ext.numberIndicator.firstDigit = 0;
    }
    else
    {
        if (value < 0)
        {
            entity->ext.numberIndicator.prefix = 10;
            value = -value;
        }
        else
        {
            if (entity->state == 1) entity->ext.numberIndicator.prefix = 11;
            else entity->ext.numberIndicator.prefix = 12;
        }
        for (i = 1; i <= 10; i++)
        {
            u8 digit;
            DIGIT(entity,i) = 12;
            if (10 - entity->ext.numberIndicator.padDigits == i) started = 1;
            for (digit = 0; digit <= 9; digit++)
            {
                if (value < divisor)
                {
                    if (digit != 0 && started == 0) started = 1;
                    if (started != 0) DIGIT(entity,i) = digit;
                    break;
                }
                value -= divisor;
            }
            divisor /= 10;
        }
    }
}

/* Number indicator movement: kind zero follows the player with a wobble;
 * the other kinds rise. All are removed after forty updates. */
/**
 * @brief Kind 0 follows the player with vertical wobble; kinds 1 and 2 rise. Request deletion when the timer reaches 40.
 */
void PlayerDamageIndicator_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        s32 wobble = ((40 - entity->timer) >> 4) << 16;
        struct EwramData_EntityData **player;
        s32 velocity;
        s32 y;
        if (entity->timer & 2) wobble = -wobble;
        velocity = entity->velY.whole;
        entity->velY.whole = velocity - 0x4000;
        player = &gEwramData->playerState.player;
        y = (*player)->posY.whole;
        velocity -= 0x1A4000;
        entity->posY.whole = y + velocity + wobble;
        entity->posX.whole = (*player)->posX.whole;
        break;
    }
    case 1: entity->posY.whole -= 0x8000; break;
    case 2: entity->posY.whole -= 0x8000; break;
    }
    if (++entity->timer == 40) entity->deleteRequested = 1;
}

/**
 * @brief Rise and blink near expiry; kind 0 deletes at tick 28 and kinds 1/2 at tick 40.
 */
void EntityDamageIndicator_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->posY.whole -= 0x8000;
        if (entity->timer > 20)
        {
            if (entity->timer % 2) entity->hidden = 1;
            else entity->hidden = 0;
        }
        if (++entity->timer == 28)
            entity->deleteRequested = 1;
        break;
    case 1:
    case 2:
        entity->posY.whole -= 0x8000;
        if (entity->timer > 30)
        {
            if (entity->timer % 2) entity->hidden = 1;
            else entity->hidden = 0;
        }
        if (++entity->timer == 40)
            entity->deleteRequested = 1;
        break;
    }
}

/* Center and draw eleven entries, skipping value 12.
 * Retain the signed remainder/row arithmetic and short register lifetimes:
 * they reproduce agbcc's allocation and address calculation. The output
 * pointer is formed before the first loop for the same reason. A match
 * does not establish the original source's spelling or organization. */
/**
 * @brief When flag 0x20 is clear, center and draw nonblank entries from eleven packed glyph slots at five-pixel spacing, then restore X.
 */
void NumberIndicator_DrawCentered(struct EwramData_EntityData *entity)
{
    s32 savedX, i;

    if (!(gEwramData->gameState.stateFlags & 0x20))
    {
        u8 *entries;
        u8 *output;
        savedX = entity->posX.whole;
        i = 0;
        entries = (u8 *)&entity->ext.raw.unk_4F4;
        output = &entity->animFrame;
        {
            u8 *base = entries;
            s32 delta = -0x28000;
            for (; i < 11; i++)
            {
                register s32 remainder asm("r0") = i % 4; /* FAKEMATCH: pinned register */
                s32 row = (i >> 2) + 2;
                remainder += row * 4;
                if (*(base + remainder) != 12)
                    entity->posX.whole += delta;
            }
        }
        for (i = 0; i < 11; i++)
        {
            register s32 remainder asm("r1") = i % 4; /* FAKEMATCH: pinned register */
            s32 row = (i >> 2) + 2;
            u8 *entry = &entries[remainder + row * 4];
            *output = *entry;
            if (*entry != 12)
            {
                Sprite_Draw(entity);
                entity->posX.whole += 0x50000;
            }
        }
        entity->posX.whole = savedX;
    }
}

/* Create a debug-style value display at (x, y): graphics 081C15F4 /
 * 0820ED60 with the given palette, watching the 8-, 16- or 32-bit value at
 * value (bits) through NumberWatch_Update and showing it via NumberIndicator_Format. */

/*
 * Number watch (NumberWatch_Create; same aliases as NumberWatch_Update):
 * width of the watched value in bits (8 / 16 / 32), its address and the last
 * shown value. Token-identical expansion.
 */
#define NUMBER_WATCH_LAST(e) ENTITY_WORD_510(e)
extern void NumberWatch_Update(struct EwramData_EntityData *);

/**
 * @brief Create a numeric display at fixed-point x/y watching a signed 8-, 16- or default 32-bit memory value with the supplied palette.
 */
struct EwramData_EntityData *NumberWatch_Create(void *value, s32 x, s32 y, u8 palette, u8 bits)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)NumberWatch_Update);
    s32 graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, (s8)palette);
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->ext.numberIndicator.watchBits = bits;
    if ((s8)bits == 8)
        NUMBER_WATCH_LAST(entity) = *(s8 *)value;
    else if ((s8)bits == 16)
        NUMBER_WATCH_LAST(entity) = *(s16 *)value;
    else
        NUMBER_WATCH_LAST(entity) = *(s32 *)value;
    entity->ext.numberIndicator.watchAddress = (s32)value;
    NumberIndicator_Format(entity, NUMBER_WATCH_LAST(entity));
    entity->drawFunc = (u32 *)NumberIndicator_Draw;
    entity->spriteLayer = 2;
    return entity;
}

/*
 * Number watch (NumberWatch_Update): width of the watched value in bits
 * (8 / 16 / 32), its address and the last shown value. Token-identical
 * expansion.
 */

/**
 * @brief Read the configured signed memory value and reformat only when it differs from the cached value.
 */
void NumberWatch_Update(struct EwramData_EntityData *entity)
{
    s32 value;
    switch (entity->ext.numberIndicator.watchBits)
    {
    case 8: value = *(s8 *)entity->ext.numberIndicator.watchAddress; break;
    case 16: value = *(s16 *)entity->ext.numberIndicator.watchAddress; break;
    default: value = *(s32 *)entity->ext.numberIndicator.watchAddress; break;
    }
    if (value != NUMBER_WATCH_LAST(entity))
    {
        NumberIndicator_Format(entity, value);
        NUMBER_WATCH_LAST(entity) = value;
    }
}

/* Draw eleven packed glyph entries, skipping empty entry 0x0C. */
static inline u8 *Glyph(u8 *base, s32 index) { return base + index; }
/**
 * @brief Draw nonblank entries from eleven packed glyph slots at five-pixel spacing, then restore X.
 */
void NumberIndicator_Draw(struct EwramData_EntityData *entity)
{
    s32 savedX = entity->posX.whole;
    s32 i;
    u8 *glyph;
    for (i = 0; i <= 10; i++)
    {
        s32 index = i;
        index %= 4;
        index += ((i >> 2) + 2) * 4;
        glyph = Glyph((u8 *)&entity->ext.raw.unk_4F4, index);
        entity->animFrame = *glyph;
        if (*glyph != 0xC)
        {
            Sprite_Draw(entity);
            entity->posX.whole += 0x50000;
        }
    }
    entity->posX.whole = savedX;
}

/* HUD HP number: the smoothed HUD HP value last shown. Token-identical. */
#define HUD_HP_SHOWN(e) ENTITY_WORD_510(e)
void NumberIndicator_DrawCentered(struct EwramData_EntityData *entity);

/**
 * @brief Set the existing entity flag and reformat the number when the smoothed HUD HP value changes.
 */
void HudHPNumber_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram;
    entity->roomActive = 1;
    ewram = &gEwramData;
    if ((*ewram)->gameState.hudHP != HUD_HP_SHOWN(entity))
    {
        NumberIndicator_Format(entity, (*ewram)->gameState.hudHP);
        HUD_HP_SHOWN(entity) = (*ewram)->gameState.hudHP;
    }
}

/**
 * @brief Create the HUD HP number in slots 0x16..0x19 at pixel position 18,10 using the centered draw callback and cached smoothed HP.
 */
void HudHPNumber_Create(void)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x16, 0x19, (u32 *)HudHPNumber_Update);
    s32 gfx;
    s32 value;

    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
    value = gEwramData->gameState.hudHP;
    HUD_HP_SHOWN(entity) = value;
    NumberIndicator_Format(entity, value);
    entity->drawFunc = (u32 *)NumberIndicator_DrawCentered;
    entity->spriteLayer = 0;
    entity->posX.part16.integer = 0x12;
    entity->posY.part16.integer = 0xA;
}
