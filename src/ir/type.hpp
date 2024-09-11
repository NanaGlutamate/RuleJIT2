#pragma once

#include <type_traits>
#include <map>
#include <unordered_map>
#include <utility>
#include <variant>

#include "defs.hpp"
#include "tools/string_pool.hpp"

namespace rulejit {

struct Layout {
    std::vector<TypeToken> memberTypes;
};

struct FunctionType {
    struct FunctionParamType {
        TypeToken baseType;
        bool isConst;
        bool isVararg;
        bool isReferenced;
        bool isMutReferenced;
    };
    std::vector<FunctionParamType> params;
    TypeToken returnType;
};
struct ArrayType {
    // size.empty() if dynamic array
    std::vector<usize> size;
    TypeToken elementType;
};
struct BaseType {
    bool isClass;
    StringToken name;
};
struct TupleType {
    LayoutToken layout;
};
struct StructOrClassType {
    struct NamedLayout {
        std::vector<StringToken> memberNames;
        LayoutToken layout;
    };
    bool isClass;
    TypeTemplateToken templateBase;
    // nullptr means no name
    std::unordered_map<StringToken, NamedLayout> sumTypes;
};
struct TraitObj {
    TraitToken token;
};

struct Type {
    using InnerType = std::variant<FunctionType, ArrayType, BaseType, TupleType, StructOrClassType, TraitObj>;
    template <typename T>
        requires std::is_constructible_v<InnerType, T>
    Type(T&& data): data(std::forward<T>(data)) {};
    Type(const Type&) = default;
    Type(Type&&) = default;

    InnerType data;
};

struct Trait {

};


struct TypeManager {
    TypeManager(StringPool* sp): sp(sp) {
        layouts.reserve(256);
        types.reserve(256);
        templates.reserve(256);

        // make base types
        layouts.push_back({{}});
        types.push_back(TupleType{0});
        types.push_back(BaseType{false, sp->take("dynamic")});
        types.push_back(BaseType{false, sp->take("i64")});
        types.push_back(BaseType{false, sp->take("u64")});
        types.push_back(BaseType{false, sp->take("f64")});
        types.push_back(BaseType{true, sp->take("any")});
    }
    TypeManager(const TypeManager&) = delete;
    auto& operator=(const TypeManager&) = delete;

    Type& getType(TypeToken t) {
        return types[t];
    }

    TypeToken arrayTypeOf(TypeToken base) {
        return findCached(base, arrayType, [this](TypeToken base){
            types.emplace_back(ArrayType{base});
            return types.size() - 1;
        });
    }
    TypeToken tupleTypeOf(const std::vector<TypeToken>& base) {
        return findCached(base, tupleType, [this, base](std::vector<TypeToken> member){
            
            return types.size() - 1;
        });
    }
    // PooledList<TypeToken> getTypeList() {
    //     return PooledList<TypeToken>{pool0};
    // }
    // PooledList<std::tuple<StringToken, TypeToken>> getTokenTypePairList() {
    //     return PooledList<std::tuple<StringToken, TypeToken>>{pool1};
    // }
private:
    StringPool* sp;

    std::vector<Layout> layouts = {};
    std::vector<Type> types = {};

    // PooledList<TypeToken>::Pool pool0;
    // PooledList<std::tuple<StringToken, TypeToken>>::Pool pool1;

    std::unordered_map<TypeToken, TypeToken> arrayType = {};
    std::map<std::vector<TypeToken>, TypeToken, std::less<>> tupleType = {};

    template <typename Key, typename Container, typename CreateCallback>
        requires std::is_constructible_v<TypeToken, std::invoke_result_t<CreateCallback, Key>>
    TypeToken findCached(Key&& p, Container& cache, CreateCallback&& c) {
        auto it = cache.find(p);
        if (it == cache.end()) {
            it = cache.emplace(p, c(p)).first;
        }
        return it->second;
    }
};

} 