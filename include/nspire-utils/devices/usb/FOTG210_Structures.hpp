#pragma once

#include <cstdint>
#include <cstddef>

namespace ntls::devices::usb {

    // FOTG210 Register Offsets
    // Capability Registers
    constexpr uintptr_t FOTG210_CapLength = 0x00;
    constexpr uintptr_t FOTG210_HCIVersion = 0x02;
    constexpr uintptr_t FOTG210_HCSPARAMS = 0x04;
    constexpr uintptr_t FOTG210_HCCPARAMS = 0x08;
    constexpr uintptr_t FOTG210_PORTROUTE = 0x0C;

    // Operational Registers
    constexpr uintptr_t FOTG210_CommandRegister = 0x00;
    constexpr uintptr_t FOTG210_StatusRegister  = 0x04;
    constexpr uintptr_t FOTG210_InterruptEnable  = 0x08;
    constexpr uintptr_t FOTG210_FrameIndex  = 0x0C;
    constexpr uintptr_t FOTG210_PeriodicListBase  = 0x14;
    constexpr uintptr_t FOTG210_AsyncListAddr  = 0x18;
    constexpr uintptr_t FOTG210_PortStatusControl  = 0x20;
    
    constexpr uintptr_t FOTG210_OTGStatusControl  = 0x70;
    constexpr uintptr_t FOTG210_OTGInterruptStatus  = 0x74;
    constexpr uintptr_t FOTG210_GlobalInterruptMask = 0xB4;

    // Command Register Bits
    enum class FOTG210_CommandRegisterBits : uint32_t {
        Run = 1 << 0,
        Reset = 1 << 1,
        PeriodicScheduleEnable = 1 << 4,
        AsyncScheduleEnable = 1 << 5,
        IAAD_Doorbell = 1 << 6,
        // park cnt bits idk
        Park = 1 << 11
    };
    constexpr uint32_t FOTG210_CommandRegister_ParkCntBits(uint32_t cnt) { return (cnt & 0x3) << 8; };
    constexpr uint32_t FOTG210_CommandRegister_GetParkCnt(uint32_t command) { return (command >> 8) & 0x3; };
    
    // Status Register Bits
    enum class FOTG210_StatusRegisterBits : uint32_t {
        USBInterrupt = 1 << 0,
        USBErrorInterrupt = 1 << 1,
        PortChangeDetect = 1 << 2,
        FrameListRollover = 1 << 3,
        HostSystemError = 1 << 4,
        InterruptOnAsyncAdvance = 1 << 5,
        HostControllerHalted = 1 << 12,
        Reclamation = 1 << 13,
        PeriodicScheduleStatus = 1 << 14,
        AsyncScheduleStatus = 1 << 15
    };

    // Interrupt Enable Bits
    enum class FOTG210_InterruptEnableBits : uint32_t {
        USBInterruptEnable = 1 << 0,
        USBErrorInterruptEnable = 1 << 1,
        PortChangeDetectEnable = 1 << 2,
        FrameListRolloverEnable = 1 << 3,
        HostSystemErrorEnable = 1 << 4,
        InterruptOnAsyncAdvanceEnable = 1 << 5
    };

    // Port Status/Control Bits
    enum class FOTG210_PortStatusControlBits : uint32_t {
        CurrentConnectStatus = 1 << 0,
        ConnectStatusChange = 1 << 1,
        PortEnabled = 1 << 2,
        PortEnabledChange = 1 << 3,
        OverCurrentActive = 1 << 4,
        OverCurrentChange = 1 << 5,
        ForcePortResume = 1 << 6,
        Suspend = 1 << 7,
        PortReset = 1 << 8,
        
        LineStatus_MASK = 3 << 10,
        PortPower = 1 << 12,
        PortOwner = 1 << 13,
        PortTestControl_MASK = 0xF << 16,
        WakeOnConnectEnable = 1 << 20,
        WakeOnDisconnectEnable = 1 << 21,
        WakeOnOverCurrentEnable = 1 << 22
    };

    // OTC Status/Control Bits
    enum class FOTG210_OTGStatusControlBits : uint32_t {
        A_Device_BusRequest = 1 << 4,
        A_Device_BusDrop = 1 << 5,
        HostSpeeedType_MASK = 3 << 22
    };
    constexpr uint32_t FOTG210_OTGStatusControl_HostSpeedType(uint32_t type) { return (type & 0x3) << 22; };
    constexpr uint32_t FOTG210_OTGStatusControl_GetHostSpeedType(uint32_t otgStatus) { return (otgStatus >> 22) & 0x3; };

