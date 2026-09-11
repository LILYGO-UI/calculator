#include "pages/calculator/calculator_model.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <utility>

namespace calculator {
namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double e  = 2.71828182845904523536;

bool is_binary_operator(char value)
{
    return value == '+' || value == '-' || value == '*' || value == '/' || value == '^';
}

bool is_value_ending(char value)
{
    return std::isdigit(static_cast<unsigned char>(value)) || std::isalpha(static_cast<unsigned char>(value)) ||
           value == ')' || value == '!' || value == '%';
}

std::string trim_trailing_zeros(std::string text)
{
    if (text.find_first_of("eE") != std::string::npos || text.find('.') == std::string::npos) return text;
    while (!text.empty() && text.back() == '0') text.pop_back();
    if (!text.empty() && text.back() == '.') text.pop_back();
    return text.empty() ? "0" : text;
}

std::string close_open_parentheses(const std::string &expression)
{
    int balance = 0;
    std::string normalized;
    normalized.reserve(expression.size() + 8);
    for (const char value : expression) {
        if (value == '(') {
            ++balance;
            normalized.push_back(value);
        } else if (value == ')') {
            if (balance > 0) {
                --balance;
                normalized.push_back(value);
            }
        } else {
            normalized.push_back(value);
        }
    }
    normalized.append(static_cast<std::size_t>(balance), ')');
    return normalized;
}

std::string prettify(const std::string &expression)
{
    std::string result;
    result.reserve(expression.size() + 8);
    for (std::size_t index = 0; index < expression.size(); ++index) {
        if (expression.compare(index, 4, "sqrt") == 0) {
            result += "√";
            index += 3;
            continue;
        }
        if (expression.compare(index, 2, "pi") == 0) {
            result += "π";
            ++index;
            continue;
        }
        switch (expression[index]) {
            case '*':
                result += " × ";
                break;
            case '/':
                result += " ÷ ";
                break;
            case '+':
                result += " + ";
                break;
            case '-':
                result += " − ";
                break;
            default:
                result.push_back(expression[index]);
                break;
        }
    }
    return result;
}

class ExpressionParser {
public:
    ExpressionParser(std::string expression, AngleMode angle_mode)
        : expression_(std::move(expression)), angle_mode_(angle_mode)
    {
    }

    EvaluationResult evaluate()
    {
        position_ = 0;
        error_message_.clear();
        const double value = parse_expression();
        skip_spaces();
        if (error_message_.empty() && position_ != expression_.size()) set_error("Unexpected trailing input");
        return {error_message_.empty(), value, expression_, error_message_};
    }

private:
    void skip_spaces()
    {
        while (position_ < expression_.size() && std::isspace(static_cast<unsigned char>(expression_[position_]))) {
            ++position_;
        }
    }

    void set_error(const char *message)
    {
        if (error_message_.empty()) error_message_ = message;
    }

    bool match(char expected)
    {
        skip_spaces();
        if (position_ >= expression_.size() || expression_[position_] != expected) return false;
        ++position_;
        return true;
    }

    char peek()
    {
        skip_spaces();
        return position_ < expression_.size() ? expression_[position_] : '\0';
    }

    double parse_expression()
    {
        double value = parse_term();
        while (error_message_.empty()) {
            if (match('+'))
                value += parse_term();
            else if (match('-'))
                value -= parse_term();
            else
                break;
        }
        return value;
    }

    double parse_term()
    {
        double value = parse_power();
        while (error_message_.empty()) {
            if (match('*')) {
                value *= parse_power();
            } else if (match('/')) {
                const double divisor = parse_power();
                if (std::fabs(divisor) < 1e-12) {
                    set_error("Division by zero");
                    return 0.0;
                }
                value /= divisor;
            } else {
                break;
            }
        }
        return value;
    }

    double parse_power()
    {
        double value = parse_unary();
        if (match('^')) {
            value = std::pow(value, parse_power());
            if (!std::isfinite(value)) set_error("Power out of range");
        }
        return value;
    }

