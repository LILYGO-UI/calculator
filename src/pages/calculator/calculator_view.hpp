#ifndef LILYGO_UI_CALCULATOR_PAGES_CALCULATOR_VIEW_HPP
#define LILYGO_UI_CALCULATOR_PAGES_CALCULATOR_VIEW_HPP

#include "pages/calculator/calculator_view_model.hpp"

#include <lvgl.h>

#include <array>
#include <cstddef>

class CalculatorView {
public:
    explicit CalculatorView(CalculatorViewModel &view_model) noexcept;

    lv_obj_t *create(lv_obj_t *parent);
    void destroy() noexcept;

private:
    enum class ButtonKind {
        number,
        control,
        operation,
        scientific,
    };

    struct ButtonContext {
        CalculatorView *view = nullptr;
        CalculatorCommand command{};
        char digit = '\0';
    };

    struct ButtonRecord {
        lv_obj_t *button = nullptr;
        lv_obj_t *label  = nullptr;
        CalculatorCommand command{};
        ButtonKind kind = ButtonKind::number;
    };

    static void button_event(lv_event_t *event);
    static void key_event(lv_event_t *event);
    static void size_changed_event(lv_event_t *event);

    lv_obj_t *create_keypad(lv_obj_t *parent, bool scientific);
    void create_keypad_button(lv_obj_t *grid, const char *name, const char *label, CalculatorCommand command,
                              ButtonKind kind, int column, int row, int column_span = 1, char digit = '\0');
    void apply_layout() noexcept;
    void refresh_visual_state();
    void refresh_display_font();
    void style_button(ButtonRecord &record, bool selected);

    CalculatorViewModel &view_model_;
    lv_obj_t *surface_           = nullptr;
    lv_obj_t *display_panel_     = nullptr;
    lv_obj_t *formula_label_     = nullptr;
    lv_obj_t *display_label_     = nullptr;
    lv_obj_t *keypad_panel_      = nullptr;
    lv_obj_t *scientific_keypad_ = nullptr;
    lv_obj_t *basic_keypad_      = nullptr;
    std::array<ButtonContext, 56> button_contexts_{};
    std::array<ButtonRecord, 56> button_records_{};
    std::size_t button_count_ = 0;
    bool landscape_           = false;
};

#endif
