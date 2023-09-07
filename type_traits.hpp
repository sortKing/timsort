#pragma once

#include <type_traits>

namespace my {

template <class F, class... ArgTypes>
class invoke_result {
public:
    using type = decltype(std::declval<F>()(std::declval<ArgTypes>()...));
};

template <class F, class... ArgTypes>
using invoke_result_t = typename invoke_result<F, ArgTypes...>::type;

} // namespace my
