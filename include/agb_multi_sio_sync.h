#ifndef AGB_MULTI_SIO_SYNC_H
#define AGB_MULTI_SIO_SYNC_H

#include "types.h"

struct MultiSioPacket {
    u8 frameCounter;
    u8 connectionMaskXor;
    u16 checksum;
    u8 payload[0x10];
};

struct MultiSioSyncState {
    u8 isParent;
    u8 stage;
    u8 connectedMask;
    u8 receivedMask;
    u8 sendReady;
    u8 receiveReady;
    u8 transfersEnabled;
    u8 errorFlags;
    u8 startupFrameCounter;
    u8 sioInterrupted;
    u8 unk_A;
    u8 counter;
    u8 pad_C[0x14 - 0xC];
    s32 sendWordIndex;
    s32 receiveWordIndex;
    // void *unk_1C;
    struct MultiSioPacket *sendBuildBuffer;
    // void *unk_20;
    struct MultiSioPacket *sendActiveBuffer;
    void *receiveWriteBuffer;
    void *receivePendingBuffer;
    void *receiveReadBuffer;
    // u32 unk_30;
    struct MultiSioPacket *unk_30;
    u8 pad_34[0x48 - 0x34];
    // u32 unk_48;
    struct MultiSioPacket *unk_48;
    u8 pad_4C[0x60 - 0x4C];
    u32 unk_60;
    u8 pad_64[0x90 - 0x64];
    u32 unk_90;
    u8 pad_94[0xC0 - 0x94];
    u32 unk_C0;
    u8 pad_C4[0xF0 - 0xC4];
};

extern struct MultiSioSyncState sMultiSioSyncState;

struct LinkFrameSlot {
    u16 markerWords[1];
    u8 payloadBytes[1];
    u8 pad_3[0x10 - 0x3];
};

struct LinkFrameState {
    struct LinkFrameSlot packetSlots[3];
    u16 sendSlotBusy[14];
    u32 sioResultFlags;
    u8 active;
    u8 pad_51[0x54 - 0x51];
};

extern struct LinkFrameState sLinkFrameState;

void MultiSioSyncInitialize(void);
s32 MultiSioSyncMain(struct LinkFrameSlot *param_0);
void MultiSioSyncSendDataSet(struct LinkFrameState *param_0);
s32 MultiSioSyncRecvDataCheck(struct LinkFrameSlot *param_0);
void MultiSioSyncVSync(void);
void MultiSioSyncInterrupt(void);
void MultiSioSyncStart(void);
void MultiSioSyncStop(void);
void Link_UpdateFrame(void);
void Link_WaitForFrame(void);
void Link_VSync(void);
s32 Link_IsConnected(void);
s32 Link_IsPeerActive(void);
void Link_Init(void);
void Link_Start(void);
void Link_Stop(void);
s32 Link_QueueSendByte(u8 param_0, s32 param_1);
s32 Link_QueueSendBytes(u8 *data, s32 count); // param_0 type?
u8 Link_GetReceivedByte(s32 param_0);
void Link_CopyReceivedBytes(u8 *param_0, s32 param_1); // param_0 type?
s16 MultiSioSyncGetId(void);
s32 Link_IsResultValid(void);
s32 Link_ReceivedFromAll(void);

#endif /* AGB_MULTI_SIO_SYNC_H */
