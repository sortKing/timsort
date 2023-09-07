#pragma once

#include <type_traits>

namespace my {

<<<<<<< HEAD
template <class F, class... ArgTypes>
class invoke_result {
public:
    using type = decltype(std::declval<F>()(std::declval<ArgTypes>()...));
};

template <class F, class... ArgTypes>
using invoke_result_t = typename invoke_result<F, ArgTypes...>::type;
=======
    template<class F, class... ArgTypes>
    class invoke_result {
    public:
        using type = decltype(std::declval<F>()(std::declval<ArgTypes>()...));
    };

    template<class F, class... ArgTypes> using invoke_result_t = typename invoke_result<F, ArgTypes...>::type;
>>>>>>> 6eba88305f037d296a18d55a228d6267110dec94

} // namespace my
