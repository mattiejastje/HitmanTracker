#pragma once

#include <tuple>

#include "read_write.hpp"

// Binds a version enum value to a descriptor type that should be
// used to read the game struct for that platform.
template <auto V, mempeep::IsDescriptor Desc>
struct Variant {
    using VariantType = decltype(V);
    static constexpr VariantType variant = V;
    using DescType = Desc;
};

// Reads a game struct that has one descriptor type per platform variant
// (Steam, GOG, ...), all sharing one native output type.
template <
    typename... Variants,
    mempeep::IsMemoryReader Reader,
    mempeep::IsTracer Tracer>
bool read_variant(
    mempeep::address_t<Reader> address,
    Reader& reader,
    Tracer& tracer,
    typename std::tuple_element_t<0, std::tuple<Variants...>>::VariantType
        variant,
    mempeep::native_type_t<
        typename std::tuple_element_t<0, std::tuple<Variants...>>::DescType>&
        out
) {
    static_assert(sizeof...(Variants) > 0, "must provide at least one variant");
    using First = std::tuple_element_t<0, std::tuple<Variants...>>;
    using NativeType = mempeep::native_type_t<typename First::DescType>;
    using VariantType = typename First::VariantType;
    static_assert(
        (std::same_as<
             NativeType,
             mempeep::native_type_t<typename Variants::DescType>>
         && ...),
        "all variant descriptor types must have the same native type"
    );
    static_assert(
        (std::same_as<VariantType, typename Variants::VariantType> && ...),
        "all variants must have the same variant type"
    );
    return (
        (variant == Variants::variant
             ? read_at_address<typename Variants::DescType>(
                   address, reader, tracer, out
               )
             : false)
        || ...
    );
}