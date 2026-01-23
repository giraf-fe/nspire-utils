#pragma once

#include <cstdint>
#include <cstddef>

#include <tuple>

namespace ntls::platform {

    constexpr size_t TTB_Size = 16384; // 16 KB
    constexpr size_t TTB_Alignment = 16384; // 16 KB

    constexpr size_t L2Table_Size = 1024; // 1 KB
    constexpr size_t L2Table_Alignment = 1024; // 1 KB

    // Get the currently mapped address in an entry
    inline static uintptr_t MMUL1EntryMappedAddress(uint32_t entry) {
        return entry & 0xFFF00000;
    }

    // MMU Access Permission bits, [11:10] on section entry
    enum class MMUAccessPermission : uint32_t {
        NoAccess = 0b00 << 10,
        PrivilegedOnly = 0b01 << 10,
        UserReadOnly = 0b10 << 10,
        FullAccess = 0b11 << 10
    };

    // Subpage access permissions for L2 page table entries, bits [11:4]
    inline static uint32_t MMUL2SubpageAccessPermissions(
        MMUAccessPermission firstSubPage, 
        MMUAccessPermission secondSubPage, 
        MMUAccessPermission thirdSubPage, 
        MMUAccessPermission fourthSubPage
    ) {
        return  (static_cast<uint32_t>(firstSubPage) >> 6) |
                (static_cast<uint32_t>(secondSubPage) >> 4) |
                (static_cast<uint32_t>(thirdSubPage) >> 2) |
                (static_cast<uint32_t>(fourthSubPage));
    }
    // Helper for setting all 4 subpages to the same access permission
    inline static uint32_t MMUL2EntirePageAccessPermissions(
        MMUAccessPermission ap
    ) {
        return MMUL2SubpageAccessPermissions(ap, ap, ap, ap);
    }

    // Get access permission type for a table entry
    inline static MMUAccessPermission MMUAccessPermissionFromSectionEntry(uint32_t entry) {
        return static_cast<MMUAccessPermission>(entry & (0b11 << 10));
    }
    inline static std::tuple<MMUAccessPermission, MMUAccessPermission, MMUAccessPermission, MMUAccessPermission>
    MMUAccessPermissionsFromL2PageEntry(uint32_t entry) {
        MMUAccessPermission firstSubPage = static_cast<MMUAccessPermission>((entry & (0b11 << 4)) << 6);
        MMUAccessPermission secondSubPage = static_cast<MMUAccessPermission>((entry & (0b11 << 6)) << 4);
        MMUAccessPermission thirdSubPage = static_cast<MMUAccessPermission>((entry & (0b11 << 8)) << 2);
        MMUAccessPermission fourthSubPage = static_cast<MMUAccessPermission>(entry & (0b11 << 10));
        return std::tuple(firstSubPage, secondSubPage, thirdSubPage, fourthSubPage);    
    }

    // Cache policy bits [3:2]
    enum class MMUCachePolicy : uint32_t {
        NonCachedNonBuffered = 0b00 << 2, // for MMIO
        NonCachedWriteBuffered = 0b01 << 2, // writes buffered, reads not cached
        WriteThroughCache = 0b10 << 2, // reads cached, writes go to both cache and memory immediately
        WriteBackCache = 0b11 << 2 // best performance, reads and writes cached
    };
    inline static MMUCachePolicy MMUCachePolicyFromEntry(uint32_t entry) {
        return static_cast<MMUCachePolicy>(entry & (0b11 << 2));
    }

    // Domain number shifted into position, bits [8:5]
    // Valid domain numbers are 0-15
    inline static uint32_t MMUSelectDomain(uint8_t domain) {
        return (static_cast<uint32_t>(domain) & 0xF) << 5;
    };
    inline static uint8_t MMUDomainFromEntry(uint32_t entry) {
        return static_cast<uint8_t>((entry >> 5) & 0xF);
    };
    
    // Entry descriptor types, bits [1:0]
    enum class MMUDescriptorType : uint32_t {
        Fault = 0b00,
        CoarsePageTable = 0b01,
        Section = 0b10,
        FinePageTable = 0b11
    };
    inline static MMUDescriptorType MMUDescriptorTypeFromEntry(uint32_t entry) {
        return static_cast<MMUDescriptorType>(entry & 0b11);
    };

    enum class MMUL2DescriptorType : uint32_t {
        Fault = 0b00,
        LargePage = 0b01,
        SmallPage = 0b10,
        TinyPage = 0b11
    };
    inline static MMUL2DescriptorType MMUL2DescriptorTypeFromEntry(uint32_t entry) {
        return static_cast<MMUL2DescriptorType>(entry & 0b11);
    };

    // Copies the existing MMU configuration and allows editing it
    // for the duration that the MMUEditor object exists.
    class MMUEditor {
        uint32_t original_ttbr0_reg;
        uint32_t* editedTable = nullptr;

        static uint32_t get_ttbr0();
        static void set_ttbr0(uint32_t ttbr0);

        static uint32_t L1Index(uintptr_t addr);

    public:
        // May throw std::bad_alloc if edited table allocation fails.
        MMUEditor();
        ~MMUEditor();

        // virtual_addr: The address the code tries to access, used to calculate first level table index (e.g., 0xA4000000 for the sram)
        // tableDescriptorEntry: The descriptor entry value. Meant to be used with helpers.
        void map(uintptr_t virtual_addr, uint32_t tableDescriptorEntry);

        // For 1MB section mappings,
        // attributes should include: MMUAccessPermission, MMUCachePolicy, domain
        void MapSection(uintptr_t virtual_addr, uintptr_t physical_addr, uint32_t attributes);

        // gets the current L1 entry for the given virtual address
        // returns 0xFFFFFFFF if the edited table is not available
        uint32_t getEntry(uintptr_t virtual_addr);
    };

    // Copies out and edits a Level 2 Coarse Page Table for the
    // duration that the MMUL2CoarsePageTableEditor object exists.
    class MMUL2CoarsePageTableEditor {
        MMUEditor& parentMMUEditor;
        uint32_t oldL1Entry;
        uintptr_t virtualAddr;

        uint32_t* l2Table = nullptr;

        static uint32_t L2LargePageIndex(uintptr_t addr);
        static uint32_t L2SmallPageIndex(uintptr_t addr);
    public:
        // throws std::bad_alloc if l2 table allocation fails
        // throws std::runtime_error if existing L1 entry is a fine page table
        // virtual_addr determines the l1 index the new l2 table is mapped to, should be 1MB aligned
        MMUL2CoarsePageTableEditor(MMUEditor& mmuEditor, uintptr_t virtual_addr, uint8_t domain);    
        ~MMUL2CoarsePageTableEditor();

        // addrs must be 64KB aligned, will silently ignore lower bits
        // attributes should include: MMUL2SubpageAccessPermissions and MMUCachePolicy
        void Map64KBPage(uintptr_t virtual_addr, uintptr_t physical_addr, uint32_t attributes);

        // addrs must be 4KB aligned, will silently ignore lower bits
        // attributes should include: MMUL2SubpageAccessPermissions and MMUCachePolicy
        void Map4KBPage(uintptr_t virtual_addr, uintptr_t physical_addr, uint32_t attributes);
    };
        
} // namespace ntls
