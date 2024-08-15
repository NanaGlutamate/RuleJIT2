#pragma once

#include <cstddef>
#include <deque>
#include <unordered_set>
#include <vector>
#include <memory>
#include <bitset>

#include "defs.hpp"
#include "ir/type.hpp"

namespace rulejit {

template <size_t ObjSize>
struct FreeList {
    // static_assert((ObjSize & (ObjSize - 1)) == 0, "ObjSize should fit 2^n");
    static_assert((ObjSize % sizeof(u64)) == 0, "ObjSize should fit n*sizeof(u64)");
    struct alignas(PageSize) Page {
        struct alignas(ObjSize) Obj {
            u8 bytes[ObjSize];
            template <typename T>
            T& as() { return *reinterpret_cast<T*>(bytes); }
        } data[PageSize / ObjSize];
    };
    struct Chunk {
        Page::Obj* free;
        std::unique_ptr<Page> storage;
        std::bitset<PageSize / ObjSize> allocated;

        Chunk() {}
    };
};

struct Memory {
    reg* allocHeap(ObjHeader h) {
        bool hasFinalizer = h.flag & static_cast<u8>(ObjHeader::Flags::kHasFinalizer);
        bool oversized = (h.size == decltype(h.size)(-1)) || (getSize(h) > getMinorRemain());
        if (hasFinalizer || oversized) {
            return allocMajor(h);
        }
        return allocMinor(h);
    }
    reg* allocStatic(ObjHeader h) {
        return nullptr;
    }
    void recordGcRoot(reg** p) {
    }
    void gc() {}

    /**
     * @brief write pointer from src to dst with write barrier
     * write barrier:
     * 1. mark card table
     * 
     * @param dst destination
     * @param src source value
     */
    void writeWithBarrier(reg** dst, reg* src) {
        if (src == nullptr) {
            *dst = src;
            return;
        }
        if (auto srcId = getPageId(usize(src)); getMemType(srcId) == MemType::kMinor) {
            auto dstId = getPageId(usize(dst));
            auto dstMemType = getMemType(dstId);
            if (dstMemType == MemType::kMajor || dstMemType == MemType::kStatic) {
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
        if (!(stage & static_cast<u8>(Stage::kMinorGC))) {
            return *src;
        }
        ObjHeader h = getHeader(*src);
        if (h.flag & static_cast<u8>(ObjHeader::Flags::kIsMoved)) {
            *src = (**src).as<reg*>();
        }
        return *src;
    }

    /**
     * @brief concurrent GC can called through main thread running.
     * may need called multi times
     * 
     * @return bool is finished
     */
    bool concurrentGc() { return false; };
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
        kFullGC = 6,
    };
    u8 stage = static_cast<u8>(Stage::kNormal);

    enum class MemType {
        kUnmanaged,
        kMinor,
        kMajor,
        kStatic,
    };
    struct PageInfo {
        MemType type;
    };
    std::unordered_map<usize, PageInfo> memType;

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
        if (h.size == decltype(h.size)(-1)) [[unlikely]] {
            return tm->getType(h.typeId);
        }
        return h.size;
    }
    MemType getMemType(usize pageId) {
        auto it = memType.find(pageId);
        if (it == memType.end()) {
            return MemType::kUnmanaged;
        }
        return it->second.type;
    }

    usize getMinorRemain() {
        // TODO:
        return 0;
    }
    reg* allocMinor(ObjHeader h) {
        return nullptr;
    }
    reg* allocMajor(ObjHeader h) {
        return nullptr;
    }
};

}