    double parse_unary()
    {
        if (match('+')) return parse_unary();
        if (match('-')) return -parse_unary();
        return parse_postfix();
    }

    double parse_postfix()
    {
        double value = parse_primary();
        while (error_message_.empty()) {
            if (match('!'))
                value = factorial_value(value);
            else if (match('%'))
                value *= 0.01;
            else
                break;
        }
        return value;
    }

    double parse_primary()
    {
        if (match('(')) {
            const double value = parse_expression();
            if (!match(')')) set_error("Missing closing parenthesis");
            return value;
        }

        if (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '.') return parse_number();
        if (!std::isalpha(static_cast<unsigned char>(peek()))) {
            set_error("Expected a number or function");
            return 0.0;
        }

        const std::string identifier = parse_identifier();
        if (identifier == "pi") return pi;
        if (identifier == "e") return e;
        if (!match('(')) {
            set_error("Function call missing parenthesis");
            return 0.0;
        }

        const double first = parse_expression();
        if (!error_message_.empty()) return 0.0;
        if (match(',')) {
            const double second = parse_expression();
            if (!match(')')) set_error("Missing closing parenthesis");
            return error_message_.empty() ? apply_binary_function(identifier, first, second) : 0.0;
        }
        if (!match(')')) {
            set_error("Missing closing parenthesis");
            return 0.0;
        }
        return apply_unary_function(identifier, first);
    }

    double parse_number()
    {
        skip_spaces();
        const char *start  = expression_.c_str() + position_;
        char *end          = nullptr;
        const double value = std::strtod(start, &end);
        if (end == start) {
            set_error("Invalid number");
            return 0.0;
        }
        position_ += static_cast<std::size_t>(end - start);
        return value;
    }

    std::string parse_identifier()
    {
        const std::size_t start = position_;
        while (position_ < expression_.size() && std::isalpha(static_cast<unsigned char>(expression_[position_]))) {
            ++position_;
        }
        return expression_.substr(start, position_ - start);
    }

    double factorial_value(double value)
    {
        const double rounded = std::round(value);
        if (value < 0.0 || std::fabs(rounded - value) > 1e-9 || rounded > 170.0) {
            set_error("Factorial out of range");
            return 0.0;
        }
        double result = 1.0;
        for (int factor = 2; factor <= static_cast<int>(rounded); ++factor) result *= factor;
        return result;
    }

    double to_radians(double value) const
    {
        return angle_mode_ == AngleMode::radians ? value : value * pi / 180.0;
    }

    double from_radians(double value) const
    {
        return angle_mode_ == AngleMode::radians ? value : value * 180.0 / pi;
    }

    double checked(double value)
    {
        if (!std::isfinite(value)) {
            set_error("Invalid function domain");
            return 0.0;
        }
        return value;
    }

    double apply_unary_function(const std::string &name, double value)
    {
        if (name == "sin") return checked(std::sin(to_radians(value)));
        if (name == "cos") return checked(std::cos(to_radians(value)));
        if (name == "tan") return checked(std::tan(to_radians(value)));
        if (name == "asin") return checked(from_radians(std::asin(value)));
        if (name == "acos") return checked(from_radians(std::acos(value)));
        if (name == "atan") return checked(from_radians(std::atan(value)));
        if (name == "sinh") return checked(std::sinh(value));
        if (name == "cosh") return checked(std::cosh(value));
        if (name == "tanh") return checked(std::tanh(value));
        if (name == "asinh") return checked(std::asinh(value));
        if (name == "acosh") return checked(std::acosh(value));
        if (name == "atanh") return checked(std::atanh(value));
        if (name == "ln") return checked(std::log(value));
        if (name == "log") return checked(std::log10(value));
        if (name == "sqrt") return checked(std::sqrt(value));
        if (name == "cbrt") return checked(std::cbrt(value));
        if (name == "abs") return checked(std::fabs(value));
        if (name == "exp") return checked(std::exp(value));
        set_error("Unsupported function");
        return 0.0;
    }