    // OTG Interrupt Status Bits
    enum class FOTG210_OTGInterruptStatusBits : uint32_t {
        OverCurrent = 1 << 10
    };

    // Global Interrupt Mask Bits
    enum class FOTG210_GlobalInterruptMaskBits : uint32_t {
        MaskDeviceInterrupts = 1 << 0,
        MaskOTGInterrupts = 1 << 1,
        MaskHostControllerInterrupts = 1 << 2,
        InterruptPolarity = 1 << 3
    };

    /*
            ---- Data Structures ----
    */

    // -- Queue Transfer Descriptor --
    // Align 32 bytes!
    // EHCI Section 3.5
    constexpr size_t FOTG210_QTD_Alignment = 32;
    struct QueueTransferDescriptor {
        uint32_t nextQTDPointer;
        uint32_t alternateNextQTDPointer;
        uint32_t token;
        uint32_t bufferPagePointerList[5];
        uint32_t bufferPagePointerListHigh[5]; // for 64-bit addressing
    };

    // Token bits
    enum class QTD_TokenBits : uint32_t {
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

    // helpers
    constexpr uint32_t QTD_Length(uint32_t token) { return (token >> 16) & 0x7FFF; };
    constexpr uint32_t QTD_PID(uint32_t token) { return (token >> 8) & 0x3; };
    constexpr uint32_t QTD_Cerr(uint32_t token) { return (token >> 10) & 0x3; };
    enum class QTD_PIDCode : uint32_t {
        OUT = 0,
        IN = 1,
        SETUP = 2
    };

    // -- Queue Head --
    // Align 32 bytes!
    // EHCI Section 3.6
    constexpr size_t FOTG210_QueueHead_Alignment = 32;
    struct QueueHead {
        uint32_t horizontalLinkPointer;
        uint32_t info1_endpointCharacteristics;
        uint32_t info2_endpointCapabilities;
        uint32_t currentQTDPointer;
        // qtd overlay
        QueueTransferDescriptor overlayQTD;
    };

    // Endpoint Characteristics bits
    enum class QH_EndpointCharacteristicsBits : uint32_t {
        Inactive = 1 << 7,
        FullSpeed = 0 << 12, // 0??
        LowSpeed = 1 << 12,
        HighSpeed = 2 << 12,
        DataToggleControl = 1 << 14,
        HeadOfReclamationList = 1 << 15,
        ControlEndpointFlag = 1 << 27
    };

    // Endpoint Capabilities masks
    enum class QH_EndpointCapabilitiesMasks : uint32_t {
        InterruptScheduleMask = 0x000000FF,
        SplitCompletionMask = 0x0000FF00,
        HubAddressMask = 0x007F0000,
        HubPortMask = 0x3F800000,
        HighBandwidthMultiplierMask = 0xC0000000
    };

    // Isochronous Transfer Descriptor
    // Align 32 bytes!
    // EHCI Section 3.3
    constexpr size_t FOTG210_ITD_Alignment = 32;
    struct IsochronousTransferDescriptor {
        uint32_t nextLinkPointer;
        uint32_t transactionStatusAndControl[8];
        uint32_t bufferPagePointerList[7];
        uint32_t bufferPagePointerListHigh[7]; // for 64-bit addressing
    };

    // Transaction bits
    enum class ITD_TransactionBits : uint32_t {
        InterruptOnComplete = 1 << 15,
        Active = 1U << 31,
        BufferError = 1 << 30,
        BabbleDetected = 1 << 29,
        TransactionError = 1 << 28
    };

    constexpr uint32_t ITD_Length(uint32_t token) { return (token >> 16) & 0x0FFF; };
    
    // Periodic Frame Span Traversal Node
    // Align 32 bytes!
    // EHCI Section 3.7
    constexpr size_t FOTG210_FSTN_Alignment = 32;
    struct FrameSpanTraversalNode {
        uint32_t nextLinkPointer;
        uint32_t backPathLinkPointer;
    };

    enum class LinkPointerTypeTags : uint32_t {
        IsochronousTransferDescriptor = 0 << 1,
        QueueHead = 1 << 1,
        SplitTransactionIsochronousTransferDescriptor = 2 << 1,
        FrameSpanTraversalNode = 3 << 1
    };

}