/**
 * @file ast_node.hpp
 * @author nanaglutamate
 * @brief 
 * @date 2024-11-15
 * 
 * @details 
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>nanaglutamate</td><td>2024-11-15</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <variant>
#include <type_traits>
#include <span>

#include "common_datastructure.hpp"

namespace rulejit {

struct Statement {};
struct Expression : public Statement { TypeHandle type; };
struct Def : public Statement { Token token; };
struct TemplateDef : public Def {
    std::span<Token> t_params;
    std::span<std::span<Token>> constraints;
};
struct SumTypeDef : public TemplateDef {
    struct MemberPair { TypeHandle mem_type; Token mem_token; };
    std::span<MemberPair> members;
};

struct ASTNode;
template<typename RESTRICTION = Statement>
class ASTHandle {
public:
    ASTHandle(): node(nullptr) {}
    template<typename OTHER>
        requires std::is_base_of_v<RESTRICTION, OTHER>
    ASTHandle(ASTHandle<OTHER>&& o) { node = o.node; }

    template<typename T>
    void emplace(T&&);
private:
    ASTNode* node;
};

struct ControlFlowStatement : public Statement {};

struct BinOpExpr final : public Expression { ASTHandle<Expression> lhs, rhs; };
struct ParenthesesExpr final : public Expression { ASTHandle<Expression> value; };
struct FunctionCallExpr final : public Expression { std::span<ASTHandle<Expression>> args; };
struct MemberExpr final : public Expression { Token member_token; };
struct SubscriptExpr final : public Expression { std::span<ASTHandle<Expression>> subscript; };

// codegen ...
struct AnnotatedDef final : public Def { ASTHandle<FunctionCallExpr> annotation; ASTHandle<Def> content; };
struct VarDef : public Def { TypeHandle value_type; ASTHandle<Expression> init_value; };
struct ConstDef : public Def { TypeHandle value_type; ASTHandle<Expression> init_value; };
// var a: vector3
// var b: Box<vector3> = &a // < lifetime check, cannot return
// // where Box use code gen, like `@getter(data) class Box<T> ( data: T )`
// // or extend language, class Box<T> ( *T )

// class与struct语义差别：值语义与引用语义，每个函数计算生命周期标注，将非逃逸值class分配在栈上
// 隐式为每个作用域创建一个class，利用上述机制自动完成逃逸分析(每个class包含上一层作用域，创建闭包时隐式传递当前class，获取环境值时通过反射)

// dynamic: 元表

// trait object: interface, 只能用于class
struct StructDef final : public SumTypeDef {};
struct ClassDef final : public SumTypeDef {};
struct TraitDef final : public TemplateDef {};
struct AliasDef final : public TemplateDef {};
struct FuncDef final : public TemplateDef { TypeHandle func_type; ASTHandle<Expression> body; };

struct PackageDeclare final : public Statement { Token package; };
struct ImportDeclare final : public Statement { Token package; Token target; Token rename; };
struct ExportDeclare final : public Statement { ASTHandle<Def> content; };
struct ExternTypeDeclare final : public Statement { Token token; };
struct ExternFuncDeclare final : public Statement { Token token; TypeHandle func_type; };

template<typename Ty, typename Ret, typename...Params>
concept visitor = 
    std::is_convertible_v<std::invoke_result_t<Ty, void, Params...>, Ret>;

struct ASTNode {
    std::variant<Expression> data;
    template<typename Self, typename Ret, typename ...Params, visitor<Ret, Params...> Ty>
    Ret visit(this Self&& self, Ty&& visitor, Params&& ...params) {
        if constexpr (sizeof...(params) == 0) {
            // todo: check if make sense
            return std::visit(visitor, std::forward<Self>(self).data);
        } else {
            return std::visit(
                [&visitor, &params...]<typename V>(V&& tar){
                    return std::invoke(std::forward<Ty>(visitor), std::forward<V>(tar), std::forward<Params>(params)...);
                },
                std::forward<Self>(self).data
            );
        }
    }
};

template<typename RES>
template<typename T>
inline void ASTHandle<RES>::emplace(T&& t) {
    static_assert(std::is_base_of_v<RES, std::remove_cvref_t<T>>);
    node->data = std::forward<T>(t);
}

}