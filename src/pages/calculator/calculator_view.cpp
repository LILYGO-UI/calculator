#include "pages/calculator/calculator_view.hpp"

#include "components/components.hpp"

#include <cm0/typography.h>

#include <algorithm>
#include <cstring>

namespace {

constexpr std::uint32_t color_number_button    = 0x333333;
constexpr std::uint32_t color_science_button   = 0x242424;
constexpr std::uint32_t color_control_button   = 0xa5a5a5;
constexpr std::uint32_t color_operation_button = 0xff9f0a;
constexpr std::uint32_t color_primary_text     = 0xffffff;
constexpr std::uint32_t color_secondary_text   = 0x9b9b9f;
constexpr std::uint32_t color_dark_text        = 0x101010;

constexpr lv_coord_t basic_columns[] = {
    LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST,
};
constexpr lv_coord_t scientific_columns[] = {
    LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST,
};
constexpr lv_coord_t keypad_rows[] = {
    LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST,
};

int clamp_value(int value, int minimum, int maximum)
{
    return std::max(minimum, std::min(value, maximum));
}

char command_operator(CalculatorCommand command)
{
    switch (command) {
        case CalculatorCommand::divide:
            return '/';
        case CalculatorCommand::multiply:
            return '*';
        case CalculatorCommand::subtract:
            return '-';
        case CalculatorCommand::add:
            return '+';
        case CalculatorCommand::power:
            return '^';
        default:
            return '\0';
    }
}

}  // namespace

CalculatorView::CalculatorView(CalculatorViewModel &view_model) noexcept : view_model_(view_model)
{
}

