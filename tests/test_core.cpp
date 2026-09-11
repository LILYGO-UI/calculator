#include "pages/calculator/calculator_model.hpp"

#include <cassert>
#include <cmath>
#include <string>

namespace {

bool nearly_equal(double left, double right, double tolerance = 1e-9)
{
    return std::fabs(left - right) <= tolerance;
}

void test_expression_engine()
{
    using calculator::AngleMode;
    const auto precedence = calculator::evaluate_expression("1+2*3", AngleMode::radians);
    assert(precedence.ok && nearly_equal(precedence.value, 7.0));

    const auto parentheses = calculator::evaluate_expression("(2+3)*4", AngleMode::radians);
    assert(parentheses.ok && nearly_equal(parentheses.value, 20.0));

    const auto auto_close = calculator::evaluate_expression("2*(3+4", AngleMode::radians);
    assert(auto_close.ok && nearly_equal(auto_close.value, 14.0));

    const auto functions = calculator::evaluate_expression("sqrt(81)+3!+log(100)", AngleMode::radians);
    assert(functions.ok && nearly_equal(functions.value, 17.0));

    const auto degrees = calculator::evaluate_expression("sin(30)+cos(60)", AngleMode::degrees);
    assert(degrees.ok && nearly_equal(degrees.value, 1.0));

    const auto root = calculator::evaluate_expression("root(-27,3)", AngleMode::radians);
    assert(root.ok && nearly_equal(root.value, -3.0));

    assert(!calculator::evaluate_expression("1/0", AngleMode::radians).ok);
    assert(!calculator::evaluate_expression("sqrt(-1)", AngleMode::radians).ok);
    assert(!calculator::evaluate_expression("root(-16,2)", AngleMode::radians).ok);
    assert(calculator::format_value(1000.0) == "1000");
    assert(calculator::format_value(-0.0) == "0");
}

void test_calculator_workflow()
{
    calculator::CalculatorModel model;
    assert(model.display_text() == "0");
    assert(std::string(model.clear_label()) == "AC");

    model.input_digit('1');
    model.input_digit('2');
    model.set_binary_operator('+');
    assert(model.active_operator() == '+');
    model.input_digit('7');
    assert(model.display_text() == "19");
    model.equals();
    assert(model.display_text() == "19");
    assert(model.formula_text() == "12 + 7 =");

    model.input_digit('3');
    assert(model.display_text() == "3");
    model.toggle_sign();
    assert(model.display_text() == "-3");
    model.percent();
    assert(model.display_text() == "-0.03");

    model.clear();
    model.input_decimal();
    model.input_digit('5');
    model.set_binary_operator('*');
    model.input_digit('8');
    model.equals();
    assert(model.display_text() == "4");

    model.clear();
    model.input_digit('3');
    model.input_digit('0');
    model.toggle_angle_mode();
    model.apply_function("sin");
    assert(model.display_text() == "0.5");
    assert(std::string(model.angle_label()) == "Deg");

    model.clear();
    model.input_digit('8');
    model.input_digit('1');
    model.begin_root();
    model.input_digit('4');
    model.equals();
    assert(model.display_text() == "3");

    model.memory_add();
    assert(model.has_memory());
    assert(nearly_equal(model.memory_value(), 3.0));
    model.clear();
    model.memory_recall();
    assert(model.display_text() == "3");
    model.memory_subtract();
    assert(!model.has_memory());

    model.clear();
    model.input_digit('1');
    model.input_exponent();
    model.input_digit('3');
    model.set_binary_operator('+');
    model.input_digit('2');
    model.equals();
    assert(model.display_text() == "1002");
}

}  // namespace

int main()
{
    test_expression_engine();
    test_calculator_workflow();
    return 0;
}
