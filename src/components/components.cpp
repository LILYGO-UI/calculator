#include "components/components.hpp"

namespace components {

lv_obj_t *create_page(lv_obj_t *parent)
{
    auto *page = lv_obj_create(parent);
    lv_obj_set_name_static(page, "app_surface");
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(color_page_background), 0);
    lv_obj_set_style_bg_grad_dir(page, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    return page;
}

}  // namespace components
