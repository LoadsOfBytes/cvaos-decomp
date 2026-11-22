#include "agb_multi_sio_sync.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

#define SIO_RESULT_SHIFT_UNK_3 0
#define SIO_RESULT_SHIFT_IS_PARENT 7
#define SIO_RESULT_SHIFT_UNK_2 8
#define SIO_RESULT_SHIFT_ERROR 12
#define SIO_RESULT_SHIFT_INVALID_ID 13
#define SIO_RESULT_SHIFT_UNK_8 15

#define SIO_RESULT_UNK_3 (0x7F << SIO_RESULT_SHIFT_UNK_3)
#define SIO_RESULT_IS_PARENT (1 << SIO_RESULT_SHIFT_IS_PARENT)
#define SIO_RESULT_UNK_2 (0xF << SIO_RESULT_SHIFT_UNK_2)
#define SIO_RESULT_ERROR (1 << SIO_RESULT_SHIFT_ERROR)
#define SIO_RESULT_INVALID_ID (1 << SIO_RESULT_SHIFT_INVALID_ID)
#define SIO_RESULT_UNK_8 (1 << SIO_RESULT_SHIFT_UNK_8)

struct SioMultiCnt
{
    u16 baudRate:2;    // baud rate
    u16 si:1;          // SI terminal, 0 = parent, 1 = child
    u16 sd:1;          // SD terminal, 0 = bad connection, 1 = all GBAs ready
    u16 id:2;          // ID, 0 = parent, 1-3 = 1st-3rd child
    u16 error:1;       // error flag
    u16 enable:1;      // SIO enable
    u16 unused_11_8:4;
    u16 mode:2;        // communication mode (should equal 2)
    u16 intrEnable:1;  // IRQ enable
    u16 unused_15:1;
    u16 data;          // data
};

#define SIO_MULTI_CNT ((struct SioMultiCnt *)REG_SIO)

const char sAgbLibMultiSioSyncVersion[] = "MultiSioSync010214";

/**
 * @brief D79E4 | Initialize SIO registers and data
 * 
 */
void MultiSioSyncInitialize(void)
{
    WRITE_16(REG_IME, 0);
    WRITE_16(REG_IE, READ_16(REG_IE) & ~(IF_SERIAL | IF_TIMER3));
    WRITE_16(REG_IME, 1);

    WRITE_16(REG_RNCT, 0);
    WRITE_16(REG_SIO, SIO_MULTI_MODE);
    WRITE_16(REG_SIO, READ_16(REG_SIO) | (SIO_IRQ_ENABLE | SIO_BAUD_RATE_115200));

    CPU_FILL_32(0, &sMultiSioSyncState, sizeof(sMultiSioSyncState));

    sMultiSioSyncState.sendWordIndex = 12;
    sMultiSioSyncState.receiveWordIndex = 12;
    sMultiSioSyncState.sendBuildBuffer = (struct MultiSioPacket *)&sMultiSioSyncState.unk_30;
    sMultiSioSyncState.sendActiveBuffer = (struct MultiSioPacket *)&sMultiSioSyncState.unk_48;
    sMultiSioSyncState.receiveWriteBuffer = &sMultiSioSyncState.unk_60;
    sMultiSioSyncState.receivePendingBuffer = &sMultiSioSyncState.unk_90;
    sMultiSioSyncState.receiveReadBuffer = &sMultiSioSyncState.unk_C0;

    WRITE_16(REG_IME, 0);
    WRITE_16(REG_IE, READ_16(REG_IE) | IF_SERIAL);
    WRITE_16(REG_IME, 1);
}

/**
 * @brief Per-frame link step: on the parent start the timer-driven transfer
 * once all units are ready, count the start-up frames, check received
 * packets into recv and return the SIO result flags.
 */
