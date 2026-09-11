#include "pages/calculator/calculator_view_model.hpp"

CalculatorViewModel::CalculatorViewModel()
{
    publish();
}

const char *CalculatorViewModel::display_text() const noexcept
{
    return model_.display_text().c_str();
}

const char *CalculatorViewModel::formula_text() const noexcept
{
    return model_.formula_text().c_str();
}

const char *CalculatorViewModel::clear_label() const noexcept
{
    return model_.clear_label();
}

const char *CalculatorViewModel::angle_label() const noexcept
{
    return model_.angle_label();
}

char CalculatorViewModel::active_operator() const noexcept
{
    return model_.active_operator();
}

bool CalculatorViewModel::second_mode() const noexcept
{
    return model_.second_mode();
}

bool CalculatorViewModel::has_memory() const noexcept
{
    return model_.has_memory();
}

double CalculatorViewModel::memory_value() const noexcept
{
    return model_.memory_value();
}

lv_subject_t *CalculatorViewModel::display_subject() noexcept
{
    return display_.get();
}

lv_subject_t *CalculatorViewModel::formula_subject() noexcept
{
    return formula_.get();
}

lv_subject_t *CalculatorViewModel::clear_subject() noexcept
{
    return clear_.get();
}

lv_subject_t *CalculatorViewModel::angle_subject() noexcept
{
    return angle_.get();
}

void CalculatorViewModel::perform(CalculatorCommand command, char digit)
{
    switch (command) {
        case CalculatorCommand::digit:
            model_.input_digit(digit);
            break;
        case CalculatorCommand::decimal:
            model_.input_decimal();
            break;
        case CalculatorCommand::clear:
            model_.clear();
            break;
        case CalculatorCommand::backspace:
            model_.backspace();
            break;
        case CalculatorCommand::toggle_sign:
            model_.toggle_sign();
            break;
        case CalculatorCommand::percent:
            model_.percent();
            break;
        case CalculatorCommand::divide:
            model_.set_binary_operator('/');
            break;
        case CalculatorCommand::multiply:
            model_.set_binary_operator('*');
            break;
        case CalculatorCommand::subtract:
            model_.set_binary_operator('-');
            break;
        case CalculatorCommand::add:
            model_.set_binary_operator('+');
            break;
        case CalculatorCommand::equals:
            model_.equals();
            break;
        case CalculatorCommand::open_parenthesis:
            model_.open_parenthesis();
            break;
        case CalculatorCommand::close_parenthesis:
            model_.close_parenthesis();
            break;
        case CalculatorCommand::memory_clear:
            model_.memory_clear();
            break;
        case CalculatorCommand::memory_add:
            model_.memory_add();
            break;
        case CalculatorCommand::memory_subtract:
            model_.memory_subtract();
            break;
        case CalculatorCommand::memory_recall:
            model_.memory_recall();
            break;
        case CalculatorCommand::toggle_second:
            model_.toggle_second_mode();
            break;
        case CalculatorCommand::square:
            model_.apply_power(2);
            break;
        case CalculatorCommand::cube:
            model_.apply_power(3);
            break;
        case CalculatorCommand::power:
            model_.set_binary_operator('^');
            break;
        case CalculatorCommand::exponential:
            model_.apply_function("exp");
            break;
        case CalculatorCommand::ten_power:
            model_.apply_base_power(10);
            break;
        case CalculatorCommand::reciprocal:
            model_.apply_power(-1);
            break;
        case CalculatorCommand::square_root:
            model_.apply_function("sqrt");
            break;
        case CalculatorCommand::cube_root:
            model_.apply_function("cbrt");
            break;
        case CalculatorCommand::root:
            model_.begin_root();
            break;
        case CalculatorCommand::natural_log:
            model_.apply_function("ln");
            break;
        case CalculatorCommand::common_log:
            model_.apply_function("log");
            break;
        case CalculatorCommand::factorial:
            model_.factorial();
            break;
        case CalculatorCommand::sine:
            model_.apply_function(model_.second_mode() ? "asin" : "sin");
            break;
        case CalculatorCommand::cosine:
            model_.apply_function(model_.second_mode() ? "acos" : "cos");
            break;
        case CalculatorCommand::tangent:
            model_.apply_function(model_.second_mode() ? "atan" : "tan");
            break;
        case CalculatorCommand::constant_e:
            model_.insert_constant("e");
            break;
        case CalculatorCommand::exponent:
            model_.input_exponent();
            break;
        case CalculatorCommand::toggle_angle:
            model_.toggle_angle_mode();
            break;
        case CalculatorCommand::hyperbolic_sine:
            model_.apply_function(model_.second_mode() ? "asinh" : "sinh");
            break;
        case CalculatorCommand::hyperbolic_cosine:
            model_.apply_function(model_.second_mode() ? "acosh" : "cosh");
            break;
        case CalculatorCommand::hyperbolic_tangent:
            model_.apply_function(model_.second_mode() ? "atanh" : "tanh");
            break;
        case CalculatorCommand::constant_pi:
            model_.insert_constant("pi");
            break;
        case CalculatorCommand::random:
            model_.random_value();
            break;
        case CalculatorCommand::absolute:
            model_.apply_function("abs");
            break;
    }
    publish();
}

void CalculatorViewModel::publish()
{
    display_.set(model_.display_text().c_str());
    formula_.set(model_.formula_text().c_str());
    clear_.set(model_.clear_label());
    angle_.set(model_.angle_label());
}
