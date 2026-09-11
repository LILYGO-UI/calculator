#ifndef LILYGO_UI_CALCULATOR_PAGES_CALCULATOR_VIEW_MODEL_HPP
#define LILYGO_UI_CALCULATOR_PAGES_CALCULATOR_VIEW_MODEL_HPP

#include "components/lv_subject.hpp"
#include "pages/calculator/calculator_model.hpp"

#include <lvgl.h>

enum class CalculatorCommand {
    digit,
    decimal,
    clear,
    backspace,
    toggle_sign,
    percent,
    divide,
    multiply,
    subtract,
    add,
    equals,
    open_parenthesis,
    close_parenthesis,
    memory_clear,
    memory_add,
    memory_subtract,
    memory_recall,
    toggle_second,
    square,
    cube,
    power,
    exponential,
    ten_power,
    reciprocal,
    square_root,
    cube_root,
    root,
    natural_log,
    common_log,
    factorial,
    sine,
    cosine,
    tangent,
    constant_e,
    exponent,
    toggle_angle,
    hyperbolic_sine,
    hyperbolic_cosine,
    hyperbolic_tangent,
    constant_pi,
    random,
    absolute,
};

class CalculatorViewModel {
public:
    CalculatorViewModel();

    [[nodiscard]] const char *display_text() const noexcept;
    [[nodiscard]] const char *formula_text() const noexcept;
    [[nodiscard]] const char *clear_label() const noexcept;
    [[nodiscard]] const char *angle_label() const noexcept;
    [[nodiscard]] char active_operator() const noexcept;
    [[nodiscard]] bool second_mode() const noexcept;
    [[nodiscard]] bool has_memory() const noexcept;
    [[nodiscard]] double memory_value() const noexcept;

    [[nodiscard]] lv_subject_t *display_subject() noexcept;
    [[nodiscard]] lv_subject_t *formula_subject() noexcept;
    [[nodiscard]] lv_subject_t *clear_subject() noexcept;
    [[nodiscard]] lv_subject_t *angle_subject() noexcept;

    void perform(CalculatorCommand command, char digit = '\0');

private:
    void publish();

    calculator::CalculatorModel model_;
    StringSubject<64> display_{"0"};
    StringSubject<768> formula_{""};
    StringSubject<4> clear_{"AC"};
    StringSubject<8> angle_{"Rad"};
};

#endif
