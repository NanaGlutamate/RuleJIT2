#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <deque>
#include <functional>
#include <new>
#include <unordered_set>
#include <memory>
#include <bitset>
#include <bit>

#include "defs.hpp"
#include "ir/type.hpp"

namespace rulejit {

template <size_t ObjSize = sizeof(u64)>
struct alignas(PageSize) Page {
    static_assert(ObjSize % sizeof(u64) == 0);
    static_assert(ObjSize != 0);
    static_assert(PageSize / ObjSize > 2);
    
    struct alignas(ObjSize) Obj {
        u8 bytes[ObjSize];
        template <typename T>
        T& as() { return *std::launder(reinterpret_cast<T*>(bytes)); }
    } data[PageSize / ObjSize];

    /**
     * @brief init data as a free list; first 'offset' object is reserved.
     * 
     * @param offset 
     * @return void* pointer to first chunk
     */
    void* initFreeList(usize offset = 1) {
        // TODO:
        void* tmp = nullptr;
        for (usize i = offset; i < PageSize / ObjSize; ++i) {
            data[i].template as<void*>() = tmp;
            tmp = reinterpret_cast<void*>(&data[i]);
        }
        return tmp;
    }
};

struct MinorPageInfo {
    bool isMinorGcTarget;
};

// template <size_t ObjSize = sizeof(u64)>
// struct FreeList {
//     // static_assert((ObjSize & (ObjSize - 1)) == 0, "ObjSize should fit 2^n");
//     struct Chunk {
//         Page<ObjSize>::Obj* free;
//         std::unique_ptr<Page<ObjSize>> storage;
//         std::bitset<PageSize / ObjSize> allocated;
//         Chunk() {}
//     };
// };

namespace helper {

ObjHeader& getHeader(reg* objPtr) {
    return objPtr[-1].as<ObjHeader>();
}

usize getPageId(usize ptr) {
    constexpr usize mask = !(PageSize - 1);
    return mask & ptr;
}

}

/**
 * @brief GC
 *
 * life cycle:
 * 
 * -----------
 * |         |<-------------
 * |  Normal |             | if has enough memory
 * |         |-allocHeap()-- 
 * -----------             | if minor storage is nearly full
 *                         | changeState();
 *                         | rootScanner(); // should call registerGcRoot();
 *                              
 *
 *
 * -----------
 * |         |
 * | MinorGC |TODO:
 * |         |
 * -----------
 * 
 * -----------
 * |         |
 * | MajorGC |
 * |         |
 * -----------
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */
struct Memory {
    /**
     * @brief alloc object on heap
     * 
     * @param t
     * @return reg* 
     */
    reg* allocHeap(TypeToken t) {
        ObjHeader h = tm->getType(t).headerPrototype();
        if (h.hasFlag(ObjHeader::Flags::kHasFinalizer) || 
            h.isBigObject() ||
            getSize(h) >= PageSize) {
                
            return allocMajor(h);
        }
        return allocMinor(h);
    }

    reg* allocStatic(ObjHeader h) {
        return nullptr;
    }

    /**
     * @brief register a function to scan GC Root. automatically called
     * on begin of GC
     * 
     * @param f 
     */
    void registerGcRootScanner(std::move_only_function<void()> f) {
        rootScanner = std::move(f);
    }

    /**
     * @brief manually register GC Root before GC, may need Stop The World
     * 
     * @param p pointer to one of GC Root, *p may edit during GC.
     */
    void registerGcRoot(reg** p) {
        root.emplace_back(p);
    }

    /**
     * @brief simplified GC can only called in single thread runtime.
     * may need called multi times
     * must called after registerGcRoot() finished
     * 
     * @return bool is finished
     */
    bool singleThreadGcStep() {
        if (stage == static_cast<u8>(Stage::kMinorGC)) {
            minorGcStep();
            return minorGcFinished();
        } else if (stage == static_cast<u8>(Stage::kMajorGC)) {

            return false;
        }
        return true;
    }

