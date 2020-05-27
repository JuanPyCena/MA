#pragma once

#include "avauthlib_export.h"

namespace av
{
namespace auth
{
/**
 * @brief Converts value from one type to another.
 * @tparam ToType The type to convert to.
 * @tparam FromType The type to convert from.
 * @param [in] from The value to convert from.
 * @param [in] defaulValue The default value to return in case conversion fails.
 * @return The converted value or the defaultValue if conversion fails.
 * @note This function must be specialized for each combination of types.
 */
template <typename ToType, typename FromType>
ToType convert(FromType const& from, ToType&& defaultValue = ToType{});

}  // namespace auth
}  // namespace av
