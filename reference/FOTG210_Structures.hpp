#pragma once
#include <cstdint>

namespace FOTG210 {

// ===========================================================================================
// CAPABILITY REGISTERS (Base + 0x00)
// ===========================================================================================
namespace CapabilityRegisters {
constexpr uintptr_t CapLength = 0x00; // Capability Register Length (8-bit)
constexpr uintptr_t HCIVersion =
    0x02; // Host Controller Interface Version (16-bit)
constexpr uintptr_t HCSPARAMS = 0x04; // Host Controller Structural Parameters
constexpr uintptr_t HCCPARAMS = 0x08; // Host Controller Capability Parameters
constexpr uintptr_t PortRoute = 0x0C; // Port Routing (nibbles for routing)
} // namespace CapabilityRegisters

// HCSPARAMS bits
enum class HCSPARAMS_Bits : uint32_t {
  N_PORTS_MASK = 0xF, // bits 3:0, ports on HC
};

// HCCPARAMS bits
enum class HCCPARAMS_Bits : uint32_t {
  PGM_FRAMELISTLEN = 1 << 1, // Programmable Frame List Length
  CAN_PARK = 1 << 2,         // Park async QH capability
};

// ===========================================================================================
// OPERATIONAL REGISTERS (Base + CapLength)
// ===========================================================================================
namespace OperationalRegisters {
constexpr uintptr_t Command = 0x00;         // USB Command Register
constexpr uintptr_t Status = 0x04;          // USB Status Register
constexpr uintptr_t InterruptEnable = 0x08; // USB Interrupt Enable Register
constexpr uintptr_t FrameIndex = 0x0C;      // Frame Index Register
constexpr uintptr_t CtrlDsSegment =
    0x10; // Control Data Structure Segment (address bits 63:32)
constexpr uintptr_t PeriodicListBase = 0x14; // Periodic Frame List Base Address
constexpr uintptr_t AsyncListAddr = 0x18;    // Async List Address
constexpr uintptr_t PortStatusCtrl = 0x20;   // Port Status and Control

// OTG Registers
constexpr uintptr_t OTGCSR = 0x70; // OTG Control and Status Register
constexpr uintptr_t OTGISR = 0x74; // OTG Interrupt Status Register

// Global Interrupt
constexpr uintptr_t GMIR = 0xB4; // Global Mask of Interrupt Register
} // namespace OperationalRegisters

// ===========================================================================================
// USB COMMAND REGISTER BITS (Offset 0x00)
// ===========================================================================================
enum class CommandBits : uint32_t {
  Run = 1 << 0,                    // Run/Stop (RS)
  Reset = 1 << 1,                  // Host Controller Reset (HCRESET)
  PeriodicScheduleEnable = 1 << 4, // Periodic Schedule Enable (PSE)
  AsyncScheduleEnable = 1 << 5,    // Async Schedule Enable (ASE)
  InterruptAsyncAdvance = 1 << 6,  // Interrupt on Async Advance Doorbell (IAAD)
  Park = 1 << 11,                  // Asynchronous Schedule Park Mode Enable
};

// Park count helper (bits 9:8)
constexpr uint32_t CMD_PARK_CNT(uint32_t c) { return (c >> 8) & 0x3; }

// ===========================================================================================
// USB STATUS REGISTER BITS (Offset 0x04)
// ===========================================================================================
enum class StatusBits : uint32_t {
  USBInterrupt = 1 << 0,          // USB Interrupt (USBINT) - normal completion
  USBErrorInterrupt = 1 << 1,     // USB Error Interrupt (USBERRINT)
  PortChangeDetect = 1 << 2,      // Port Change Detect (PCD)
  FrameListRollover = 1 << 3,     // Frame List Rollover (FLR)
  HostSystemError = 1 << 4,       // Host System Error (HSE) - PCI access error
  InterruptAsyncAdvance = 1 << 5, // Interrupt on Async Advance (IAA)
  HCHalted = 1 << 12,             // Host Controller Halted (HCHalted)
  Reclamation = 1 << 13,          // Reclamation
  PeriodicScheduleStatus = 1 << 14, // Periodic Schedule Status (PSS)
  AsyncScheduleStatus = 1 << 15,    // Async Schedule Status (ASS)
};

// ===========================================================================================
// USB INTERRUPT ENABLE BITS (Offset 0x08)
// Uses same bit positions as Status Register for interrupt sources
// ===========================================================================================
enum class InterruptEnableBits : uint32_t {
  USBInterruptEnable = 1 << 0,      // USB Interrupt Enable
  USBErrorInterruptEnable = 1 << 1, // USB Error Interrupt Enable
  PortChangeIntEnable = 1 << 2,     // Port Change Interrupt Enable
  FrameListRolloverEnable = 1 << 3, // Frame List Rollover Enable
  HostSystemErrorEnable = 1 << 4,   // Host System Error Enable
  AsyncAdvanceIntEnable = 1 << 5,   // Interrupt on Async Advance Enable
};

// ===========================================================================================
// PORT STATUS AND CONTROL BITS (Offset 0x20)
// ===========================================================================================
enum class PortStatusBits : uint32_t {
  CurrentConnectStatus = 1 << 0, // Current Connect Status (CCS)
  ConnectStatusChange = 1 << 1,  // Connect Status Change (CSC)
  PortEnabled = 1 << 2,          // Port Enabled (PE)
  PortEnableChange = 1 << 3,     // Port Enable/Disable Change (PEC)
  OvercurrentActive = 1 << 4,    // Over-current Active (OCA)
  OvercurrentChange = 1 << 5,    // Over-current Change (OCC)
  ForcePortResume = 1 << 6,      // Force Port Resume (FPR)
  Suspend = 1 << 7,              // Suspend
  PortReset = 1 << 8,            // Port Reset (PR)
  LineStatus_MASK = 3 << 10,     // Line Status (bits 11:10)
  PortPower = 1 << 12,           // Port Power (PP)
  PortOwner = 1 << 13,           // Port Owner (PO)
  // Bits 15:14 - Port Indicator Control
  PortTestControl_MASK = 0xF << 16, // Port Test Control (bits 19:16)
  WakeOnConnect = 1 << 20,          // Wake on Connect Enable (WKOC_E)
  WakeOnDisconnect = 1 << 21,       // Wake on Disconnect Enable (WKDC_E)
  WakeOnOvercurrent = 1 << 22,      // Wake on Over-current Enable (WKOC_E)
};

// Line status helper
constexpr bool PORT_USB11(uint32_t portsc) {
  return ((portsc & (3 << 10)) == (1 << 10));
}

// Read/Write-to-Clear bits
constexpr uint32_t PORT_RWC_BITS =
    static_cast<uint32_t>(PortStatusBits::ConnectStatusChange) |
    static_cast<uint32_t>(PortStatusBits::PortEnableChange);

// ===========================================================================================
// OTG CONTROL AND STATUS REGISTER BITS (Offset 0x70)
// ===========================================================================================
enum class OTGCSRBits : uint32_t {
  A_BUS_REQ = 1 << 4,          // A-device bus request
  A_BUS_DROP = 1 << 5,         // A-device bus drop
  HOST_SPD_TYP_MASK = 3 << 22, // Host Speed Type (bits 23:22)
};

// Speed type extraction
constexpr uint32_t OTGCSR_GET_SPD_TYP(uint32_t val) {
  return (val >> 22) & 0x3;
}

// ===========================================================================================
// OTG INTERRUPT STATUS REGISTER BITS (Offset 0x74)
// ===========================================================================================
enum class OTGISRBits : uint32_t {
  OverCurrent = 1 << 10, // Over-current interrupt
};

// ===========================================================================================
// GLOBAL MASK INTERRUPT REGISTER BITS (Offset 0xB4)
// ===========================================================================================
enum class GMIRBits : uint32_t {
  MDEV_INT = 1 << 0,     // Mask Device Interrupt
  MOTG_INT = 1 << 1,     // Mask OTG Interrupt
  MHC_INT = 1 << 2,      // Mask Host Controller Interrupt
  INT_POLARITY = 1 << 3, // Interrupt Polarity (Active High)
};

// ===========================================================================================
// DEVICE CONTROLLER REGISTERS (Base + 0x100)
// ===========================================================================================
namespace DeviceRegisters {
constexpr uintptr_t DMCR = 0x100;    // Device Main Control Register
constexpr uintptr_t DAR = 0x104;     // Device Address Register
constexpr uintptr_t DTR = 0x108;     // Device Test Register
constexpr uintptr_t PHYTMSR = 0x114; // PHY Test Mode Selector Register
constexpr uintptr_t DCFESR = 0x120;  // Cx Config and FIFO Empty Status Register
constexpr uintptr_t DICR = 0x124;    // Device IDLE Counter Register
constexpr uintptr_t DMIGR = 0x130;   // Device Mask of Interrupt Group Register
constexpr uintptr_t DMISGR0 = 0x134; // Device Mask of Interrupt Source Group 0
constexpr uintptr_t DMISGR1 = 0x138; // Device Mask of Interrupt Source Group 1
constexpr uintptr_t DMISGR2 = 0x13C; // Device Mask of Interrupt Source Group 2
constexpr uintptr_t DIGR = 0x140;    // Device Interrupt Group Register
constexpr uintptr_t DISGR0 = 0x144;  // Device Interrupt Source Group 0
constexpr uintptr_t DISGR1 = 0x148;  // Device Interrupt Source Group 1
constexpr uintptr_t DISGR2 = 0x14C;  // Device Interrupt Source Group 2
constexpr uintptr_t RX0BYTE = 0x150; // Device Receive Zero-Length Data Packet
constexpr uintptr_t TX0BYTE = 0x154; // Device Transfer Zero-Length Data Packet
constexpr uintptr_t EPMAP = 0x1A0;   // Device Endpoint 1~4 Map Register
constexpr uintptr_t FIFOMAP = 0x1A8; // Device FIFO Map Register
constexpr uintptr_t FIFOCF = 0x1AC;  // Device FIFO Configuration Register
constexpr uintptr_t DMATFNR = 0x1C0; // Device DMA Target FIFO Number Register
constexpr uintptr_t DMACPSR1 = 0x1C8; // Device DMA Controller Parameter 1
constexpr uintptr_t DMACPSR2 = 0x1CC; // Device DMA Controller Parameter 2
constexpr uintptr_t CXPORT = 0x1D0;   // Control Endpoint Port

// Dynamic endpoint registers
constexpr uintptr_t INEPMPSR(uint8_t ep) { return 0x160 + 4 * (ep - 1); }
constexpr uintptr_t OUTEPMPSR(uint8_t ep) { return 0x180 + 4 * (ep - 1); }
constexpr uintptr_t FIBCR(uint8_t fifo) { return 0x1B0 + fifo * 4; }
} // namespace DeviceRegisters

// ===========================================================================================
// DEVICE MAIN CONTROL REGISTER BITS (Offset 0x100)
// ===========================================================================================
enum class DMCRBits : uint32_t {
  CAP_RMWAKUP = 1 << 0, // Capability Remote Wakeup
  HALF_SPEED = 1 << 1,  // Half Speed Mode
  GLINT_EN = 1 << 2,    // Global Interrupt Enable
  GOSUSP = 1 << 3,      // Go Suspend
  SFRST = 1 << 4,       // Soft Reset
  CHIP_EN = 1 << 5,     // Chip Enable
  HS_EN = 1 << 6,       // High Speed Enable
};

// ===========================================================================================
// DEVICE ADDRESS REGISTER BITS (Offset 0x104)
// ===========================================================================================
enum class DARBits : uint32_t {
  AFT_CONF = 1 << 7, // After Configuration
};

// ===========================================================================================
// DEVICE TEST REGISTER BITS (Offset 0x108)
// ===========================================================================================
enum class DTRBits : uint32_t {
  TST_CLRFF = 1 << 0, // Test Clear FIFO
};

// ===========================================================================================
// PHY TEST MODE SELECTOR REGISTER BITS (Offset 0x114)
// ===========================================================================================
enum class PHYTMSRBits : uint32_t {
  UNPLUG = 1 << 0,     // Unplug
  TST_JSTA = 1 << 1,   // Test J-State
  TST_KSTA = 1 << 2,   // Test K-State
  TST_SE0NAK = 1 << 3, // Test SE0/NAK
  TST_PKT = 1 << 4,    // Test Packet
};

// ===========================================================================================
// CX CONFIGURATION AND FIFO EMPTY STATUS REGISTER BITS (Offset 0x120)
// ===========================================================================================
enum class DCFESRBits : uint32_t {
  CX_DONE = 1 << 0,    // Control Transfer Done
  TST_PKDONE = 1 << 1, // Test Packet Done
  CX_STL = 1 << 2,     // Control Transfer Stall
  CX_CLR = 1 << 3,     // Control Transfer Clear
  CX_EMP = 1 << 5,     // Control Endpoint Empty
};

// FIFO Empty helper
constexpr uint32_t DCFESR_FIFO_EMPTY(uint8_t fifo) { return 1 << (8 + fifo); }

// ===========================================================================================
// DEVICE MASK OF INTERRUPT GROUP REGISTER BITS (Offset 0x130)
// ===========================================================================================
enum class DMIGRBits : uint32_t {
  MINT_G0 = 1 << 0, // Mask Interrupt Group 0
  MINT_G1 = 1 << 1, // Mask Interrupt Group 1
  MINT_G2 = 1 << 2, // Mask Interrupt Group 2
};

// ===========================================================================================
// DEVICE MASK INTERRUPT SOURCE GROUP 0 BITS (Offset 0x134)
// ===========================================================================================
enum class DMISGR0Bits : uint32_t {
  MCX_SETUP_INT = 1 << 0, // Mask CX Setup Interrupt
  MCX_IN_INT = 1 << 1,    // Mask CX IN Interrupt
  MCX_OUT_INT = 1 << 2,   // Mask CX OUT Interrupt
  MCX_COMEND = 1 << 3,    // Mask CX Command End
};

// ===========================================================================================
// DEVICE MASK INTERRUPT SOURCE GROUP 1 BITS (Offset 0x138)
// ===========================================================================================
enum class DMISGR1Bits : uint32_t {
  MF0_OUT_INT = 1 << 0, // Mask FIFO 0 OUT Interrupt
  MF0_SPK_INT = 1 << 1, // Mask FIFO 0 Short Packet Interrupt
  MF1_OUT_INT = 1 << 2, // Mask FIFO 1 OUT Interrupt
  MF1_SPK_INT = 1 << 3, // Mask FIFO 1 Short Packet Interrupt
  MF2_OUT_INT = 1 << 4, // Mask FIFO 2 OUT Interrupt
  MF2_SPK_INT = 1 << 5, // Mask FIFO 2 Short Packet Interrupt
  MF3_OUT_INT = 1 << 6, // Mask FIFO 3 OUT Interrupt
  MF3_SPK_INT = 1 << 7, // Mask FIFO 3 Short Packet Interrupt
  MF0_IN_INT = 1 << 16, // Mask FIFO 0 IN Interrupt
  MF1_IN_INT = 1 << 17, // Mask FIFO 1 IN Interrupt
  MF2_IN_INT = 1 << 18, // Mask FIFO 2 IN Interrupt
  MF3_IN_INT = 1 << 19, // Mask FIFO 3 IN Interrupt
};

// Helpers
constexpr uint32_t DMISGR1_MF_IN_INT(uint8_t fifo) { return 1 << (16 + fifo); }
constexpr uint32_t DMISGR1_MF_OUTSPK_INT(uint8_t fifo) {
  return 0x3 << (fifo * 2);
}

// ===========================================================================================
// DEVICE MASK INTERRUPT SOURCE GROUP 2 BITS (Offset 0x13C)
// ===========================================================================================
enum class DMISGR2Bits : uint32_t {
  MDMA_CMPLT = 1 << 7, // Mask DMA Complete
  MDMA_ERROR = 1 << 8, // Mask DMA Error
};

// ===========================================================================================
// DEVICE INTERRUPT GROUP REGISTER BITS (Offset 0x140)
// ===========================================================================================
enum class DIGRBits : uint32_t {
  INT_G0 = 1 << 0, // Interrupt Group 0
  INT_G1 = 1 << 1, // Interrupt Group 1
  INT_G2 = 1 << 2, // Interrupt Group 2
};

// ===========================================================================================
// DEVICE INTERRUPT SOURCE GROUP 0 BITS (Offset 0x144)
// ===========================================================================================
enum class DISGR0Bits : uint32_t {
  CX_SETUP_INT = 1 << 0,   // CX Setup Interrupt
  CX_IN_INT = 1 << 1,      // CX IN Interrupt
  CX_OUT_INT = 1 << 2,     // CX OUT Interrupt
  CX_COMEND_INT = 1 << 3,  // CX Command End Interrupt
  CX_COMFAIL_INT = 1 << 4, // CX Command Fail Interrupt
  CX_COMABT_INT = 1 << 5,  // CX Command Abort Interrupt
};

// ===========================================================================================
// DEVICE INTERRUPT SOURCE GROUP 1 BITS (Offset 0x148)
// ===========================================================================================
// Helpers for FIFO interrupts
constexpr uint32_t DISGR1_OUT_INT(uint8_t fifo) { return 1 << (fifo * 2); }
constexpr uint32_t DISGR1_SPK_INT(uint8_t fifo) {
  return 1 << (1 + (fifo * 2));
}
constexpr uint32_t DISGR1_IN_INT(uint8_t fifo) { return 1 << (16 + fifo); }

// ===========================================================================================
// DEVICE INTERRUPT SOURCE GROUP 2 BITS (Offset 0x14C)
// ===========================================================================================
enum class DISGR2Bits : uint32_t {
  USBRST_INT = 1 << 0,        // USB Reset Interrupt
  SUSP_INT = 1 << 1,          // Suspend Interrupt
  RESM_INT = 1 << 2,          // Resume Interrupt
  ISO_SEQ_ERR_INT = 1 << 3,   // Isochronous Sequence Error Interrupt
  ISO_SEQ_ABORT_INT = 1 << 4, // Isochronous Sequence Abort Interrupt
  TX0BYTE_INT = 1 << 5,       // TX Zero-Byte Interrupt
  RX0BYTE_INT = 1 << 6,       // RX Zero-Byte Interrupt
  DMA_CMPLT = 1 << 7,         // DMA Complete
  DMA_ERROR = 1 << 8,         // DMA Error
};

// ===========================================================================================
// RX0BYTE REGISTER BITS (Offset 0x150)
// ===========================================================================================
enum class RX0BYTEBits : uint32_t {
  EP1 = 1 << 0,
  EP2 = 1 << 1,
  EP3 = 1 << 2,
  EP4 = 1 << 3,
  EP5 = 1 << 4,
  EP6 = 1 << 5,
  EP7 = 1 << 6,
  EP8 = 1 << 7,
};

// ===========================================================================================
// TX0BYTE REGISTER BITS (Offset 0x154)
// ===========================================================================================
enum class TX0BYTEBits : uint32_t {
  EP1 = 1 << 0,
  EP2 = 1 << 1,
  EP3 = 1 << 2,
  EP4 = 1 << 3,
  EP5 = 1 << 4,
  EP6 = 1 << 5,
  EP7 = 1 << 6,
  EP8 = 1 << 7,
};

// ===========================================================================================
// ENDPOINT MAX PACKET SIZE REGISTER BITS (Offset 0x160/0x180)
// ===========================================================================================
constexpr uint32_t INOUTEPMPSR_MPS(uint32_t mps) { return mps & 0x2FF; }

enum class EPMPSRBits : uint32_t {
  STL_EP = 1 << 11,     // Stall Endpoint
  RESET_TSEQ = 1 << 12, // Reset Toggle Sequence
};

// ===========================================================================================
// FIFO CONFIGURATION REGISTER HELPERS (Offset 0x1AC)
// ===========================================================================================
constexpr uint32_t FIFOCF_TYPE(uint8_t type, uint8_t fifo) {
  return type << (fifo * 8);
}
constexpr uint32_t FIFOCF_BLK_SIN(uint8_t fifo) {
  return 0x0 << ((fifo * 8) + 2);
}
constexpr uint32_t FIFOCF_BLK_DUB(uint8_t fifo) {
  return 0x1 << ((fifo * 8) + 2);
}
constexpr uint32_t FIFOCF_BLK_TRI(uint8_t fifo) {
  return 0x2 << ((fifo * 8) + 2);
}
constexpr uint32_t FIFOCF_BLKSZ_512(uint8_t fifo) {
  return 0x0 << ((fifo * 8) + 4);
}
constexpr uint32_t FIFOCF_BLKSZ_1024(uint8_t fifo) {
  return 0x1 << ((fifo * 8) + 4);
}
constexpr uint32_t FIFOCF_FIFO_EN(uint8_t fifo) {
  return 0x1 << ((fifo * 8) + 5);
}

// ===========================================================================================
// FIFO INSTRUCTION AND BYTE COUNT REGISTER BITS (Offset 0x1B0+)
// ===========================================================================================
constexpr uint32_t FIBCR_BCFX_MASK = 0x7FF;

enum class FIBCRBits : uint32_t {
  FFRST = 1 << 12, // FIFO Reset
};

// ===========================================================================================
// DMA TARGET FIFO NUMBER REGISTER BITS (Offset 0x1C0)
// ===========================================================================================
enum class DMATFNRBits : uint32_t {
  ACC_F0 = 1 << 0,  // Access FIFO 0
  ACC_F1 = 1 << 1,  // Access FIFO 1
  ACC_F2 = 1 << 2,  // Access FIFO 2
  ACC_F3 = 1 << 3,  // Access FIFO 3
  ACC_CXF = 1 << 4, // Access Control Endpoint FIFO
  DISDMA = 0,       // Disable DMA
};

constexpr uint32_t DMATFNR_ACC_FN(uint8_t fifo) { return 1 << fifo; }

// ===========================================================================================
// DMA CONTROLLER PARAMETER 1 REGISTER BITS (Offset 0x1C8)
// ===========================================================================================
enum class DMACPSR1Bits : uint32_t {
  DMA_START = 1 << 0,   // DMA Start
  DMA_TYPE_IN = 1 << 1, // DMA Type: 1=IN, 0=OUT
  DMA_ABORT = 1 << 3,   // DMA Abort
};

constexpr uint32_t DMACPSR1_DMA_LEN(uint32_t len) {
  return (len & 0xFFFF) << 8;
}
constexpr uint32_t DMACPSR1_DMA_TYPE(bool dir_in) {
  return (dir_in ? 1 : 0) << 1;
}

// ===========================================================================================
// QUEUE TRANSFER DESCRIPTOR (QTD) - EHCI Section 3.5
// ===========================================================================================
struct alignas(32) QTD {
  uint32_t hw_next;      // Next qTD Pointer
  uint32_t hw_alt_next;  // Alternate Next qTD Pointer
  uint32_t hw_token;     // qTD Token
  uint32_t hw_buf[5];    // Buffer Page Pointer List
  uint32_t hw_buf_hi[5]; // Buffer Page Pointer (High 32-bits, 64-bit mode)
};

// QTD Token bits
enum class QTDTokenBits : uint32_t {
  PING = 1 << 0,             // PING State
  SplitState = 1 << 1,       // Split Transaction State
  MissedMicroFrame = 1 << 2, // Missed Micro-Frame
  TransactionError = 1 << 3, // Transaction Error
  Babble = 1 << 4,           // Babble Detected
  DataBufferError = 1 << 5,  // Data Buffer Error
  Halted = 1 << 6,           // Halted
  Active = 1 << 7,           // Active
  IOC = 1 << 15,             // Interrupt On Complete
  DataToggle = 1U << 31,     // Data Toggle
};

// QTD helpers
constexpr uint32_t QTD_LENGTH(uint32_t tok) { return (tok >> 16) & 0x7FFF; }
constexpr uint32_t QTD_PID(uint32_t tok) { return (tok >> 8) & 0x3; }
constexpr uint32_t QTD_CERR(uint32_t tok) { return (tok >> 10) & 0x3; }

// PID Codes
enum class QTD_PID_Code : uint8_t {
  OUT = 0,
  IN = 1,
  SETUP = 2,
};

// ===========================================================================================
// QUEUE HEAD (QH) - EHCI Section 3.6
// ===========================================================================================
struct alignas(32) QH_HW {
  uint32_t hw_next;    // Queue Head Horizontal Link Pointer
  uint32_t hw_info1;   // Endpoint Characteristics
  uint32_t hw_info2;   // Endpoint Capabilities
  uint32_t hw_current; // Current qTD Pointer
  // QTD overlay
  uint32_t hw_qtd_next;  // Next qTD Pointer
  uint32_t hw_alt_next;  // Alternate Next qTD Pointer
  uint32_t hw_token;     // Token
  uint32_t hw_buf[5];    // Buffer Page Pointer List
  uint32_t hw_buf_hi[5]; // Buffer Page Pointer (High 32-bits)
};

// QH info1 bits
enum class QHInfo1Bits : uint32_t {
  Inactivate = 1 << 7,             // Inactivate on Next Transaction
  FullSpeed = 0 << 12,             // Endpoint Speed: Full Speed
  LowSpeed = 1 << 12,              // Endpoint Speed: Low Speed
  HighSpeed = 2 << 12,             // Endpoint Speed: High Speed
  DataToggleControl = 1 << 14,     // Data Toggle Control
  HeadOfReclamationList = 1 << 15, // Head of Reclamation List Flag
  ControlEndpoint = 1 << 27,       // Control Endpoint Flag
};

// QH info2 masks
constexpr uint32_t QH_SMASK = 0x000000FF;   // Interrupt Schedule Mask
constexpr uint32_t QH_CMASK = 0x0000FF00;   // Split Completion Mask
constexpr uint32_t QH_HUBADDR = 0x007F0000; // Hub Address
constexpr uint32_t QH_HUBPORT = 0x3F800000; // Hub Port
constexpr uint32_t QH_MULT = 0xC0000000;    // High-Bandwidth Pipe Multiplier

// ===========================================================================================
// ISOCHRONOUS TRANSFER DESCRIPTOR (iTD) - EHCI Section 3.3
// ===========================================================================================
struct alignas(32) ITD {
  uint32_t hw_next;           // Next Link Pointer
  uint32_t hw_transaction[8]; // Transaction Status and Control
  uint32_t hw_bufp[7];        // Buffer Page Pointer List
  uint32_t hw_bufp_hi[7];     // Buffer Page Pointer (High 32-bits)
};

// iTD Transaction bits
enum class ITDTransactionBits : uint32_t {
  IOC = 1 << 15,              // Interrupt On Complete
  Active = 1U << 31,          // Active
  BufferError = 1 << 30,      // Data Buffer Error
  Babble = 1 << 29,           // Babble Detected
  TransactionError = 1 << 28, // Transaction Error
};

constexpr uint32_t ITD_LENGTH(uint32_t tok) { return (tok >> 16) & 0x0FFF; }

// ===========================================================================================
// PERIODIC FRAME SPAN TRAVERSAL NODE (FSTN) - EHCI Section 3.7
// ===========================================================================================
struct alignas(32) FSTN {
  uint32_t hw_next; // Next Link Pointer
  uint32_t hw_prev; // Back Path Link Pointer
};

// ===========================================================================================
// LINK POINTER TYPE TAGS
// ===========================================================================================
enum class QueueType : uint32_t {
  ITD = 0 << 1,  // Isochronous Transfer Descriptor
  QH = 1 << 1,   // Queue Head
  SITD = 2 << 1, // Split Transaction Isochronous TD
  FSTN = 3 << 1, // Frame Span Traversal Node
};

constexpr uint32_t LINK_TERMINATE = 0x1;

// ===========================================================================================
// SPEED TYPES
// ===========================================================================================
enum class SpeedType : uint8_t {
  FullSpeed = 0,
  LowSpeed = 1,
  HighSpeed = 2,
};

// ===========================================================================================
// CONSTANTS
// ===========================================================================================
constexpr uint8_t MAX_ROOT_PORTS = 1;
constexpr uint8_t MAX_NUM_EP = 5;   // EP0...EP4
constexpr uint8_t MAX_FIFO_NUM = 4; // FIFO0...FIFO3
constexpr uint16_t DEFAULT_PERIODIC_SIZE = 1024;

// ===========================================================================================
// QH STATE VALUES (Software)
// ===========================================================================================
enum class QHState : uint8_t {
  Linked = 1,
  Unlink = 2,
  Idle = 3,
  UnlinkWait = 4,
  Completing = 5,
};

} // namespace FOTG210