s32 MultiSioSyncMain(struct LinkFrameSlot *param_0)
{
    struct SioMultiCnt sioMultiCnt;

    sioMultiCnt = *SIO_MULTI_CNT;

    switch (sMultiSioSyncState.stage)
    {
        case 0:
            if (sioMultiCnt.id == 0)
            {
                // if connection not ready, or already started, then break
                if (!sioMultiCnt.sd || sioMultiCnt.enable)
                    break;

                // if parent GBA and unk_14 is 12 (what does this mean?)
                if (!sioMultiCnt.si && (sMultiSioSyncState.sendWordIndex == 12))
                {   
                    WRITE_16(REG_IME, 0);
                    WRITE_16(REG_IE, READ_16(REG_IE) & ~IF_SERIAL);
                    WRITE_16(REG_IE, READ_16(REG_IE) | IF_TIMER3);
                    WRITE_16(REG_IME, 1);

                    SIO_MULTI_CNT->intrEnable = 0;
                    WRITE_32(REG_TM3CNT_L, 0xB1FC);
                    WRITE_16(REG_IF, IF_SERIAL | IF_TIMER3);
                    sMultiSioSyncState.isParent = 8;
                }
            }

            sMultiSioSyncState.stage = 1;
            /* fallthrough */
        case 1:
            if (sMultiSioSyncState.connectedMask != 0)
            {
                if (sMultiSioSyncState.startupFrameCounter < 8)
                {
                    sMultiSioSyncState.startupFrameCounter += 1;
                }
                else
                {
                    sMultiSioSyncState.stage = 2;
                }
            }
            /* fallthrough */
        case 2:
            MultiSioSyncRecvDataCheck(param_0);
            break;
    }

    sMultiSioSyncState.counter += 1;

    return    sMultiSioSyncState.receivedMask << SIO_RESULT_SHIFT_UNK_3
            | (sMultiSioSyncState.isParent == 8) << SIO_RESULT_SHIFT_IS_PARENT
            | sMultiSioSyncState.connectedMask << SIO_RESULT_SHIFT_UNK_2
            | (sMultiSioSyncState.errorFlags != 0) << SIO_RESULT_SHIFT_ERROR
            | (sioMultiCnt.id > 1) << SIO_RESULT_SHIFT_INVALID_ID
            | (sMultiSioSyncState.startupFrameCounter >> 3) << SIO_RESULT_SHIFT_UNK_8;
}

/**
 * @brief Build the next send packet (frame counter, connection bits, 16 data
 * bytes) with its checksum and mark it ready.
 */
void MultiSioSyncSendDataSet(struct LinkFrameState *param_0)
{
    s32 var_r5;
    u32 i;
    u16 *var_r1;

    var_r5 = 0;
    sMultiSioSyncState.sendBuildBuffer->frameCounter = sMultiSioSyncState.counter;
    sMultiSioSyncState.sendBuildBuffer->connectionMaskXor = sMultiSioSyncState.connectedMask ^ sMultiSioSyncState.receivedMask;
    sMultiSioSyncState.sendBuildBuffer->checksum = 0;
    CPU_COPY_32(param_0, &sMultiSioSyncState.sendBuildBuffer->payload, 0x10);

    for (i = 0; i < 10; i++)
    {
        var_r5 += ((u16*)sMultiSioSyncState.sendBuildBuffer)[i];
    }

    sMultiSioSyncState.sendBuildBuffer->checksum = ~var_r5 - 12;
    sMultiSioSyncState.sendReady = 1;
}

/**
 * @brief Swap in the completed receive buffers, copy every packet whose
 * checksum is valid to recv and return the mask of units received.
 */
s32 MultiSioSyncRecvDataCheck(struct LinkFrameSlot *param_0)
{
    s32 var_r4;
    s32 i;
    u16 *temp_r1_3;
    u32 j;
    s32 temp_r1_2;
    vu32 var_0 = 0; // Necessary to match

    WRITE_16(REG_IME, 0);
    temp_r1_3 = sMultiSioSyncState.receiveReadBuffer;
    sMultiSioSyncState.receiveReadBuffer = sMultiSioSyncState.receivePendingBuffer;
    sMultiSioSyncState.receivePendingBuffer = temp_r1_3;
    temp_r1_2 = sMultiSioSyncState.receiveReady;
    sMultiSioSyncState.receiveReady = 0;
    WRITE_16(REG_IME, 1);

    sMultiSioSyncState.receivedMask = 0;
    if (temp_r1_2 != 0)
    {
        for(i = 0; i < 2; i++)
        {
            temp_r1_3 = sMultiSioSyncState.receiveReadBuffer + (i * 0x18);
            var_r4 = 0;

            for (j = 0; j < 10; j++)
            {
                var_r4 += temp_r1_3[j];
            }

            if ((s16)var_r4 == -13)
            {
                CPU_COPY_32(temp_r1_3 + 2, &param_0[i], 0x10);
                sMultiSioSyncState.receivedMask |= 1 << i;
            }
            CPU_FILL_32(0, temp_r1_3 + 2, 0x10);
        } 
    }

    sMultiSioSyncState.connectedMask |= sMultiSioSyncState.receivedMask;
    return sMultiSioSyncState.receivedMask;
}

