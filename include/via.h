#ifndef VIA_H
#define VIA_H

enum intr_status_bits {
        IntrRxDone      = 0x0001,
        IntrTxDone      = 0x0002,
        IntrRxErr       = 0x0004,
        IntrTxError     = 0x0008,
		IntrTxAbort		= 0x0008,
        IntrRxEmpty     = 0x0020,
        IntrPCIErr      = 0x0040,
        IntrStatsMax    = 0x0080,
        IntrRxEarly     = 0x0100,
		IntrMIIChange	= 0x0200,
        IntrTxUnderrun  = 0x0210,
        IntrRxOverflow  = 0x0400,
        IntrRxDropped   = 0x0800,
        IntrRxNoBuf     = 0x1000,
        IntrTxAborted   = 0x2000,
        IntrLinkChange  = 0x4000,
        IntrRxWakeUp    = 0x8000,
        IntrTxDescRace          = 0x080000,     /* mapped from IntrStatus2 */
        IntrNormalSummary       = IntrRxDone | IntrTxDone,
        IntrTxErrSummary        = IntrTxDescRace | IntrTxAborted | IntrTxError |
                                  IntrTxUnderrun,
};

#define RHINE_EVENT_NAPI_RX     (IntrRxDone | \
                                 IntrRxErr | \
                                 IntrRxEmpty | \
                                 IntrRxOverflow | \
                                 IntrRxDropped | \
                                 IntrRxNoBuf | \
                                 IntrRxWakeUp)

#define RHINE_EVENT_NAPI_TX_ERR (IntrTxError | \
                                 IntrTxAborted | \
                                 IntrTxUnderrun | \
                                 IntrTxDescRace)
#define RHINE_EVENT_NAPI_TX     (IntrTxDone | RHINE_EVENT_NAPI_TX_ERR)

#define RHINE_EVENT_NAPI        (RHINE_EVENT_NAPI_RX | \
                                 RHINE_EVENT_NAPI_TX | \
                                 IntrStatsMax)
#define RHINE_EVENT_SLOW        (IntrPCIErr | IntrLinkChange)
#define RHINE_EVENT             (RHINE_EVENT_NAPI | RHINE_EVENT_SLOW)

void start_nic(void);
void via_handle_intr(void);
void dhcp_get_ip(void);

#endif