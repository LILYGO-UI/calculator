#ifndef LILYGO_UI_CALCULATOR_PAGES_CALCULATOR_MODEL_HPP
#define LILYGO_UI_CALCULATOR_PAGES_CALCULATOR_MODEL_HPP

#include <string>

namespace calculator {

enum class AngleMode {
    radians,
    degrees,
};

struct EvaluationResult {
    bool ok      = false;
    double value = 0.0;
    std::string normalized_expression;
    std::string error_message;
};

[[nodiscard]] std::string format_value(double value);
[[nodiscard]] EvaluationResult evaluate_expression(const std::string &expression, AngleMode angle_mode);

class CalculatorModel {
public:
    [[nodiscard]] const std::string &expression() const noexcept;
    [[nodiscard]] const std::string &formula_text() const noexcept;
    [[nodiscard]] const std::string &display_text() const noexcept;
    [[nodiscard]] const char *clear_label() const noexcept;
    [[nodiscard]] const char *angle_label() const noexcept;
    [[nodiscard]] char active_operator() const noexcept;
    [[nodiscard]] AngleMode angle_mode() const noexcept;
    [[nodiscard]] bool second_mode() const noexcept;
    [[nodiscard]] bool has_memory() const noexcept;
    [[nodiscard]] double memory_value() const noexcept;

    void input_digit(char digit);
    void input_decimal();
    void input_exponent();
    void set_binary_operator(char operation);
    void open_parenthesis();
    void close_parenthesis();
    void insert_constant(const std::string &constant);
    void apply_function(const std::string &function);
    void apply_power(int exponent);
    void apply_base_power(int base);
    void begin_root();
    void factorial();
    void percent();
    void toggle_sign();
    void random_value();

    void backspace();
    void clear();
    void equals();
    void toggle_angle_mode();
    void toggle_second_mode();

    void memory_clear() noexcept;
    void memory_add();
    void memory_subtract();
    void memory_recall();

private:
    static constexpr std::size_t maximum_expression_length = 192;

    void begin_input();
    void append_value_token(const std::string &token, bool number_continuation);
    void refresh_preview();
    void refresh_formula();
    [[nodiscard]] bool current_value(double &value) const;

    std::string expression_;
    std::string formula_text_;
    std::string display_text_ = "0";
    double memory_value_      = 0.0;
    AngleMode angle_mode_     = AngleMode::radians;
    bool second_mode_         = false;
    bool just_evaluated_      = false;
    bool error_               = false;
};

}  // namespace calculator

#endif
