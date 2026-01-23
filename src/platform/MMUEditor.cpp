#include <nspire-utils/platform/MMUEditor.hpp>

#include <nspire-utils/mem/Alloc.hpp>
#include <nspire-utils/platform/Caches.hpp>

#include <stdexcept>
#include <exception>
#include <cstring>

namespace ntls::platform {

    uint32_t MMUEditor::get_ttbr0() {
        uint32_t ttbr0;
        asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r" (ttbr0));
        return ttbr0;
    }

    void MMUEditor::set_ttbr0(uint32_t ttbr0) {
        asm volatile ("mcr p15, 0, %0, c2, c0, 0" :: "r" (ttbr0));
    }

    uint32_t MMUEditor::L1Index(uintptr_t addr) {
        return (addr >> 20); // 1 MB sections
    }

    MMUEditor::MMUEditor() {
        // capture original TTBR0
        original_ttbr0_reg = get_ttbr0();

        // get table physical address
        uintptr_t old_table_addr = original_ttbr0_reg & 0xFFFFC000;
        
        // WARNING: aligned_alloc might not return a physical or identity mapped address
        // on the ti-nspire, the sdram is identity mapped, and is fine
        editedTable = static_cast<uint32_t*>(mem::aligned_alloc(TTB_Alignment, TTB_Size));
        if(editedTable == nullptr) {
            throw std::bad_alloc();
        }
        
        // copy existing table
        // WARNING: assumes identity mapped address, mmu relies solely on physical addresses
        std::memcpy(editedTable, reinterpret_cast<void*>(old_table_addr), TTB_Size);

        // flush dcache to ensure MMU reads updated table
        FlushDataCacheRange(
            reinterpret_cast<uintptr_t>(editedTable), 
            reinterpret_cast<uintptr_t>(editedTable) + TTB_Size
        );

        // set new TTBR0
        uint32_t old_flags = original_ttbr0_reg & ~0xFFFFC000;
        uint32_t new_ttbr0 = (reinterpret_cast<uint32_t>(editedTable)) | old_flags;
        set_ttbr0(new_ttbr0);
        InvalidateTLB();
    }

    MMUEditor::~MMUEditor() {
        if(editedTable) {
            // restore original TTBR0
            set_ttbr0(original_ttbr0_reg);
            InvalidateTLB();

            mem::aligned_free(editedTable);
            editedTable = nullptr;
        }
    }

    void MMUEditor::map(uintptr_t virtual_addr, uint32_t tableDescriptorEntry) {
        if(editedTable == nullptr) return;

        uint32_t index = L1Index(virtual_addr);
        editedTable[index] = tableDescriptorEntry;

        uintptr_t entry_addr = reinterpret_cast<uintptr_t>(&editedTable[index]);
        FlushDataCacheRange(entry_addr, entry_addr + sizeof(uint32_t));
        InvalidateTLB();
    }

    void MMUEditor::MapSection(uintptr_t virtual_addr, uintptr_t physical_addr, uint32_t attributes) {
        if(editedTable == nullptr) return;

        uint32_t index = MMUEditor::L1Index(virtual_addr);
        uint32_t descriptor = (physical_addr & 0xFFF00000) | attributes | static_cast<uint32_t>(MMUDescriptorType::Section);

        // bit 4 must be 1 for section entries
        descriptor |= (1 << 4);

        editedTable[index] = descriptor;

        uintptr_t entry_addr = reinterpret_cast<uintptr_t>(&editedTable[index]);
        FlushDataCacheRange(entry_addr, entry_addr + sizeof(uint32_t));
        InvalidateTLB();
    }

    uint32_t MMUEditor::getEntry(uintptr_t virtual_addr) {
        if(editedTable == nullptr) {
            return 0xFFFFFFFF;
        }

        uint32_t index = L1Index(virtual_addr);
        return editedTable[index];
    }

    // returns first index for 64KB large page
    uint32_t MMUL2CoarsePageTableEditor::L2LargePageIndex(uintptr_t addr) {
        // use small page index and mask lower bits
        return MMUL2CoarsePageTableEditor::L2SmallPageIndex(addr) & 0xF0; // 64KB pages
    }

    uint32_t MMUL2CoarsePageTableEditor::L2SmallPageIndex(uintptr_t addr) {
        // 256 entries, 4KB each
        return (addr >> 12) & 0xFF; // 4KB pages
    }

