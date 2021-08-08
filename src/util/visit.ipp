#include <boost/variant/variant.hpp>

#include <type_traits>

namespace util
{
  namespace detail
  {
    template<typename... Ts>
      struct require_all_same;
    template<typename T, typename... Ts>
      struct require_all_same<T, T, Ts...> : require_all_same<T, Ts...> {};
    template<typename T>
      struct require_all_same<T> { using type = T; };

    template<typename Variant, typename Fun>
      struct require_all_results_same_for_variant_apply;
    template<typename... VariantTypes, typename Fun>
      struct require_all_results_same_for_variant_apply<boost::variant<VariantTypes...>, Fun>
        : require_all_same<std::result_of_t<Fun (VariantTypes)>...> {};
  }

  template <typename Variant, typename... Funs>
    auto visit (Variant& variant, Funs... funs)
  {
    struct only_inherit_funs : Funs... {
      using Funs::operator()...;
    };
    using Ret = typename detail::require_all_results_same_for_variant_apply<std::remove_const_t<Variant>, only_inherit_funs>::type;

    struct lambda_visitor : boost::static_visitor<Ret>, Funs...
    {
      using Funs::operator()...;
      lambda_visitor (Funs... funs)
        : Funs (funs)...
      {}
    } visitor = {funs...};

    return boost::apply_visitor (visitor, variant);
  }
}
