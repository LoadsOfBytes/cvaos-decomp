	.include "asm/gba_constants.inc"
    .include "asm/macros.inc"

	.syntax unified

	thumb_func_start __Obj_InstallCallbacks_from_thumb
__Obj_InstallCallbacks_from_thumb: @ 0x080E032C
	bx pc
	nop

	arm_func_start __Obj_InstallCallbacks_change_to_arm
__Obj_InstallCallbacks_change_to_arm: @ 0x080E0330
	b Obj_InstallCallbacks
