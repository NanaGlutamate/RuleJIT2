#pragma once

// #include <cstddef>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <string_view>
#include <type_traits>
#include <vector>

namespace rulejit {

constexpr size_t PageSize = 4 * 1024;

using f32 = float;
using f64 = double;

using i8    = int8_t;
using u8    = uint8_t;
using i16   = int16_t;
using u16   = uint16_t;
using i32   = int32_t;
using u32   = uint32_t;
using i64   = int64_t;
using u64   = uint64_t;
using isize = std::ptrdiff_t;
using usize = std::size_t;

static_assert(sizeof(void*) == sizeof(usize));
static_assert(sizeof(auto(*)()->void) == sizeof(usize));

static_assert(sizeof(usize) <= sizeof(u64));
static_assert(sizeof(f64) <= sizeof(u64));
static_assert(sizeof(u64) == sizeof(u64));

struct alignas(u64) ObjHeader {
    // type id
    u32 typeId;
    // size of total object / sizeof(u64) (DONOT contains ObjHeader); u8(-1) means out of range, should lookup through typemanager
    u8 sizeCompressed;
    // age in minor, color in elder
    u8 ageOrColor;
    // flags
    u8 flag;
    // bit mask of pointer member, !!(pointerMask & (1 << n)) if ObjPtr[n] is pointer. avaliable even size > 8 * sizeof(u64)
    u8 pointerMask;

    ObjHeader getPrototype() {
        auto h = *this;
        // TODO: h &= 0xxxx
        h.ageOrColor = 0;
        h.removeFlag(Flags::kIsMinorObject);
        h.removeFlag(Flags::kIsMoved);
        return h;
    }

    enum class Color {
        kWhite = 0,
        kGray = 1,
        kBlack = 2,
    };
    void setColor(Color c) {
        assert(!hasFlag(Flags::kIsMinorObject));
        ageOrColor = static_cast<u8>(c);
    }
    
    bool isBigObject() {
        return sizeCompressed == decltype(sizeCompressed)(-1);
    }

    // TODO: record memory type in flags
    enum class Flags {
        kHasPointerMember = 1, 
        kHasFinalizer = 2, 
        kHasNativeScanner = 4, // for example, u64[] / native hash map
        
        kIsMoved = 8, // used in move GC
        kIsMinorObject = 16, // minor if set, major / static / huge if unset
    };
    bool hasFlag(Flags f) {
        return (flag & static_cast<u8>(f)) != 0;
    }
    void addFlag(Flags f) {
        flag &= static_cast<u8>(f);
    }
    void removeFlag(Flags f) {
        flag &= ~static_cast<u8>(f);
    }
};
static_assert(sizeof(ObjHeader) == sizeof(u64));

struct alignas(u64) reg {
    u8 bytes[sizeof(u64)];
    template<typename Ty>
        requires std::is_same_v<Ty, u64> ||
            std::is_same_v<Ty, i64> ||
            std::is_same_v<Ty, f64> ||
            std::is_same_v<Ty, reg*> ||
            std::is_same_v<Ty, ObjHeader>
    Ty& as() { return *std::launder(reinterpret_cast<Ty*>(bytes)); }
};
static_assert(sizeof(reg) == sizeof(u64));

// unique through packages
struct TypeToken {
    u32 data;
    TypeToken(u32 data): data(data) {};
    operator u32() { return data; }
};
static_assert(std::is_constructible_v<TypeToken, decltype(ObjHeader{}.typeId)> &&
    std::is_constructible_v<decltype(ObjHeader{}.typeId), TypeToken>);
// inner layout, ```type A struct (b: int)``` has same layout with ```type C struct (d: int)```, but are different types
struct LayoutToken {
    u32 data;
    LayoutToken(u32 data): data(data) {};
    operator u32() { return data; }
};
struct TypeTemplateToken {
    u32 data;
    TypeTemplateToken(u32 data): data(data) {};
    operator u32() { return data; }
};
struct TraitToken {
    u32 data;
    TraitToken(u32 data): data(data) {};
    operator u32() { return data; }
};

using FunctionTemplateToken = u32;

using GlobalVarToken = u32;

struct StringToken {
    const std::string_view* data;
    constexpr auto operator<=>(StringToken other) noexcept {
        return data <=> other.data;
    }
};

struct PackagedToken {
    StringToken package;
    StringToken token;
};

}