/**
 * @brief D7CA0 | Handle Multi SIO during V-Blank
 * 
 */
void MultiSioSyncVSync(void)
{
    void *temp_r1;

    if (sMultiSioSyncState.isParent)
    {
        if ((sMultiSioSyncState.stage != 0) && (sMultiSioSyncState.transfersEnabled != 0))
        {
            sMultiSioSyncState.receiveWordIndex = -1;
            temp_r1 = sMultiSioSyncState.receivePendingBuffer;
            sMultiSioSyncState.receivePendingBuffer = sMultiSioSyncState.receiveWriteBuffer;
            sMultiSioSyncState.receiveWriteBuffer = temp_r1;

            if (sMultiSioSyncState.sendReady != 0)
            {
                temp_r1 = sMultiSioSyncState.sendActiveBuffer;
                sMultiSioSyncState.sendActiveBuffer = sMultiSioSyncState.sendBuildBuffer;
                sMultiSioSyncState.sendBuildBuffer = temp_r1;
                sMultiSioSyncState.sendReady = 0;
                sMultiSioSyncState.sendWordIndex = 0;
            }

            sMultiSioSyncState.errorFlags = SIO_MULTI_CNT->error;
            SIO_MULTI_CNT->data = 0xFEFE;
            // SIO_MULTI_CNT->enable = 1;
            WRITE_16(REG_SIO, READ_16(REG_SIO) | SIO_START_BIT_ACTIVE);
            WRITE_16(REG_TM3CNT_H, TIMER_CONTROL_ACTIVE | TIMER_CONTROL_IRQ_ENABLE);
        }
    }
    else
    {
        if (!sMultiSioSyncState.sioInterrupted)
        {
            WRITE_16(REG_IME, 0);
            INTR_CHECK |= IF_SERIAL;
            WRITE_16(REG_IME, 1);
        }
        sMultiSioSyncState.sioInterrupted = 0;
    }
}

/**
 * @brief D7D30 | Handle sending and receiving data during SIO interrupt
 * 
 */
void MultiSioSyncInterrupt(void)
{
    s32 i;
    struct MultiSioPacket *temp_r1_2;
    u16 *var_r1;
    u16 *var_r2;
    void *temp_r1;
    u16 recv[4];

    WRITE_64(recv, READ_64(REG_SIO_MULTI));
    sMultiSioSyncState.errorFlags = SIO_MULTI_CNT->error;
    if ((recv[0] == 0xFEFE) && (sMultiSioSyncState.receiveWordIndex > 9))
    {
        sMultiSioSyncState.receiveWordIndex = -1;
        temp_r1 = sMultiSioSyncState.receivePendingBuffer;
        sMultiSioSyncState.receivePendingBuffer = sMultiSioSyncState.receiveWriteBuffer;
        sMultiSioSyncState.receiveWriteBuffer = temp_r1;

        if (sMultiSioSyncState.sendReady != 0)
        {
            temp_r1_2 = sMultiSioSyncState.sendActiveBuffer;
            sMultiSioSyncState.sendActiveBuffer = sMultiSioSyncState.sendBuildBuffer;
            sMultiSioSyncState.sendBuildBuffer = temp_r1_2;
            sMultiSioSyncState.sendReady = 0;
            sMultiSioSyncState.sendWordIndex = 0;
        }

        WRITE_16(REG_IME, 0);
        INTR_CHECK |= IF_SERIAL;
        WRITE_16(REG_IME, 1);
    }

    if (sMultiSioSyncState.sendWordIndex < 10)
    {
        SIO_MULTI_CNT->data = ((u16*)sMultiSioSyncState.sendActiveBuffer)[sMultiSioSyncState.sendWordIndex];
    }

    if (sMultiSioSyncState.sendWordIndex < 11)
    {
        sMultiSioSyncState.sendWordIndex += 1;
    }

    if (sMultiSioSyncState.receiveWordIndex >= 0)
    {
        for (i = 0; i < 2; i++)
        {
            ((u16 (*) [12])sMultiSioSyncState.receiveWriteBuffer)[i][sMultiSioSyncState.receiveWordIndex] = recv[i];
        }

        if (sMultiSioSyncState.receiveWordIndex == 9)
        {
            sMultiSioSyncState.receiveReady = 1;
        }
    }

    if (sMultiSioSyncState.receiveWordIndex < 11)
    {
        sMultiSioSyncState.receiveWordIndex += 1;
    }

    if (sMultiSioSyncState.isParent)
    {
        WRITE_16(REG_TM3CNT_H, 0);
    }

    if ((sMultiSioSyncState.sendWordIndex < 11) && sMultiSioSyncState.isParent)
    {
        WRITE_16(REG_SIO, READ_16(REG_SIO) | SIO_START_BIT_ACTIVE);
        WRITE_16(REG_TM3CNT_H, TIMER_CONTROL_ACTIVE | TIMER_CONTROL_IRQ_ENABLE);
    }

    sMultiSioSyncState.sioInterrupted = 1;
}

