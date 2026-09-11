#include "app.hpp"

#include <cm0/status_bar.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

namespace {

constexpr int partial_rows               = 80;
constexpr std::uint32_t color_background = 0x000000;
constexpr std::uint32_t color_operation  = 0xff9f0a;
constexpr std::uint32_t color_shell      = 0xf2f2f7;

std::vector<lv_color32_t> framebuffer;
int display_width = 0;

void flush_display(lv_display_t *display, const lv_area_t *area, std::uint8_t *pixels)
{
    const int width    = area->x2 - area->x1 + 1;
    const auto *source = reinterpret_cast<const lv_color32_t *>(pixels);
    for (int y = area->y1; y <= area->y2; ++y) {
        const auto source_offset = static_cast<std::size_t>(y - area->y1) * static_cast<std::size_t>(width);
        const auto destination_offset =
            static_cast<std::size_t>(y) * static_cast<std::size_t>(display_width) + static_cast<std::size_t>(area->x1);
        std::copy_n(source + source_offset, width, framebuffer.begin() + destination_offset);
    }
    lv_display_flush_ready(display);
}

bool write_ppm(const char *path, int width, int height)
{
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;
    file << "P6\n" << width << ' ' << height << "\n255\n";
    for (const auto &pixel : framebuffer) {
        file.put(static_cast<char>(pixel.red));
        file.put(static_cast<char>(pixel.green));
        file.put(static_cast<char>(pixel.blue));
    }
    return file.good();
}

void click(lv_obj_t *root, const char *name)
{
    auto *button = lv_obj_find_by_name(root, name);
    assert(button);
    lv_obj_send_event(button, LV_EVENT_CLICKED, nullptr);
}

lv_obj_t *create_runtime_content(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(color_shell), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(screen, 36, 0);
    lv_obj_set_style_clip_corner(screen, true, 0);

    auto *system_layer = lv_layer_top();
    auto *status       = cm0_status_bar_create(system_layer);
    assert(status);
    lv_obj_align(status, LV_ALIGN_TOP_MID, 0, 0);

    auto *content = lv_obj_create(screen);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_align(content, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_pad_top(content, CM0_STATUS_BAR_HEIGHT, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_radius(content, 0, 0);
    lv_obj_set_style_bg_color(content, lv_color_hex(color_shell), 0);
    lv_obj_set_style_bg_grad_dir(content, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    auto *home_indicator = cm0_home_indicator_create(system_layer);
    assert(home_indicator);
    lv_obj_set_size(home_indicator, LV_PCT(100), 38);
    lv_obj_align(home_indicator, LV_ALIGN_BOTTOM_MID, 0, -3);
    lv_obj_clear_flag(home_indicator,
                      static_cast<lv_obj_flag_t>(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));
    return content;
}

}  // namespace

int main(int argc, char **argv)
{
    assert(argc == 4);
    const int width  = std::atoi(argv[1]);
    const int height = std::atoi(argv[2]);
    assert(width >= 320 && height >= 240);

    display_width = width;
    framebuffer.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    std::vector<lv_color32_t> draw_buffer(static_cast<std::size_t>(width) * partial_rows);

    lv_init();
    auto *display = lv_display_create(width, height);
    assert(display);
    lv_display_set_buffers(display, draw_buffer.data(), nullptr,
                           static_cast<std::uint32_t>(draw_buffer.size() * sizeof(lv_color32_t)),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush_display);
    lv_display_set_default(display);

    cm0_app_context_t context{};
    context.root           = create_runtime_content(lv_screen_active());
    const auto *descriptor = cm0_app_get_descriptor();
    descriptor->open(&context);
    lv_obj_update_layout(context.root);

    auto *surface      = lv_obj_find_by_name(context.root, "calculator_surface");
    auto *display_text = lv_obj_find_by_name(context.root, "calculator_display");
    auto *formula      = lv_obj_find_by_name(context.root, "calculator_formula");
    auto *basic        = lv_obj_find_by_name(context.root, "calculator_basic_keypad");
    auto *scientific   = lv_obj_find_by_name(context.root, "calculator_scientific_keypad");
    auto *zero         = lv_obj_find_by_name(context.root, "calculator_0");
    auto *decimal      = lv_obj_find_by_name(context.root, "calculator_decimal");
    auto *operation    = lv_obj_find_by_name(context.root, "calculator_add");
    assert(surface && display_text && formula && basic && scientific && zero && decimal && operation);
    assert(lv_color_eq(lv_obj_get_style_bg_color(surface, LV_PART_MAIN), lv_color_hex(color_background)));
    assert(lv_color_eq(lv_obj_get_style_bg_color(operation, LV_PART_MAIN), lv_color_hex(color_operation)));
    assert(lv_obj_get_width(zero) > lv_obj_get_width(decimal));
    assert((width > height) == !lv_obj_has_flag(scientific, LV_OBJ_FLAG_HIDDEN));

    click(context.root, "calculator_8");
    click(context.root, "calculator_multiply");
    click(context.root, "calculator_9");
    click(context.root, "calculator_equals");
    assert(std::string(lv_label_get_text(display_text)) == "72");
    assert(std::string(lv_label_get_text(formula)) == "8 × 9 =");

    lv_tick_inc(20);
    lv_refr_now(display);
    const auto first        = framebuffer.front();
    const bool has_contrast = std::any_of(framebuffer.cbegin() + 1, framebuffer.cend(), [&](const auto &pixel) {
        return pixel.red != first.red || pixel.green != first.green || pixel.blue != first.blue;
    });
    assert(has_contrast);
    assert(write_ppm(argv[3], width, height));

    descriptor->close();
    lv_display_delete(display);
    lilygo_ui_fonts_deinit();
    lv_deinit();
    return 0;
}
