// EPOS-- ARMv7 MMU Mediator Declarations

#ifndef __riscv_mmu_h
#define __riscv_mmu_h

#include <system/memory_map.h>
#include <utility/string.h>
#include <utility/list.h>
#include <utility/debug.h>
#include <architecture/cpu.h>
#include <architecture/mmu.h>

__BEGIN_SYS

class MMU: public MMU_Common<0, 0, 0>
{
    friend class CPU;

private:
    typedef Grouping_List<unsigned int> List;

    static const unsigned int PHY_MEM = Memory_Map::PHY_MEM;

public:
    // Page Flags
    typedef MMU_Common<0, 0, 0>::Flags RISCV_Flags;

    // Page_Table
    class Page_Table {};

    // Chunk (for Segment)
    class Chunk
    {
    public:
        Chunk() {}
        Chunk(unsigned int bytes, Flags flags): _phy_addr(alloc(bytes)), _bytes(bytes), _flags(flags) {}
        Chunk(Phy_Addr phy_addr, unsigned int bytes, Flags flags): _phy_addr(phy_addr), _bytes(bytes), _flags(flags) {}

        ~Chunk() { free(_phy_addr, _bytes); }

        unsigned int pts() const { return 0; }
        Flags flags() const { return _flags; }
        Page_Table * pt() const { return 0; }
        unsigned int size() const { return _bytes; }
        Phy_Addr phy_address() const { return _phy_addr; } // always CT
        int resize(unsigned int amount) { return 0; } // no resize in CT

    private:
        Phy_Addr _phy_addr;
        unsigned int _bytes;
        RISCV_Flags _flags;
    };

    // Page Directory
    typedef Page_Table Page_Directory;

    // Directory (for Address_Space)
    class Directory
    {
    public:
        Directory() {}
        Directory(Page_Directory * pd) {}

        Page_Table * pd() const { return 0; }

        void activate() {}

        Log_Addr attach(const Chunk & chunk) { return chunk.phy_address(); }
        Log_Addr attach(const Chunk & chunk, Log_Addr addr) { return (addr == chunk.phy_address())? addr : Log_Addr(false); }
        void detach(const Chunk & chunk) {}
        void detach(const Chunk & chunk, Log_Addr addr) {}

        Phy_Addr physical(Log_Addr addr) { return addr; }
    };

    // DMA_Buffer (straightforward without paging)
    class DMA_Buffer: public Chunk
    {
    public:
        DMA_Buffer(unsigned int s): Chunk(s, Flags::CT){};
    };

public:
    MMU() {}

    static Phy_Addr alloc(unsigned int bytes = 1);

    static Phy_Addr calloc(unsigned int bytes = 1);

    static void free(Phy_Addr addr, unsigned int n = 1);

    static unsigned int allocable() { return _free.head() ? _free.head()->size() : 0; }

    static Page_Directory * volatile current() { return 0; }

    static Phy_Addr physical(Log_Addr addr) { return addr; }

    static void flush_tlb() {}
    static void flush_tlb(Log_Addr addr) {}

private:
    static void init();

private:
    static List _free;
};

__END_SYS

#endif