/**
 * @brief On the parent, allow the transfers to start.
 */
void MultiSioSyncStart(void)
{
    if (sMultiSioSyncState.isParent)
    {
        sMultiSioSyncState.transfersEnabled = 1;
    }
}

/**
 * @brief Disable the serial and timer 3 interrupts, reset multi-player mode
 * (115200 baud) and stop the transfers.
 */
void MultiSioSyncStop(void)
{
    WRITE_16(REG_IME, 0);
    WRITE_16(REG_IE, READ_16(REG_IE) & ~(IF_SERIAL | IF_TIMER3));
    WRITE_16(REG_IME, 1);

    WRITE_16(REG_SIO, SIO_MULTI_MODE | SIO_BAUD_RATE_115200);
    WRITE_32(REG_TM3CNT_L, 0xB1FC);
    WRITE_16(REG_IF, IF_SERIAL | IF_TIMER3);

    sMultiSioSyncState.transfersEnabled = 0;
}

// file split?

/**
 * @brief Called every frame by AgbMain: while the link is active, start it
 * and send the 0xA2C marker; run MultiSioSyncMain, queue the send packet
 * and clear the send-slot flags.
 */
void Link_UpdateFrame(void)
{
    if (sLinkFrameState.active != 0)
    {
        MultiSioSyncStart();
    }

    sLinkFrameState.packetSlots[0].markerWords[0] = (sLinkFrameState.active != 0) ? 0xA2C : 0;
    sLinkFrameState.sioResultFlags = MultiSioSyncMain(&sLinkFrameState.packetSlots[1]);
    MultiSioSyncSendDataSet(&sLinkFrameState);
    CPU_FILL_16(0, sLinkFrameState.sendSlotBusy, sizeof(sLinkFrameState.sendSlotBusy));
}

/**
 * @brief Wait for V-blank, or for the serial interrupt when this unit is a
 * connected child.
 */
void Link_WaitForFrame(void)
{
    if (!(sLinkFrameState.sioResultFlags & SIO_RESULT_UNK_2) || (sLinkFrameState.sioResultFlags & SIO_RESULT_IS_PARENT))
    {
        VBlankIntrWait();
    }
    else
    {
        IntrWait(1, IF_SERIAL);
    }
}

/**
 * @brief V-blank handler hook: MultiSioSyncVSync.
 */
void Link_VSync(void)
{
    MultiSioSyncVSync();
}

/**
 * @brief Return 1 when the last link result is valid and all connected units
 * delivered data.
 */
s32 Link_IsConnected(void)
{
    return Link_IsResultValid() != 0 && Link_ReceivedFromAll() != 0;
}

/**
 * @brief Return 1 when the other unit sends the 0xA2C active marker.
 */
s32 Link_IsPeerActive(void)
{
    s32 var_r2;

    var_r2 = MultiSioSyncGetId() == 0;

    // TODO: figure out match
    return sLinkFrameState.packetSlots[var_r2].markerWords[8] == 0xA2C;
}

