#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_set>
#include <memory>
#include <bitset>
#include <bit>

#include "defs.hpp"
#include "ir/type.hpp"

namespace rulejit {

template <size_t ObjSize = sizeof(u64)>
struct alignas(PageSize) Page {
    static_assert((ObjSize % sizeof(u64)) == 0, "ObjSize should fit n*sizeof(u64)");
    struct alignas(ObjSize) Obj {
        u8 bytes[ObjSize];
        template <typename T>
        T& as() { return *std::launder(reinterpret_cast<T*>(bytes)); }
    } data[PageSize / ObjSize];
};

template <size_t ObjSize = sizeof(u64)>
struct FreeList {
    // static_assert((ObjSize & (ObjSize - 1)) == 0, "ObjSize should fit 2^n");
    struct Chunk {
        Page<ObjSize>::Obj* free;
        std::unique_ptr<Page<ObjSize>> storage;
        std::bitset<PageSize / ObjSize> allocated;
        Chunk() {}
    };
};

struct Memory {
    reg* allocHeap(ObjHeader h) {
        if ((h.flag & static_cast<u8>(ObjHeader::Flags::kHasFinalizer)) || 
            h.sizeCompressed == decltype(h.sizeCompressed)(-1) ||
            getSize(h) >= PageSize) {
                
            return allocMajor(h);
        }
        return allocMinor(h);
    }
    reg* allocStatic(ObjHeader h) {
        return nullptr;
    }

    /**
     * @brief manually register GC Root before GC, may need Stop The World
     * 
     * @param p pointer to one of GC Root, *p may edit during GC.
     */
    void registerGcRoot(reg** p) {
    }

    /**
     * @brief simplified GC can only called in single thread runtime.
     * may need called multi times
     * 
     * @return bool is finished
     */
    bool singleThreadGc() {
        if (stage == static_cast<u8>(Stage::kMinorGC)) {

        } else if (stage == static_cast<u8>(Stage::kMinorGC)) {

        }
        return false;
    }
    /**
     * @brief concurrent GC can called through main thread running.
     * may need called multi times
     * 
     * @return bool is finished
     */
    bool concurrentGc() { return false; };

    /**
     * @brief write pointer from src to dst with write barrier
     * write barrier:
     * 1. mark card table
     * 2. TODO: change color
     * 
     * @param dst pointer to destination field, may get through &(xxx.as<reg*>())
     * @param src value need write
     */
    void writeWithBarrier(reg** dst, reg* src) {
        if (src == nullptr) {
            *dst = src;
            return;
        }
        if (getMemType(usize(src)) == MemType::kMinor) {
            auto dstMemType = getMemType(usize(dst));
            // TODO: dstMemType & (m | s | h)?
            if (dstMemType == MemType::kMajor || dstMemType == MemType::kStatic || dstMemType == MemType::kHuge) {
                postWrite(dst, src);
            }
        }
        *dst = src;
    }

    /**
     * @brief read pointer with read barrier
     * read barrier:
     * 1. if *src is moved, change *src to **src and return *src
     * 
     * @param src 
     * @return reg* 
     */
    reg* readWithBarrier(reg** src) {
        if (stage != static_cast<u8>(Stage::kMinorGC)) {
            return *src;
        }
        ObjHeader h = getHeader(*src);
        if (h.flag & static_cast<u8>(ObjHeader::Flags::kIsMoved)) {
            *src = (**src).as<reg*>();
        }
        return *src;
    }
private:

    std::unordered_set<usize> cardTable;
    /**
     * @brief post write event to mark
     * 
     */
    void postWrite(reg** dst, reg* src) {
        cardTable.emplace(usize(dst));
    }

    enum class Stage {
        kNormal = 1,
        kMinorGC = 2,
        kMajorGC = 4,
    };
    u8 stage = static_cast<u8>(Stage::kNormal);

    enum class MemType {
        kUnmanaged = 1,
        kMinor = 2,
        kMajor = 4,
        kHuge = 8,
        kStatic = 16,
    };
    struct PageInfo {
        MemType type;
    };
    // TODO: usize(ptr) / PageSize?
    std::unordered_map<usize, PageInfo> pageMemType;
    // obj ptr -> info
    std::map<reg*, PageInfo, std::greater<reg*>> hugeMemType;

    TypeManager* tm;
    std::deque<reg**> root;

    static ObjHeader getHeader(reg* objPtr) {
        return objPtr[-1].as<ObjHeader>();
    }
    static usize getPageId(usize ptr) {
        constexpr usize mask = !(PageSize - 1);
        return mask & ptr;
    }

    usize getSize(ObjHeader h) {
        if (h.sizeCompressed == decltype(h.sizeCompressed)(-1)) [[unlikely]] {
            return tm->getType(h.typeId);
        }
        return h.sizeCompressed * sizeof(u64);
    }
    /**
     * @brief get MemType of ptr pointed
     * 
     * @param ptr to managed memory
     * @return MemType of memory which ptr pointed
     */
    MemType getMemType(usize ptr) {
        if (auto it = pageMemType.find(getPageId(ptr)); it != pageMemType.end()) {
            return it->second.type;
        }
        if (auto it = hugeMemType.lower_bound(std::bit_cast<reg*>(ptr)); it != hugeMemType.end()) [[likely]] {
            // TODO: test, lower_bound?
            auto size = getSize((it->first)[-1].as<ObjHeader>());
            if (size + usize(it->first) > ptr) {
                return MemType::kHuge;
            }
        }
        return MemType::kUnmanaged;
    }

    reg* allocMinor(ObjHeader h) {
        return nullptr;
    }
    reg* allocMajor(ObjHeader h) {
        return nullptr;
    }
};

}