    /**
     * @brief concurrent GC can called through main thread running.
     * may need called multi times
     * 
     * @return bool is finished
     */
    bool concurrentGcStep() { return false; };

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
        if (!isMemTypeMinor(usize(dst))) {
            if (isMemTypeMinor(usize(src))) {
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
        ObjHeader h = helper::getHeader(*src);
        if (h.hasFlag(ObjHeader::Flags::kIsMoved)) {
            *src = (**src).as<reg*>();
        }
        return *src;
    }
private:

    /**
     * @brief hold pointers to storage may pointed Minor object
     * 
     */
    std::unordered_set<reg**> cardTable;
    std::deque<reg**> markQueue;
    bool minorGcFinished() {
        // size of card table is stable, call cardTable.empty() before markStack.empty() to make branch prediction happy. TODO: test
        return cardTable.empty() && markQueue.empty();
    }
    void minorGcStep() {
        reg** now;
        if (!markQueue.empty()) {
            now = markQueue.front();
            markQueue.pop_front();
        }
        assert(!cardTable.empty());
        now = *cardTable.begin();
        cardTable.erase(cardTable.begin());

        // 3 cases for abort mark
        if (!isMemTypeMinor(usize(*now))) {
            // 1. not pointed a minor object (changed after mark)
            return;
        }
        if (helper::getHeader(*now).hasFlag(ObjHeader::Flags::kIsMoved)) {
            // 2. target object already moved and this pointer is not modified
            *now = (**now).as<reg*>();
            return;
        }
        if (getMinorPageInfo(helper::getPageId(usize(*now))).isMinorGcTarget) {
            // 3. target object already moved and this pointer already modified
            return;
        }

        *now = moveMinor(*now);
        callWithPointerMember(*now, [this](reg** tar){
            markQueue.push_back(tar);
        });
    }

    /**
     * @brief post write event to mark
     * TODO: run on another thread
     * 
     */
    void postWrite(reg** dst, reg* src) {
        cardTable.emplace(dst);
    }
    // void postWriteCancelled(reg** dst) {
    //     cardTable.erase(reinterpret_cast<reg*>(dst));
    // }

    enum class Stage {
        kNormal = 1,
        kMinorGC = 2,
        kMajorGC = 4,
    };
    u8 stage = static_cast<u8>(Stage::kNormal);

    enum class MemType {
        kUnmanaged,
        kMinor,
        kMajor,
        kHuge,
        kStatic,
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
    std::move_only_function<void()> rootScanner;

    usize getSize(ObjHeader h) {
        if (!h.isBigObject()) {
            return h.sizeCompressed * sizeof(u64);
        }
        return tm->getType(h.typeId).objSize();
    }
    /**
     * @brief get MemType of ptr pointed
     * 
     * @param ptr to managed memory
     * @return MemType of memory which ptr pointed
     */
    MemType getMemType(usize ptr) {
        if (auto it = pageMemType.find(helper::getPageId(ptr)); it != pageMemType.end()) {
            return it->second.type;
        }
        if (auto it = hugeMemType.lower_bound(std::bit_cast<reg*>(ptr)); it != hugeMemType.end()) [[likely]] {
            // TODO: test, lower_bound?
            auto size = getSize((it->first)[-1].as<ObjHeader>());
            if (size + usize(it->first) > ptr) {
                return MemType::kHuge;
            }
        }
        assert(false);
        return MemType::kUnmanaged;
    }
    bool isMemTypeMinor(usize ptr) {
        // return helper::getHeader(reinterpret_cast<reg*>(ptr)).hasFlag(ObjHeader::Flags::kMinorObject);
        if (auto it = pageMemType.find(helper::getPageId(ptr)); it != pageMemType.end()) {
            return it->second.type == MemType::kMinor;
        }
        return false;
    }
    MinorPageInfo& getMinorPageInfo(usize pageId) {
        return *std::launder(reinterpret_cast<MinorPageInfo*>(pageId));
    }

    /**
     * @brief move minor object to new storage.
     * SHOULDNOT called on moved object
     * 
     * @param objPtr 
     * @return pointer to new object
     */
    reg* moveMinor(reg* objPtr) {
        auto h = helper::getHeader(objPtr);
        assert(!h.hasFlag(ObjHeader::Flags::kIsMoved));
        assert(h.hasFlag(ObjHeader::Flags::kIsMinorObject));

        reg *p;
        // TODO: extract config
        if (h.ageOrColor >= 5) {
            p = allocMajor(h);
            helper::getHeader(p).setColor(ObjHeader::Color::kWhite);
        } else {
            p = allocMinor(h);
            helper::getHeader(p).ageOrColor++;
        }

        auto size = getSize(h);
        for (usize i = 0; i < size; ++i) {
            p[i] = objPtr[i];
        }

        objPtr[-1].as<ObjHeader>().addFlag(ObjHeader::Flags::kIsMoved);
        objPtr[0].as<reg*>() = p;

        return p;
    }

    std::vector<Page<>*> inuse, unused;
    Page<>* current;

    /**
     * @brief alloc storage and copy header to new object
     * will not change state in header except for Flags::kIsMinorObject flag
     * 
     * @param h header
     * @return reg* 
     */
    reg* allocMinor(ObjHeader h) {
        h.addFlag(ObjHeader::Flags::kIsMinorObject);
        return nullptr;
    }

    /**
     * @brief alloc storage and copy header to new object
     * will not change state in header except for Flags::kIsMinorObject flag
     * 
     * @param h header
     * @return reg* 
     */
    reg* allocMajor(ObjHeader h) {
        h.removeFlag(ObjHeader::Flags::kIsMinorObject);
        return allocMinor(h);
    }

    /**
     * @brief scan 
     * SHOULDNOT called on moved object
     * 
     * @param objPtr pointer to target object
     * @param func callback function
     */
    template <typename F>
        requires std::invocable<F, reg**>
    void callWithPointerMember(reg* objPtr, F&& func) {
        auto h = helper::getHeader(objPtr);
        assert(!h.hasFlag(ObjHeader::Flags::kIsMoved));

        if (!h.hasFlag(ObjHeader::Flags::kHasPointerMember)) {
            return;
        }
        if (h.hasFlag(ObjHeader::Flags::kHasNativeScanner)) {
            // TODO:
            tm->getType(h.typeId).pointerMemberScanner(this);
        }
        for (u8 i = 0; i < 8; ++i) {
            if (h.pointerMask & (1 << i)) {
                func(reinterpret_cast<reg**>(objPtr + i));
            }
        }
        if (h.sizeCompressed >= 8) {
            tm->getType(h.typeId).layout();
        }
    };

    struct PagePool {

    };
};

}