/**
 * @brief Initialise MultiSioSync, start the serial unit and clear the link
 * state (inactive).
 */
void Link_Init(void)
{
    MultiSioSyncInitialize();
    WRITE_16(REG_SIO, READ_16(REG_SIO) | SIO_START_BIT_ACTIVE);
    DMA_FILL_32(3, 0, &sLinkFrameState, sizeof(sLinkFrameState));
    sLinkFrameState.active = 0;
}

/**
 * @brief Start the transfers and mark the link active.
 */
void Link_Start(void)
{
    MultiSioSyncStart();
    sLinkFrameState.active = 1;
}

/**
 * @brief Stop the transfers and mark the link inactive.
 */
void Link_Stop(void)
{
    MultiSioSyncStop();
    sLinkFrameState.active = 0;
}

/**
 * @brief Unreferenced: queue one byte in send slot index unless the slot is
 * busy; return 1 on success.
 */
s32 Link_QueueSendByte(u8 param_0, s32 param_1)
{
    s16 var_0;

    var_0 = param_1;
    if (sLinkFrameState.sendSlotBusy[var_0] == 0)
    {
        sLinkFrameState.sendSlotBusy[var_0] = 1;
        sLinkFrameState.packetSlots[0].payloadBytes[param_1] = param_0;
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Queue count bytes into the send slots; return 0 as soon as a slot is
 * still busy, else 1.
 */
s32 Link_QueueSendBytes(u8 *data, s32 count)
{
    s32 i;
    s32 value;
    s32 busy;

    for (i = 0; i < count; i++)
    {
        value = data[i];
        busy = sLinkFrameState.sendSlotBusy[(s16)i] != 0;
        if (busy)
            return 0;
        sLinkFrameState.sendSlotBusy[(s16)i] = 1;
        sLinkFrameState.packetSlots[0].payloadBytes[i] = value;
    }

    return 1;
}

/**
 * @brief Unreferenced: return received byte index from the other unit.
 */
u8 Link_GetReceivedByte(s32 param_0)
{
    struct LinkFrameState *var_0;

    var_0 = &sLinkFrameState;
    if (MultiSioSyncGetId() == 0)
    {
        param_0 += 0x10;
    }
    return var_0->packetSlots[1].payloadBytes[param_0];
}

/**
 * @brief Copy count received bytes from the other unit to dst.
 */
void Link_CopyReceivedBytes(u8 *param_0, s32 param_1)
{
    s32 i;
    u8 *var_r2;
    u8 *var_r3;

    if (MultiSioSyncGetId() == 0)
    {
        var_r3 = sLinkFrameState.packetSlots[1].payloadBytes + 0x10;
    }
    else
    {
        var_r3 = sLinkFrameState.packetSlots[1].payloadBytes;
    }

    var_r2 = param_0;
    for (i = 0; i < param_1; i++)
    {
        *var_r2++ = *var_r3++;
    }
}

/**
 * @brief D80BC | Get the ID of the GBA
 * 
 * @return s16 GBA ID, 0 is parent, 1-3 is 1st-3rd child
 */
s16 MultiSioSyncGetId(void)
{
    return SIO_MULTI_CNT->id;
}

/**
 * @brief Return 1 when the link result has data and neither the error nor
 * the invalid-id flag.
 */
s32 Link_IsResultValid(void)
{
    return (sLinkFrameState.sioResultFlags & (SIO_RESULT_INVALID_ID | SIO_RESULT_ERROR | 1)) == 1;
}

// TODO: Real bitfield?
struct MultiSioResultBits {
    u32 receivedMask:4;
    u32 unk_4:4;
    u32 connectedMask:4;
    u32 unk_12:3;
    u32 startupComplete:1;
};

/**
 * @brief Return 0 when start-up is done and the units received this frame
 * differ from the connected units, else 1.
 */
s32 Link_ReceivedFromAll(void)
{
    u32 temp_r0;
    struct MultiSioResultBits *var_0;

    var_0 = (struct MultiSioResultBits *)&sLinkFrameState.sioResultFlags;
    if (sLinkFrameState.sioResultFlags & SIO_RESULT_UNK_8)
    // if (var_0->unk_15)
    {
        if (var_0->receivedMask != var_0->connectedMask)
        {
            return 0;
        }
    }
    return 1;
}

