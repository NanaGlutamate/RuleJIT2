#pragma once

// #include <cstddef>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

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
    // size of total object (contains ObjHeader); -1 means out of range, should lookup through typemanager
    u16 size;
    // age in minor, color in elder
    u8 ageOrColor;
    // flags
    u8 flag;
    enum class Flags {
        kHasPointerMember = 1, 
        kHasFinalizer = 2, 
        kHasNativeScanner = 4, // for example, native hash map
        kIsMoved = 8, // used in move GC
    };
};
static_assert(sizeof(ObjHeader) == sizeof(u64));

struct alignas(u64) reg {
    u8 data[sizeof(u64)];
    template<typename Ty>
        requires std::is_same_v<Ty, u64> ||
            std::is_same_v<Ty, i64> ||
            std::is_same_v<Ty, f64> ||
            std::is_same_v<Ty, reg*> ||
            std::is_same_v<Ty, ObjHeader>
    Ty& as() { return *reinterpret_cast<Ty*>(data); }
};
static_assert(sizeof(reg) == sizeof(u64));

// unique through packages
using TypeToken = u32;
static_assert(std::is_same_v<TypeToken, decltype(ObjHeader{}.typeId)>);
// inner layout, ```type A struct (b: int)``` has same layout with ```type C struct (d: int)```, but are different types
using LayoutToken = u32;
using TemplateToken = u32;
using TraitToken = u32;

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