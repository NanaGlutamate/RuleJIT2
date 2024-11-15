/**
 * @file context.hpp
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

#include <vector>

template <typename Ctx>
struct ThreadedContextProvider {
    static Ctx& getContext() {
        assert();
        return ctxVector.back();
    }

    static void setContext(Ctx ctx) {
        ctxVector.push_back(std::move(ctx));
    }

    static void removeContext() {
        ctxVector.pop_back();
    }

private:
    thread_local std::vector<Ctx> ctxVector = {};
}