    double apply_binary_function(const std::string &name, double radicand, double degree)
    {
        if (name != "root" || std::fabs(degree) < 1e-12) {
            set_error("Invalid root");
            return 0.0;
        }
        if (radicand < 0.0) {
            const double rounded = std::round(degree);
            if (std::fabs(rounded - degree) > 1e-9 || std::fmod(std::fabs(rounded), 2.0) < 1e-9) {
                set_error("Invalid root domain");
                return 0.0;
            }
            return checked(-std::pow(-radicand, 1.0 / degree));
        }
        return checked(std::pow(radicand, 1.0 / degree));
    }

    std::string expression_;
    std::size_t position_ = 0;
    AngleMode angle_mode_ = AngleMode::radians;
    std::string error_message_;
};

std::size_t current_number_start(const std::string &expression)
{
    if (expression.empty()) return 0;
    std::size_t index = expression.size();
    while (index > 0) {
        const char value = expression[index - 1];
        if (std::isdigit(static_cast<unsigned char>(value)) || value == '.') {
            --index;
            continue;
        }
        if ((value == 'e' || value == 'E') && index > 1) {
            --index;
            continue;
        }
        break;
    }
    return index;
}

}  // namespace

std::string format_value(double value)
{
    if (std::fabs(value) < 1e-12) value = 0.0;
    if (!std::isfinite(value)) return "Error";
    std::ostringstream stream;
    stream << std::setprecision(12) << value;
    return trim_trailing_zeros(stream.str());
}

EvaluationResult evaluate_expression(const std::string &expression, AngleMode angle_mode)
{
    const std::string normalized = close_open_parentheses(expression);
    ExpressionParser parser(normalized, angle_mode);
    auto result                  = parser.evaluate();
    result.normalized_expression = normalized;
    if (result.ok && !std::isfinite(result.value)) {
        result.ok            = false;
        result.error_message = "Result is not finite";
    }
    return result;
}

const std::string &CalculatorModel::expression() const noexcept
{
    return expression_;
}

const std::string &CalculatorModel::formula_text() const noexcept
{
    return formula_text_;
}

const std::string &CalculatorModel::display_text() const noexcept
{
    return display_text_;
}

const char *CalculatorModel::clear_label() const noexcept
{
    return expression_.empty() && !error_ ? "AC" : "C";
}

const char *CalculatorModel::angle_label() const noexcept
{
    return angle_mode_ == AngleMode::degrees ? "Deg" : "Rad";
}

char CalculatorModel::active_operator() const noexcept
{
    if (!just_evaluated_ && !expression_.empty() && is_binary_operator(expression_.back())) return expression_.back();
    return '\0';
}

AngleMode CalculatorModel::angle_mode() const noexcept
{
    return angle_mode_;
}

bool CalculatorModel::second_mode() const noexcept
{
    return second_mode_;
}

bool CalculatorModel::has_memory() const noexcept
{
    return std::fabs(memory_value_) >= 1e-12;
}

double CalculatorModel::memory_value() const noexcept
{
    return memory_value_;
}

void CalculatorModel::begin_input()
{
    if (error_ || just_evaluated_) {
        expression_.clear();
        formula_text_.clear();
        display_text_   = "0";
        error_          = false;
        just_evaluated_ = false;
    }
}

void CalculatorModel::append_value_token(const std::string &token, bool number_continuation)
{
    if (token.empty() || expression_.size() + token.size() >= maximum_expression_length) return;
    if (!expression_.empty() && is_value_ending(expression_.back()) && !number_continuation &&
        expression_.size() + token.size() + 1 < maximum_expression_length) {
        expression_.push_back('*');
    }
    expression_ += token;
    refresh_preview();
}

