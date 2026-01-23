#include <nspire-utils/devices/usb/FOTG210.hpp>

#include <nspire-utils/mem/Alloc.hpp>

namespace ntls::devices::usb {

    FOTG210_MemoryStructures::~FOTG210_MemoryStructures() {
        if (frameList) {
            mem::aligned_free(frameList);
            frameList = nullptr;
        }
        if (asyncScheduleQueueHead) {
            mem::aligned_free(asyncScheduleQueueHead);
            asyncScheduleQueueHead = nullptr;
        }
    }

    void FOTG210_MemoryStructures::allocate() {
        // allocate frame list
        if(!frameList) {
            frameList = reinterpret_cast<uint32_t*>(mem::aligned_alloc(FOTG210_FrameList_Alignment, FOTG210_FrameList_Entries * sizeof(uint32_t)));
        }
        // initialize
        for (size_t i = 0; i < FOTG210_FrameList_Entries; i++) {
            frameList[i] = 1; // Terminate bit set
        }

        // allocate async schedule queue head
        if (!asyncScheduleQueueHead) {
            asyncScheduleQueueHead = reinterpret_cast<QueueHead*>(mem::aligned_alloc(FOTG210_QTD_Alignment, 32));
        }
    
        asyncScheduleQueueHead->horizontalLinkPointer = 
            (reinterpret_cast<uintptr_t>(asyncScheduleQueueHead) & ~0x1F) | // Address (32-byte aligned)
            static_cast<uint32_t>(LinkPointerTypeTags::QueueHead);  // Q_TYPE_QH = 0x2
        asyncScheduleQueueHead->info1_endpointCharacteristics = 
            static_cast<uint32_t>(QH_EndpointCharacteristicsBits::HeadOfReclamationList);
        asyncScheduleQueueHead->info2_endpointCapabilities = 0;
        asyncScheduleQueueHead->currentQTDPointer = 0;
        // qtd overlay
        asyncScheduleQueueHead->overlayQTD.nextQTDPointer = 1; // terminate bit set
        asyncScheduleQueueHead->overlayQTD.alternateNextQTDPointer = 1; // terminate
        asyncScheduleQueueHead->overlayQTD.token = static_cast<uint32_t>(QTD_TokenBits::Halted);
        for (size_t i = 0; i < 5; i++) {
            asyncScheduleQueueHead->overlayQTD.bufferPagePointerList[i] = 0;
            asyncScheduleQueueHead->overlayQTD.bufferPagePointerListHigh[i] = 0;
        }
    }

    FOTG210::FOTG210(uintptr_t baseAddr)
        : baseAddress(baseAddr)
    {
        // Read Capability Length
        capLength = *reinterpret_cast<volatile uint8_t*>(baseAddress + FOTG210_CapLength);
    }

} // namespace ntls::devices::usb