    MMUL2CoarsePageTableEditor::MMUL2CoarsePageTableEditor(MMUEditor& mmuEditor, uintptr_t virtual_addr, uint8_t domain)
        : parentMMUEditor(mmuEditor), virtualAddr(virtual_addr)
    {
        // get L1 entry
        uint32_t l1_entry = parentMMUEditor.getEntry(virtual_addr);
        this->oldL1Entry = l1_entry;
        
        // if fine page table, cannot proceed
        // this is because there would be no way to accurately copy the higher precision table in a coarse page table
        MMUDescriptorType desc_type = static_cast<MMUDescriptorType>(l1_entry & 0b11);
        if (desc_type == MMUDescriptorType::FinePageTable) {
            throw std::runtime_error("Cannot copy fine page table");
        }

        // allocate new L2 table
        this->l2Table = static_cast<uint32_t*>(mem::aligned_alloc(L2Table_Alignment, L2Table_Size));
        if(l2Table == nullptr) {
            throw std::bad_alloc();
        }

        // if coarse page table, copy existing table
        // if section, need to replace with a coarse page table that maps the same region
        // if fault, fill table with faults
        if(desc_type == MMUDescriptorType::CoarsePageTable) {
            uintptr_t l2_table_addr = l1_entry & 0xFFFFFC00;
            std::memcpy(l2Table, reinterpret_cast<void*>(l2_table_addr), L2Table_Size);

        } else if (desc_type == MMUDescriptorType::Section) {
            // fill in entries to map same region as section
            uintptr_t sectionBasePhysAddr = l1_entry & 0xFFF00000;
            MMUCachePolicy sectionCachePolicy = MMUCachePolicyFromEntry(l1_entry);
            MMUAccessPermission sectionAccessPermission = MMUAccessPermissionFromSectionEntry(l1_entry);
            for(size_t i = 0; i < 256; ++i) {
                uintptr_t page_phys_addr = sectionBasePhysAddr + (i * 0x1000); // 4KB pages
                l2Table[i] = 
                    (page_phys_addr & 0xFFFFF000) | 
                    static_cast<uint32_t>(MMUL2DescriptorType::SmallPage) | 
                    MMUL2EntirePageAccessPermissions(sectionAccessPermission) |
                    static_cast<uint32_t>(sectionCachePolicy);
            }

        } else {
            // fill with faults
            std::memset(l2Table, 0, L2Table_Size); // all entries fault
        }

        // replace L1 entry with new coarse page table
        uintptr_t l2_table_phys_addr = reinterpret_cast<uintptr_t>(l2Table);
        uint32_t new_l1_entry =
            (l2_table_phys_addr & 0xFFFFFC00) |
            (static_cast<uint32_t>(MMUDescriptorType::CoarsePageTable)) |
            MMUSelectDomain(domain) |
            (1 << 4) // bit 4 must be 1 for coarse page table entries
            ;

        // flush dcache for new L2 table
        FlushDataCacheRange(
            reinterpret_cast<uintptr_t>(l2Table), 
            reinterpret_cast<uintptr_t>(l2Table) + L2Table_Size
        );

        // write change to main translation table
        parentMMUEditor.map(virtual_addr, new_l1_entry);

    }
    MMUL2CoarsePageTableEditor::~MMUL2CoarsePageTableEditor() {
        if(l2Table) {
            // restore old L1 entry
            parentMMUEditor.map(virtualAddr, oldL1Entry);

            mem::aligned_free(l2Table);
            l2Table = nullptr;
        }
    }

    void MMUL2CoarsePageTableEditor::Map64KBPage(uintptr_t virtual_addr, uintptr_t physical_addr, uint32_t attributes) {
        if(l2Table == nullptr) return;

        uint32_t index = L2LargePageIndex(virtual_addr);
        uint32_t newDescriptor = 
            (physical_addr & 0xFFFF0000) | 
            static_cast<uint32_t>(MMUL2DescriptorType::LargePage) |
            attributes;

        for(size_t i = 0; i < 16; ++i) { // 16 small pages in 64KB large page
            l2Table[index + i] = newDescriptor;
        }

        uintptr_t entry_addr = reinterpret_cast<uintptr_t>(&l2Table[index]);
        FlushDataCacheRange(entry_addr, entry_addr + (16 * sizeof(uint32_t)));
        InvalidateTLB();
    }

    void MMUL2CoarsePageTableEditor::Map4KBPage(uintptr_t virtual_addr, uintptr_t physical_addr, uint32_t attributes) {
        if(l2Table == nullptr) return;

        uint32_t index = L2SmallPageIndex(virtual_addr);
        uint32_t newDescriptor = 
            (physical_addr & 0xFFFFF000) | 
            static_cast<uint32_t>(MMUL2DescriptorType::SmallPage) |
            attributes;

        l2Table[index] = newDescriptor;

        uintptr_t entry_addr = reinterpret_cast<uintptr_t>(&l2Table[index]);
        FlushDataCacheRange(entry_addr, entry_addr + sizeof(uint32_t));
        InvalidateTLB();
    }

} // namespace ntls