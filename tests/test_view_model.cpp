#include "pages/calculator/calculator_view_model.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

int main()
{
    lv_init();
    {
        CalculatorViewModel view_model;
        assert(std::strcmp(view_model.display_text(), "0") == 0);
        assert(std::strcmp(lv_subject_get_string(view_model.display_subject()), "0") == 0);

        view_model.perform(CalculatorCommand::digit, '8');
        view_model.perform(CalculatorCommand::multiply);
        assert(view_model.active_operator() == '*');
        view_model.perform(CalculatorCommand::digit, '9');
        view_model.perform(CalculatorCommand::equals);
        assert(std::strcmp(view_model.display_text(), "72") == 0);
        assert(std::strcmp(lv_subject_get_string(view_model.display_subject()), "72") == 0);
        assert(std::strcmp(lv_subject_get_string(view_model.formula_subject()), "8 × 9 =") == 0);

        view_model.perform(CalculatorCommand::memory_add);
        assert(view_model.has_memory());
        assert(std::fabs(view_model.memory_value() - 72.0) < 1e-9);
        view_model.perform(CalculatorCommand::memory_clear);
        assert(!view_model.has_memory());

        view_model.perform(CalculatorCommand::toggle_second);
        assert(view_model.second_mode());
        view_model.perform(CalculatorCommand::toggle_angle);
        assert(std::strcmp(view_model.angle_label(), "Deg") == 0);
        assert(std::strcmp(lv_subject_get_string(view_model.angle_subject()), "Deg") == 0);

        view_model.perform(CalculatorCommand::clear);
        assert(std::strcmp(view_model.display_text(), "0") == 0);
        assert(std::strcmp(view_model.clear_label(), "AC") == 0);
    }
    lv_deinit();
    return 0;
}