lv_obj_t *CalculatorView::create(lv_obj_t *parent)
{
    surface_ = components::create_page(parent);
    lv_obj_set_name_static(surface_, "calculator_surface");
    lv_obj_set_style_bg_color(surface_, lv_color_hex(components::color_page_background), 0);
    lv_obj_set_style_pad_row(surface_, 12, 0);
    lv_obj_add_event_cb(surface_, size_changed_event, LV_EVENT_SIZE_CHANGED, this);

    display_panel_ = lv_obj_create(surface_);
    lv_obj_set_name_static(display_panel_, "calculator_display_panel");
    lv_obj_set_width(display_panel_, LV_PCT(100));
    lv_obj_set_flex_grow(display_panel_, 1);
    lv_obj_set_style_border_width(display_panel_, 0, 0);
    lv_obj_set_style_radius(display_panel_, 0, 0);
    lv_obj_set_style_bg_opa(display_panel_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(display_panel_, 0, 0);
    lv_obj_set_style_pad_row(display_panel_, 8, 0);
    lv_obj_clear_flag(display_panel_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(display_panel_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(display_panel_, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);

    formula_label_ = lv_label_create(display_panel_);
    lv_obj_set_name_static(formula_label_, "calculator_formula");
    lv_obj_set_width(formula_label_, LV_PCT(100));
    lv_label_set_long_mode(formula_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(formula_label_, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(formula_label_, lv_color_hex(color_secondary_text), 0);
    lv_obj_set_style_text_font(formula_label_, lilygo_ui_font_get(22), 0);
    lv_label_bind_text(formula_label_, view_model_.formula_subject(), nullptr);

    display_label_ = lv_label_create(display_panel_);
    lv_obj_set_name_static(display_label_, "calculator_display");
    lv_obj_set_width(display_label_, LV_PCT(100));
    lv_label_set_long_mode(display_label_, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(display_label_, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(display_label_, lv_color_hex(color_primary_text), 0);
    lv_label_bind_text(display_label_, view_model_.display_subject(), nullptr);

    keypad_panel_ = lv_obj_create(surface_);
    lv_obj_set_name_static(keypad_panel_, "calculator_keypad");
    lv_obj_set_width(keypad_panel_, LV_PCT(100));
    lv_obj_set_style_border_width(keypad_panel_, 0, 0);
    lv_obj_set_style_radius(keypad_panel_, 0, 0);
    lv_obj_set_style_bg_opa(keypad_panel_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(keypad_panel_, 0, 0);
    lv_obj_set_style_pad_column(keypad_panel_, 12, 0);
    lv_obj_clear_flag(keypad_panel_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(keypad_panel_, LV_FLEX_FLOW_ROW);

    scientific_keypad_ = create_keypad(keypad_panel_, true);
    basic_keypad_      = create_keypad(keypad_panel_, false);

    apply_layout();
    refresh_visual_state();
    return surface_;
}

void CalculatorView::destroy() noexcept
{
    if (surface_) lv_obj_delete(surface_);
    surface_           = nullptr;
    display_panel_     = nullptr;
    formula_label_     = nullptr;
    display_label_     = nullptr;
    keypad_panel_      = nullptr;
    scientific_keypad_ = nullptr;
    basic_keypad_      = nullptr;
    button_count_      = 0;
}

lv_obj_t *CalculatorView::create_keypad(lv_obj_t *parent, bool scientific)
{
    auto *grid = lv_obj_create(parent);
    lv_obj_set_name_static(grid, scientific ? "calculator_scientific_keypad" : "calculator_basic_keypad");
    lv_obj_set_height(grid, LV_PCT(100));
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_radius(grid, 0, 0);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_grid_dsc_array(grid, scientific ? scientific_columns : basic_columns, keypad_rows);

    if (scientific) {
        create_keypad_button(grid, "calculator_open_parenthesis", "(", CalculatorCommand::open_parenthesis,
                             ButtonKind::scientific, 0, 0);
        create_keypad_button(grid, "calculator_close_parenthesis", ")", CalculatorCommand::close_parenthesis,
                             ButtonKind::scientific, 1, 0);
        create_keypad_button(grid, "calculator_memory_clear", "mc", CalculatorCommand::memory_clear,
                             ButtonKind::scientific, 2, 0);
        create_keypad_button(grid, "calculator_memory_add", "m+", CalculatorCommand::memory_add, ButtonKind::scientific,
                             3, 0);
        create_keypad_button(grid, "calculator_memory_subtract", "m-", CalculatorCommand::memory_subtract,
                             ButtonKind::scientific, 4, 0);
        create_keypad_button(grid, "calculator_memory_recall", "mr", CalculatorCommand::memory_recall,
                             ButtonKind::scientific, 5, 0);

        create_keypad_button(grid, "calculator_second", "2nd", CalculatorCommand::toggle_second, ButtonKind::scientific,
                             0, 1);
        create_keypad_button(grid, "calculator_square", "x²", CalculatorCommand::square, ButtonKind::scientific, 1, 1);
        create_keypad_button(grid, "calculator_cube", "x³", CalculatorCommand::cube, ButtonKind::scientific, 2, 1);
        create_keypad_button(grid, "calculator_power", "xʸ", CalculatorCommand::power, ButtonKind::scientific, 3, 1);
        create_keypad_button(grid, "calculator_exponential", "eˣ", CalculatorCommand::exponential,
                             ButtonKind::scientific, 4, 1);
        create_keypad_button(grid, "calculator_ten_power", "10ˣ", CalculatorCommand::ten_power, ButtonKind::scientific,
                             5, 1);

        create_keypad_button(grid, "calculator_reciprocal", "1/x", CalculatorCommand::reciprocal,
                             ButtonKind::scientific, 0, 2);
        create_keypad_button(grid, "calculator_square_root", "√x", CalculatorCommand::square_root,
                             ButtonKind::scientific, 1, 2);
        create_keypad_button(grid, "calculator_cube_root", "3√x", CalculatorCommand::cube_root, ButtonKind::scientific,
                             2, 2);
        create_keypad_button(grid, "calculator_root", "ʸ√x", CalculatorCommand::root, ButtonKind::scientific, 3, 2);
        create_keypad_button(grid, "calculator_natural_log", "ln", CalculatorCommand::natural_log,
                             ButtonKind::scientific, 4, 2);
        create_keypad_button(grid, "calculator_common_log", "log₁₀", CalculatorCommand::common_log,
                             ButtonKind::scientific, 5, 2);

        create_keypad_button(grid, "calculator_factorial", "x!", CalculatorCommand::factorial, ButtonKind::scientific,
                             0, 3);
        create_keypad_button(grid, "calculator_sine", "sin", CalculatorCommand::sine, ButtonKind::scientific, 1, 3);
        create_keypad_button(grid, "calculator_cosine", "cos", CalculatorCommand::cosine, ButtonKind::scientific, 2, 3);
        create_keypad_button(grid, "calculator_tangent", "tan", CalculatorCommand::tangent, ButtonKind::scientific, 3,
                             3);
        create_keypad_button(grid, "calculator_constant_e", "e", CalculatorCommand::constant_e, ButtonKind::scientific,
                             4, 3);
        create_keypad_button(grid, "calculator_exponent", "EE", CalculatorCommand::exponent, ButtonKind::scientific, 5,
                             3);

        create_keypad_button(grid, "calculator_angle", "Rad", CalculatorCommand::toggle_angle, ButtonKind::scientific,
                             0, 4);
        create_keypad_button(grid, "calculator_sinh", "sinh", CalculatorCommand::hyperbolic_sine,
                             ButtonKind::scientific, 1, 4);
        create_keypad_button(grid, "calculator_cosh", "cosh", CalculatorCommand::hyperbolic_cosine,
                             ButtonKind::scientific, 2, 4);
        create_keypad_button(grid, "calculator_tanh", "tanh", CalculatorCommand::hyperbolic_tangent,
                             ButtonKind::scientific, 3, 4);
        create_keypad_button(grid, "calculator_pi", "π", CalculatorCommand::constant_pi, ButtonKind::scientific, 4, 4);
        create_keypad_button(grid, "calculator_random", "Rand", CalculatorCommand::random, ButtonKind::scientific, 5,
                             4);
    } else {
        create_keypad_button(grid, "calculator_clear", "AC", CalculatorCommand::clear, ButtonKind::control, 0, 0);
        create_keypad_button(grid, "calculator_toggle_sign", "±", CalculatorCommand::toggle_sign, ButtonKind::control,
                             1, 0);
        create_keypad_button(grid, "calculator_percent", "%", CalculatorCommand::percent, ButtonKind::control, 2, 0);
        create_keypad_button(grid, "calculator_divide", "÷", CalculatorCommand::divide, ButtonKind::operation, 3, 0);

        create_keypad_button(grid, "calculator_7", "7", CalculatorCommand::digit, ButtonKind::number, 0, 1, 1, '7');
        create_keypad_button(grid, "calculator_8", "8", CalculatorCommand::digit, ButtonKind::number, 1, 1, 1, '8');
        create_keypad_button(grid, "calculator_9", "9", CalculatorCommand::digit, ButtonKind::number, 2, 1, 1, '9');
        create_keypad_button(grid, "calculator_multiply", "×", CalculatorCommand::multiply, ButtonKind::operation, 3,
                             1);

        create_keypad_button(grid, "calculator_4", "4", CalculatorCommand::digit, ButtonKind::number, 0, 2, 1, '4');
        create_keypad_button(grid, "calculator_5", "5", CalculatorCommand::digit, ButtonKind::number, 1, 2, 1, '5');
        create_keypad_button(grid, "calculator_6", "6", CalculatorCommand::digit, ButtonKind::number, 2, 2, 1, '6');
        create_keypad_button(grid, "calculator_subtract", "−", CalculatorCommand::subtract, ButtonKind::operation, 3,
                             2);

        create_keypad_button(grid, "calculator_1", "1", CalculatorCommand::digit, ButtonKind::number, 0, 3, 1, '1');
        create_keypad_button(grid, "calculator_2", "2", CalculatorCommand::digit, ButtonKind::number, 1, 3, 1, '2');
        create_keypad_button(grid, "calculator_3", "3", CalculatorCommand::digit, ButtonKind::number, 2, 3, 1, '3');
        create_keypad_button(grid, "calculator_add", "+", CalculatorCommand::add, ButtonKind::operation, 3, 3);

        create_keypad_button(grid, "calculator_0", "0", CalculatorCommand::digit, ButtonKind::number, 0, 4, 2, '0');
        create_keypad_button(grid, "calculator_decimal", ".", CalculatorCommand::decimal, ButtonKind::number, 2, 4);
        create_keypad_button(grid, "calculator_equals", "=", CalculatorCommand::equals, ButtonKind::operation, 3, 4);
    }
    return grid;
}

void CalculatorView::create_keypad_button(lv_obj_t *grid, const char *name, const char *label,
                                          CalculatorCommand command, ButtonKind kind, int column, int row,
                                          int column_span, char digit)
{
    if (button_count_ >= button_records_.size()) return;
    auto *button = lv_button_create(grid);
    lv_obj_set_name_static(button, name);
    lv_obj_set_grid_cell(button, LV_GRID_ALIGN_STRETCH, column, column_span, LV_GRID_ALIGN_STRETCH, row, 1);
    lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_shadow_width(button, 0, 0);
    lv_obj_set_style_pad_all(button, 0, 0);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);

    auto *text = lv_label_create(button);
    lv_label_set_text(text, label);
    lv_obj_set_style_text_align(text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(text);

    auto &context   = button_contexts_[button_count_];
    context.view    = this;
    context.command = command;
    context.digit   = digit;
    lv_obj_add_event_cb(button, button_event, LV_EVENT_CLICKED, &context);

    button_records_[button_count_] = {button, text, command, kind};
    ++button_count_;

    if (command == CalculatorCommand::clear) lv_label_bind_text(text, view_model_.clear_subject(), nullptr);
    if (command == CalculatorCommand::toggle_angle) lv_label_bind_text(text, view_model_.angle_subject(), nullptr);
}

void CalculatorView::button_event(lv_event_t *event)
{
    auto *context = static_cast<ButtonContext *>(lv_event_get_user_data(event));
    if (!context || !context->view) return;
    context->view->view_model_.perform(context->command, context->digit);
    context->view->refresh_visual_state();
}

void CalculatorView::size_changed_event(lv_event_t *event)
{
    static_cast<CalculatorView *>(lv_event_get_user_data(event))->apply_layout();
}

void CalculatorView::apply_layout() noexcept
{
    if (!surface_ || !keypad_panel_ || !scientific_keypad_ || !basic_keypad_) return;
    const int width  = lv_obj_get_width(surface_);
    const int height = lv_obj_get_height(surface_);
    if (width <= 0 || height <= 0) return;

    landscape_                   = width > height && width >= 720;
    const int horizontal_padding = landscape_ ? 20 : (width < 400 ? 14 : 24);
    const int top_padding        = landscape_ ? 8 : 16;
    const int bottom_padding     = landscape_ ? 32 : 48;
    const int gap                = landscape_ ? 8 : (width < 400 ? 8 : 14);

    lv_obj_set_style_pad_left(surface_, horizontal_padding, 0);
    lv_obj_set_style_pad_right(surface_, horizontal_padding, 0);
    lv_obj_set_style_pad_top(surface_, top_padding, 0);
    lv_obj_set_style_pad_bottom(surface_, bottom_padding, 0);
    lv_obj_set_style_pad_row(surface_, landscape_ ? 8 : 12, 0);

    const int content_width  = std::max(1, width - horizontal_padding * 2);
    const int content_height = std::max(1, height - top_padding - bottom_padding);
    if (landscape_) {
        lv_obj_clear_flag(scientific_keypad_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_width(scientific_keypad_, LV_PCT(60));
        lv_obj_set_width(basic_keypad_, LV_PCT(40));
        lv_obj_set_flex_grow(basic_keypad_, 1);
        lv_obj_set_style_pad_column(keypad_panel_, 12, 0);
        lv_obj_set_height(keypad_panel_, clamp_value(content_height - 108, 260, 400));
        lv_obj_set_style_pad_column(scientific_keypad_, gap, 0);
        lv_obj_set_style_pad_row(scientific_keypad_, gap, 0);
        lv_obj_set_style_pad_column(basic_keypad_, gap, 0);
        lv_obj_set_style_pad_row(basic_keypad_, gap, 0);
        lv_obj_set_style_text_font(formula_label_, lilygo_ui_font_get(18), 0);
    } else {
        lv_obj_add_flag(scientific_keypad_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_width(basic_keypad_, LV_PCT(100));
        lv_obj_set_flex_grow(basic_keypad_, 0);
        const int button_width = (content_width - gap * 3) / 4;
        const int ideal_height = button_width * 5 + gap * 4;
        lv_obj_set_height(keypad_panel_, std::min(ideal_height, std::max(120, content_height - 170)));
        lv_obj_set_style_pad_column(basic_keypad_, gap, 0);
        lv_obj_set_style_pad_row(basic_keypad_, gap, 0);
        lv_obj_set_style_text_font(formula_label_, lilygo_ui_font_get(22), 0);
    }

    refresh_visual_state();
}

void CalculatorView::refresh_visual_state()
{
    const char active_operation = view_model_.active_operator();
    for (std::size_t index = 0; index < button_count_; ++index) {
        auto &record = button_records_[index];
        const bool selected =
            command_operator(record.command) != '\0' && command_operator(record.command) == active_operation;
        style_button(record, selected);

        if (record.command == CalculatorCommand::toggle_second) {
            lv_label_set_text(record.label, view_model_.second_mode() ? "2nd*" : "2nd");
            if (view_model_.second_mode()) {
                lv_obj_set_style_bg_color(record.button, lv_color_hex(color_primary_text), 0);
                lv_obj_set_style_text_color(record.label, lv_color_hex(color_dark_text), 0);
            }
        } else if (record.command == CalculatorCommand::sine) {
            lv_label_set_text(record.label, view_model_.second_mode() ? "asin" : "sin");
        } else if (record.command == CalculatorCommand::cosine) {
            lv_label_set_text(record.label, view_model_.second_mode() ? "acos" : "cos");
        } else if (record.command == CalculatorCommand::tangent) {
            lv_label_set_text(record.label, view_model_.second_mode() ? "atan" : "tan");
        } else if (record.command == CalculatorCommand::hyperbolic_sine) {
            lv_label_set_text(record.label, view_model_.second_mode() ? "asinh" : "sinh");
        } else if (record.command == CalculatorCommand::hyperbolic_cosine) {
            lv_label_set_text(record.label, view_model_.second_mode() ? "acosh" : "cosh");
        } else if (record.command == CalculatorCommand::hyperbolic_tangent) {
            lv_label_set_text(record.label, view_model_.second_mode() ? "atanh" : "tanh");
        }

        if (record.command == CalculatorCommand::memory_clear || record.command == CalculatorCommand::memory_recall) {
            lv_obj_remove_state(record.button, LV_STATE_DISABLED);
            if (!view_model_.has_memory())
                lv_obj_set_style_text_color(record.label, lv_color_hex(color_secondary_text), 0);
        }
    }
    refresh_display_font();
}

void CalculatorView::refresh_display_font()
{
    if (!display_label_) return;
    const std::size_t length = std::strlen(view_model_.display_text());
    std::uint32_t size       = landscape_ ? 56U : 72U;
    if (length > 11) size = landscape_ ? 40U : 52U;
    if (length > 15) size = landscape_ ? 32U : 40U;
    lv_obj_set_style_text_font(display_label_, lilygo_ui_font_get(size), 0);
}

void CalculatorView::style_button(ButtonRecord &record, bool selected)
{
    std::uint32_t background = color_number_button;
    std::uint32_t text       = color_primary_text;
    std::uint32_t pressed    = 0x5a5a5c;
    if (record.kind == ButtonKind::control) {
        background = color_control_button;
        text       = color_dark_text;
        pressed    = 0xd4d4d2;
    } else if (record.kind == ButtonKind::operation) {
        background = selected ? color_primary_text : color_operation_button;
        text       = selected ? color_operation_button : color_primary_text;
        pressed    = 0xffc15a;
    } else if (record.kind == ButtonKind::scientific) {
        background = color_science_button;
        pressed    = 0x505052;
    }

    lv_obj_set_style_bg_color(record.button, lv_color_hex(background), 0);
    lv_obj_set_style_bg_opa(record.button, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(record.button, lv_color_hex(pressed), LV_STATE_PRESSED);
    lv_obj_set_style_text_color(record.label, lv_color_hex(text), 0);

    std::uint32_t font_size = landscape_ ? 28U : 40U;
    if (record.kind == ButtonKind::scientific) {
        const std::size_t label_length = std::strlen(lv_label_get_text(record.label));
        font_size                      = label_length > 4 ? 14U : 18U;
    } else if (record.kind == ButtonKind::control) {
        font_size = landscape_ ? 24U : 32U;
    }
    lv_obj_set_style_text_font(record.label, lilygo_ui_font_get(font_size), 0);
}