void CalculatorModel::input_digit(char digit)
{
    if (digit < '0' || digit > '9') return;
    begin_input();
    const std::size_t number_start = current_number_start(expression_);
    if (expression_.substr(number_start) == "0") {
        if (digit != '0') expression_[number_start] = digit;
        refresh_preview();
        return;
    }
    append_value_token(std::string(1, digit), true);
}

void CalculatorModel::input_decimal()
{
    begin_input();
    const std::size_t number_start = current_number_start(expression_);
    if (expression_.find('.', number_start) != std::string::npos) return;
    if (expression_.empty() || is_binary_operator(expression_.back()) || expression_.back() == '(' ||
        expression_.back() == ',') {
        append_value_token("0.", true);
    } else if (is_value_ending(expression_.back()) && !std::isdigit(static_cast<unsigned char>(expression_.back()))) {
        append_value_token("0.", false);
    } else {
        append_value_token(".", true);
    }
}

void CalculatorModel::input_exponent()
{
    begin_input();
    if (expression_.size() + 1 >= maximum_expression_length || expression_.empty() ||
        !std::isdigit(static_cast<unsigned char>(expression_.back())))
        return;
    const auto number_start = current_number_start(expression_);
    if (expression_.find_first_of("eE", number_start) == std::string::npos) {
        expression_.push_back('e');
        refresh_formula();
    }
}

void CalculatorModel::set_binary_operator(char operation)
{
    if (!is_binary_operator(operation)) return;
    if (error_) clear();
    if (just_evaluated_) {
        expression_     = display_text_;
        just_evaluated_ = false;
    }
    if (expression_.empty()) {
        if (operation == '-') expression_ = "-";
        refresh_formula();
        return;
    }
    if (is_binary_operator(expression_.back())) {
        if (expression_.size() == 1 && expression_.front() == '-') return;
        expression_.back() = operation;
    } else if (expression_.back() != '(' && expression_.back() != ',') {
        expression_.push_back(operation);
    }
    refresh_formula();
}

void CalculatorModel::open_parenthesis()
{
    begin_input();
    append_value_token("(", false);
}

void CalculatorModel::close_parenthesis()
{
    if (error_ || expression_.empty() || is_binary_operator(expression_.back())) return;
    const int opens  = static_cast<int>(std::count(expression_.begin(), expression_.end(), '('));
    const int closes = static_cast<int>(std::count(expression_.begin(), expression_.end(), ')'));
    if (opens > closes && expression_.size() + 1 < maximum_expression_length) {
        expression_.push_back(')');
        refresh_preview();
    }
}

void CalculatorModel::insert_constant(const std::string &constant)
{
    if (constant != "pi" && constant != "e") return;
    begin_input();
    append_value_token(constant, false);
}

void CalculatorModel::apply_function(const std::string &function)
{
    if (error_) clear();
    const std::string argument = expression_.empty() ? "0" : expression_;
    if (function.size() + argument.size() + 2 >= maximum_expression_length) return;
    expression_     = function + "(" + argument + ")";
    just_evaluated_ = false;
    refresh_preview();
}

void CalculatorModel::apply_power(int exponent)
{
    if (error_) clear();
    if (expression_.empty()) expression_ = "0";
    const std::string exponent_text = std::to_string(exponent);
    if (expression_.size() + exponent_text.size() + 3 >= maximum_expression_length) return;
    expression_     = "(" + expression_ + ")^" + exponent_text;
    just_evaluated_ = false;
    refresh_preview();
}

void CalculatorModel::apply_base_power(int base)
{
    if (error_) clear();
    if (expression_.empty()) expression_ = "0";
    const std::string base_text = std::to_string(base);
    if (expression_.size() + base_text.size() + 3 >= maximum_expression_length) return;
    expression_     = base_text + "^(" + expression_ + ")";
    just_evaluated_ = false;
    refresh_preview();
}

