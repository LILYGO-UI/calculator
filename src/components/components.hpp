#ifndef LILYGO_UI_CALCULATOR_COMPONENTS_HPP
#define LILYGO_UI_CALCULATOR_COMPONENTS_HPP

#include <lvgl.h>

#include <cstdint>

namespace components {

inline constexpr std::uint32_t color_page_background = 0x000000;

lv_obj_t *create_page(lv_obj_t *parent);

}  // namespace components

#endif
