MEMORY {
    ewram (rwx): ORIGIN = 0x02000000 LENGTH = 256K
    iwram (rwx): ORIGIN = 0x03000000 LENGTH = 32K
    rom    (rx): ORIGIN = 0x08000000 LENGTH = 32M
}

gNumMusicPlayers = 8;
gMaxLines = 0;

SECTIONS {
    ewram (NOLOAD) : ALIGN(4) {
    } >ewram

    iwram (NOLOAD) : ALIGN(4) {
        src/debug/debug_menu.o(.bss);
        src/debug/debug_tests.o(.bss);
        src/debug/debug_sio.o(.bss);
        src/debug/debug_obj_disp.o(.bss);

        . = 0x00000058; gObjEntries = .;
        . = 0x00000C58; gOamBuffer = .;
        . = 0x00001458; gObjAffine = .;
        . = 0x00001958; gObjLayers = .;
        . = 0x000019D8; gObjState = .;
        . = 0x00001A18; gTextGlyphRows = .;
        . = 0x00001A48; gPlayerAttackRect = .;
        . = 0x00001A50; gPlayerHurtRect = .;
        . = 0x00001A58; gEntityBoxes = .;

        . = 0x000020D8; src/engine/sound.o(.bss);

        src/agb_sram.o(.bss);

        asm/sound_data.o(.bss);

        *libgcc.a:*.o(.bss);
        *libc.a:*.o(.bss);

        . = 0x000028D8; SoundMainRAM_Buffer = .;

        . = 0x00002C60; gDisplayRegisters = .;
        . = 0x00002CB0; gUnk_03002CB0 = .;

        . = 0x00003CD0; gIntrMainBuffer = .;

        . = 0x00004560; gSpriteDrawAffineFunc = .;
        . = 0x00004564; gSpriteDrawFunc = .;
        . = 0x00004570; gSpriteDrawArmCode = .;
        . = 0x00004970; gSpriteDrawAffineArmCode = .;

        . = 0x00004F70; gObjBuildOamArm = .;

        . = 0x00005170; gObjAllocManyFunc = .;
        . = 0x00005174; gObjAddToLayerFunc = .;
        . = 0x00005178; gObjAffineGetFlipYFunc = .;
        . = 0x0000517C; gObjTilesGetTileFunc = .;
        . = 0x00005180; gObjAffineGetFlipXFunc = .;
        . = 0x00005184; gMatrixMultiplyVec2Func = .;
        . = 0x00005188; gObjAffineGetMatrixFunc = .;
        . = 0x00005190; sMultiSioSyncState = .;
        . = 0x00005280; sLinkFrameState = .;

        . = 0x000052D4; gReadSramFast = .;
        . = 0x000052D8; gVerifySramFast = .;

        . = 0x000052E0; gMPlayInfo_5 = .;
        . = 0x00005320; gMPlayInfo_6 = .;
        . = 0x00005360; gMPlayInfo_0 = .;
        . = 0x000053A0; gMPlayInfo_7 = .;
        . = 0x000053E0; gMPlayInfo_1 = .;
        . = 0x00005420; gMPlayInfo_2 = .;
        . = 0x00005460; gMPlayInfo_4 = .;

        . = 0x000054A0; gMPlayMemAccArea = .;

        . = 0x000054B0; gMPlayInfo_3 = .;

        . = 0x000054F0; gSoundInfo = .;
        . = 0x00005E70; gMPlayJumpTable = .;
        . = 0x00005F00; gCgbChans = .;

        . = 0x00006000; *libc.a:*.o(COMMON);
        end = .;
    } >iwram

    .text : ALIGN(2) {
        asm/rom_header.o(.text);
        asm/crt0.o(.text);
        src/main.o(.text);
        src/engine/core.o(.text);
        src/engine/sram.o(.text);
        src/engine/bg_and_collision.o(.text);
        src/mode/game_modes.o(.text);
        src/menu/main_menu.o(.text);
        src/mode/intro_cutscene.o(.text);
        src/menu/soul_trade.o(.text);
        src/mode/in_game.o(.text);
        src/engine/bg_effects.o(.text);
        src/engine/room_entities.o(.text);
        src/menu/map_screen.o(.text);
        src/engine/map_flags.o(.text);
        src/engine/save_data.o(.text);
        src/engine/music.o(.text);
        src/player/player_soma.o(.text);
        src/player/player_julius.o(.text);
        src/player/player_core.o(.text);
        src/soul/red_souls_a.o(.text);
        src/soul/red_souls_b.o(.text);
        src/soul/blue_souls.o(.text);
        src/player/soul_inventory.o(.text);
        src/player/player_effects.o(.text);
        src/player/player_level.o(.text);
        src/debug/debug_1967.o(.text);
        src/debug/debug_2229.o(.text);
        src/debug/debug_menu.o(.text);
        src/debug/debug_tests.o(.text);
        src/debug/debug_sio.o(.text);
        src/debug/debug_obj_disp.o(.text);
        src/debug/debug_4531.o(.text);
        src/engine/obj_gfx.o(.text);
        src/engine/palette_state.o(.text);
        src/engine/palette.o(.text);
        src/engine/oam.o(.text);
        src/engine/math_anim.o(.text);
        src/engine/text12.o(.text);
        src/engine/hitbox.o(.text);
        src/engine/hit_shared_history.o(.text);
        src/engine/damage_numbers.o(.text);
        src/engine/sprite_draw_arm.o(.text);
        src/engine/sprite_affine_arm.o(.text);
        src/engine/obj_callbacks_arm.o(.text);
        src/engine/oam_build_arm.o(.text);
        src/engine/pickup.o(.text);
        src/engine/soul_pickup.o(.text);
        src/engine/effects.o(.text);
        src/menu/menu_common.o(.text);
        src/menu/pause_menu.o(.text);
        src/menu/soul_menu.o(.text);
        src/menu/equipment_menu.o(.text);
        src/menu/equipment_list_update.o(.text);
        src/menu/item_menu.o(.text);
        src/menu/system_menus.o(.text);
        src/menu/bestiary_cursor.o(.text);
        src/menu/bestiary.o(.text);
        src/object/objects_00_07.o(.text);
        src/object/objects_08_0f.o(.text);
        src/object/objects_10_13.o(.text);
        src/object/object_14.o(.text);
        src/object/objects_15_18.o(.text);
        src/object/object_19_create.o(.text);
        src/object/objects_19_1c.o(.text);
        src/object/save_point.o(.text);
        src/object/warp_room.o(.text);
        src/object/objects_22_2a.o(.text);
        src/object/objects_23_34.o(.text);
        src/object/objects_35_37.o(.text);
        src/object/shop_update.o(.text);
        src/event/scenes_a.o(.text);
        src/event/scene03.o(.text);
        src/event/scenes_b.o(.text);
        src/event/scene30.o(.text);
        src/event/scene30_transformation.o(.text);
        src/event/scenes_c.o(.text);
        src/menu/shop.o(.text);
        src/menu/text_window.o(.text);
        src/engine/obj_palette.o(.text);
        src/engine/entity_motion.o(.text);
        src/engine/box_collision.o(.text);
        src/engine/enemy_common.o(.text);
        src/engine/entity_common.o(.text);
        src/enemy/valkyrie.o(.text);
        src/enemy/werewolf.o(.text);
        src/enemy/killer_fish.o(.text);
        src/enemy/merman.o(.text);
        src/enemy/mudman.o(.text);
        src/enemy/manticore.o(.text);
        src/enemy/une.o(.text);
        src/enemy/winged_skeleton.o(.text);
        src/enemy/harpy.o(.text);
        src/enemy/white_dragon.o(.text);
        src/enemy/quezlcoatl.o(.text);
        src/enemy/werejaguar.o(.text);
        src/enemy/arachne.o(.text);
        src/enemy/zombie.o(.text);
        src/enemy/persephone.o(.text);
        src/enemy/alura_une.o(.text);
        src/enemy/giant_ghost.o(.text);
        src/enemy/golem.o(.text);
        src/enemy/headhunter.o(.text);
        src/enemy/gargoyle.o(.text);
        src/enemy/triton.o(.text);
        src/enemy/peeping_eye.o(.text);
        src/enemy/gorgon.o(.text);
        src/enemy/ghost.o(.text);
        src/enemy/great_armor.o(.text);
        src/enemy/skeleton_knight.o(.text);
        src/enemy/dead_crusader.o(.text);
        src/enemy/devil.o(.text);
        src/enemy/zombie_officer.o(.text);
        src/enemy/zombie_soldier.o(.text);
        src/enemy/curly.o(.text);
        src/enemy/medusa_head.o(.text);
        src/enemy/succubus.o(.text);
        src/enemy/lilith.o(.text);
        src/enemy/creaking_skull.o(.text);
        src/enemy/mimic.o(.text);
        src/enemy/ectoplasm.o(.text);
        src/enemy/biphron.o(.text);
        src/enemy/stolas.o(.text);
        src/enemy/legion.o(.text);
        src/enemy/kyoma_demon.o(.text);
        src/enemy/kyoma_demon_b.o(.text);
        src/enemy/gladiator.o(.text);
        src/enemy/chaos.o(.text);
        src/enemy/erinys.o(.text);
        src/enemy/tsuchinoko.o(.text);
        src/enemy/sky_fish.o(.text);
        src/enemy/big_golem.o(.text);
        src/enemy/bat.o(.text);
        src/enemy/worm.o(.text);
        src/enemy/dryad.o(.text);
        src/enemy/waiter_skeleton.o(.text);
        src/enemy/skull_archer.o(.text);
        src/enemy/minotaur.o(.text);
        src/enemy/axe_armor.o(.text);
        src/enemy/disc_armor.o(.text);
        src/enemy/bomber_rock_armor.o(.text);
        src/enemy/head_stack.o(.text);
        src/enemy/balore.o(.text);
        src/enemy/chronomage.o(.text);
        src/enemy/dead_warrior.o(.text);
        src/enemy/skull_millione.o(.text);
        src/enemy/nightmare.o(.text);
        src/enemy/student_witch.o(.text);
        src/enemy/witch.o(.text);
        src/enemy/man_eater.o(.text);
        src/enemy/killer_mantle.o(.text);
        src/enemy/death.o(.text);
        src/enemy/evil_butcher.o(.text);
        src/enemy/kicker_skeleton.o(.text);
        src/enemy/slime.o(.text);
        src/enemy/altair.o(.text);
        src/enemy/fleaman.o(.text);
        src/enemy/ripper.o(.text);
        src/enemy/ghost_dancer.o(.text);
        src/enemy/skeleton.o(.text);
        src/enemy/imp.o(.text);
        src/enemy/ukoback.o(.text);
        src/enemy/nemesis.o(.text);
        src/enemy/killer_doll.o(.text);
        src/enemy/lightning_doll.o(.text);
        src/enemy/crow.o(.text);
        src/enemy/beam_skeleton.o(.text);
        src/enemy/buer.o(.text);
        src/enemy/bael.o(.text);
        src/enemy/alastor.o(.text);
        src/enemy/mandragora.o(.text);
        src/enemy/belmont.o(.text);
        src/enemy/graham.o(.text);
        src/enemy/boss_defeat.o(.text);
        src/engine/sound.o(.text);
        src/agb_multi_sio_sync.o(.text);
        asm/libagbsyscall.o(.text);
        src/agb_sram.o(.text);
        src/agb_print.o(.text);
        asm/m4a0.o(.text);
        src/m4a.o(.text);
        *libgcc.a:_call_via_rX.o(.text);
        *libgcc.a:_divsi3.o(.text);
        *libgcc.a:_dvmd_tls.o(.text);
        *libgcc.a:_fixunsdfsi.o(.text);
        *libgcc.a:_modsi3.o(.text);
        *libgcc.a:_udivsi3.o(.text);
        *libgcc.a:_umodsi3.o(.text);
        *libgcc.a:dp-bit.o(.text);
        *libgcc.a:fp-bit.o(.text);
        *libgcc.a:_lshrdi3.o(.text);
        *libgcc.a:_muldi3.o(.text);
        *libgcc.a:_negdi2.o(.text);

        *libc.a:memcpy.o(.text);
        *libc.a:memset.o(.text);
        *libc.a:sprintf.o(.text);
        *libc.a:vfprintf.o(.text);
        *libc.a:vsprintf.o(.text);
        *libc.a:wsetup.o(.text);
        *libc.a:dtoa.o(.text);
        *libc.a:fflush.o(.text);
        *libc.a:findfp.o(.text);
        *libc.a:freer.o(.text);
        *libc.a:fvwrite.o(.text);
        *libc.a:fwalk.o(.text);
        *libc.a:locale.o(.text);
        *libc.a:makebuf.o(.text);
        *libc.a:mallocr.o(.text);
        *libc.a:mbtowc_r.o(.text);
        *libc.a:memchr.o(.text);
        *libc.a:memmove.o(.text);
        *libc.a:mlock.o(.text);
        *libc.a:mprec.o(.text);
        *libc.a:s_isinf.o(.text);
        *libc.a:s_isnan.o(.text);
        *libc.a:sbrkr.o(.text);
        *libc.a:stdio.o(.text);
        *libc.a:strcmp.o(.text);
        *libc.a:strlen.o(.text);
        *libc.a:syscalls.o(.text);
        *libc.a:writer.o(.text);
        *libc.a:callocr.o(.text);
        *libc.a:closer.o(.text);
        *libc.a:errno.o(.text);
        *libc.a:fstatr.o(.text);
        *libc.a:libcfunc.o(.text);
        *libc.a:lseekr.o(.text);
        *libc.a:readr.o(.text);
        asm/arm_callbacks.o(.text);
    } >rom

    .rodata : ALIGN(4) {
        src/data/data_0E0334.o(.rodata);
        src/engine/core.o(.rodata);
        src/engine/sram.o(.rodata);
        src/engine/bg_and_collision.o(.rodata);
        src/mode/game_modes.o(.rodata);
        src/menu/main_menu.o(.rodata);
        src/mode/intro_cutscene.o(.rodata);
        src/menu/soul_trade.o(.rodata);
        src/mode/in_game.o(.rodata);
        src/engine/bg_effects.o(.rodata);
        src/engine/room_entities.o(.rodata);
        src/menu/map_screen.o(.rodata);
        src/engine/map_flags.o(.rodata);
        src/engine/save_data.o(.rodata);
        src/engine/music.o(.rodata);
        asm/data/data_0E0B68.o(.rodata);
        src/player/soul_inventory.o(.rodata);
        src/player/player_effects.o(.rodata);
        src/player/player_level.o(.rodata);
        src/debug/debug_1967.o(.rodata);
        src/debug/debug_menu.o(.rodata);
        src/debug/debug_tests.o(.rodata);
        src/debug/debug_sio.o(.rodata);
        src/debug/debug_obj_disp.o(.rodata);
        src/debug/debug_4531.o(.rodata);
        asm/data/data_0E1CD0.o(.rodata);
        src/data/data_0E3464.o(.rodata);
        asm/data/data_0E5364.o(.rodata);
        src/event/scene30_transformation.o(.rodata);
        asm/data/data_118B48.o(.rodata);
        src/agb_multi_sio_sync.o(.rodata);
        src/agb_sram.o(.rodata);
        src/agb_print.o(.rodata);
        asm/data/data_27B2C0.o(.rodata);
        asm/sound_data.o(.rodata);
        src/data/m4a_tables.o(.rodata);
        asm/data/data_27DF8C.o(.rodata);
        *libc.a:memcpy.o(.rodata);
        *libc.a:memset.o(.rodata);
        *libc.a:sprintf.o(.rodata);
        *libc.a:vfprintf.o(.rodata);
        *libc.a:vsprintf.o(.rodata);
        *libc.a:wsetup.o(.rodata);
        *libc.a:dtoa.o(.rodata);
        *libc.a:fflush.o(.rodata);
        *libc.a:findfp.o(.rodata);
        *libc.a:freer.o(.rodata);
        *libc.a:fvwrite.o(.rodata);
        *libc.a:fwalk.o(.rodata);
        *libc.a:impure.o(.rodata);
        *libc.a:locale.o(.rodata);
        *libc.a:makebuf.o(.rodata);
        *libc.a:mallocr.o(.rodata);
        *libc.a:mbtowc_r.o(.rodata);
        *libc.a:memchr.o(.rodata);
        *libc.a:memmove.o(.rodata);
        *libc.a:mlock.o(.rodata);
        *libc.a:mprec.o(.rodata);
        *libc.a:s_isinf.o(.rodata);
        *libc.a:s_isnan.o(.rodata);
        *libc.a:sbrkr.o(.rodata);
        *libc.a:stdio.o(.rodata);
        *libc.a:strcmp.o(.rodata);
        *libc.a:strlen.o(.rodata);
        *libc.a:syscalls.o(.rodata);
        *libc.a:writer.o(.rodata);
        *libc.a:callocr.o(.rodata);
        *libc.a:closer.o(.rodata);
        *libc.a:errno.o(.rodata);
        *libc.a:fstatr.o(.rodata);
        *libc.a:libcfunc.o(.rodata);
        *libc.a:lseekr.o(.rodata);
        *libc.a:readr.o(.rodata);
    } >rom

    .data : ALIGN(4) {
        src/ewram.o(.data);
        src/engine/core.o(.data);
        src/engine/sram.o(.data);
        src/engine/bg_and_collision.o(.data);
        src/mode/game_modes.o(.data);
        src/menu/main_menu.o(.data);
        src/mode/intro_cutscene.o(.data);
        src/menu/soul_trade.o(.data);
        src/mode/in_game.o(.data);
        src/engine/bg_effects.o(.data);
        src/engine/room_entities.o(.data);
        src/menu/map_screen.o(.data);
        src/engine/map_flags.o(.data);
        src/engine/save_data.o(.data);
        src/engine/music.o(.data);
        asm/data/data_4F0B18.o(.data);
        src/debug/debug_1967.o(.data);
        src/debug/debug_menu.o(.data);
        src/debug/debug_tests.o(.data);
        src/debug/debug_sio.o(.data);
        src/debug/debug_obj_disp.o(.data);
        src/debug/debug_4531.o(.data);
        asm/data/data_4F1444.o(.data);
        src/engine/sound.o(.data);
        *libc.a:memcpy.o(.data);
        *libc.a:memset.o(.data);
        *libc.a:sprintf.o(.data);
        *libc.a:vfprintf.o(.data);
        *libc.a:vsprintf.o(.data);
        *libc.a:wsetup.o(.data);
        *libc.a:dtoa.o(.data);
        *libc.a:fflush.o(.data);
        *libc.a:findfp.o(.data);
        *libc.a:freer.o(.data);
        *libc.a:fvwrite.o(.data);
        *libc.a:fwalk.o(.data);
        *libc.a:impure.o(.data);
        *libc.a:locale.o(.data);
        *libc.a:makebuf.o(.data);
        *libc.a:mallocr.o(.data);
        *libc.a:mbtowc_r.o(.data);
        *libc.a:memchr.o(.data);
        *libc.a:memmove.o(.data);
        *libc.a:mlock.o(.data);
        *libc.a:mprec.o(.data);
        *libc.a:s_isinf.o(.data);
        *libc.a:s_isnan.o(.data);
        *libc.a:sbrkr.o(.data);
        *libc.a:stdio.o(.data);
        *libc.a:strcmp.o(.data);
        *libc.a:strlen.o(.data);
        *libc.a:syscalls.o(.data);
        *libc.a:writer.o(.data);
        *libc.a:callocr.o(.data);
        *libc.a:closer.o(.data);
        *libc.a:errno.o(.data);
        *libc.a:fstatr.o(.data);
        *libc.a:libcfunc.o(.data);
        *libc.a:lseekr.o(.data);
        *libc.a:readr.o(.data);
    } >rom

    /* DWARF debug sections.
       Symbols in the DWARF debugging sections are relative to the beginning
       of the section so we begin them at 0.  */

    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }

    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }

    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }

    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }

    /* Discard everything not specifically mentioned above. */
    /DISCARD/ :
    {
        *(*);
    }
}