void CalculatorModel::begin_root()
{
    if (error_) clear();
    if (expression_.empty()) expression_ = "0";
    if (expression_.size() + 6 >= maximum_expression_length) return;
    expression_     = "root(" + expression_ + ",";
    just_evaluated_ = false;
    refresh_formula();
}

void CalculatorModel::factorial()
{
    if (error_ || expression_.size() + 1 >= maximum_expression_length || expression_.empty() ||
        is_binary_operator(expression_.back()))
        return;
    expression_.push_back('!');
    just_evaluated_ = false;
    refresh_preview();
}

void CalculatorModel::percent()
{
    if (error_ || expression_.size() + 1 >= maximum_expression_length || expression_.empty() ||
        is_binary_operator(expression_.back()))
        return;
    expression_.push_back('%');
    just_evaluated_ = false;
    refresh_preview();
}

void CalculatorModel::toggle_sign()
{
    if (error_) clear();
    if (expression_.empty()) {
        expression_ = "-";
        refresh_formula();
        return;
    }
    if (expression_.size() > 3 && expression_.compare(0, 2, "-(") == 0 && expression_.back() == ')')
        expression_ = expression_.substr(2, expression_.size() - 3);
    else if (expression_.size() + 3 < maximum_expression_length)
        expression_ = "-(" + expression_ + ")";
    just_evaluated_ = false;
    refresh_preview();
}

void CalculatorModel::random_value()
{
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    begin_input();
    append_value_token(format_value(distribution(generator)), false);
}

void CalculatorModel::backspace()
{
    if (error_) {
        clear();
        return;
    }
    if (just_evaluated_) just_evaluated_ = false;
    if (!expression_.empty()) expression_.pop_back();
    if (expression_.empty()) display_text_ = "0";
    refresh_preview();
}

void CalculatorModel::clear()
{
    expression_.clear();
    formula_text_.clear();
    display_text_   = "0";
    just_evaluated_ = false;
    error_          = false;
}

void CalculatorModel::equals()
{
    if (expression_.empty() || error_) return;
    const auto result = evaluate_expression(expression_, angle_mode_);
    if (!result.ok) {
        formula_text_ = prettify(expression_);
        display_text_ = "Error";
        error_        = true;
        return;
    }
    formula_text_   = prettify(result.normalized_expression) + " =";
    display_text_   = format_value(result.value);
    expression_     = display_text_;
    just_evaluated_ = true;
    error_          = false;
}

void CalculatorModel::toggle_angle_mode()
{
    angle_mode_ = angle_mode_ == AngleMode::radians ? AngleMode::degrees : AngleMode::radians;
    if (!expression_.empty() && !just_evaluated_) refresh_preview();
}

void CalculatorModel::toggle_second_mode()
{
    second_mode_ = !second_mode_;
}

void CalculatorModel::memory_clear() noexcept
{
    memory_value_ = 0.0;
}

void CalculatorModel::memory_add()
{
    double value = 0.0;
    if (current_value(value)) memory_value_ += value;
}

void CalculatorModel::memory_subtract()
{
    double value = 0.0;
    if (current_value(value)) memory_value_ -= value;
}

void CalculatorModel::memory_recall()
{
    begin_input();
    const std::string value =
        memory_value_ < 0.0 ? "(" + format_value(memory_value_) + ")" : format_value(memory_value_);
    append_value_token(value, false);
}

void CalculatorModel::refresh_preview()
{
    refresh_formula();
    if (expression_.empty()) {
        display_text_ = "0";
        return;
    }
    const auto result = evaluate_expression(expression_, angle_mode_);
    if (result.ok) display_text_ = format_value(result.value);
}

void CalculatorModel::refresh_formula()
{
    formula_text_ = prettify(expression_);
}

bool CalculatorModel::current_value(double &value) const
{
    if (error_ || expression_.empty()) return false;
    const auto result = evaluate_expression(expression_, angle_mode_);
    if (!result.ok) return false;
    value = result.value;
    return true;
}

}  // namespace calculator
