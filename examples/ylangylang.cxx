#include <patterns/patterns.h>
#include <patterns/formatting.h>

using namespace pvoyle::patterns;


template<typename...TS>
bool is_match(const expression& expr, const expression& pattern, std::function<void(const TS&...)> func)
{
    bool result = false;

    match(expr, pattern, std::function([&] (const TS&...ts) {
        func(ts...);
        result = true;
    }));

    return result;
}

bool compile_statement(const expression& statement);

bool compile_statements(const std::vector<expression>& statements)
{
    for(const auto& statement: statements) {
        if (!compile_statement(statement)) {
            return false;
        }
    }
    return true;
}

bool compile_expression(const expression& expr) {
    return is_match(expr, "?", std::function([&](const std::string& s){
        std::cout << s;
    }));
}

bool compile_statement(const expression& statement) {
    bool statements_ok = false;
    if (is_match(statement, {"while", "?", "?"},
        std::function([&](
            const expression& predicate,
            const std::vector<expression>& statements){
            std::cout << "while(";
            statements_ok = compile_expression(predicate);
            std::cout << ") {\n";
            statements_ok = statements_ok && compile_statements(statements);
            std::cout << "}\n";
        })))
    {
        return statements_ok;
    }
    if (is_match(statement, {"if", "?", "?"},
        std::function([&](
            const expression& predicate,
            const std::vector<expression>& statements){
            std::cout << "if(";
            statements_ok = compile_expression(predicate);
            std::cout << ") {\n";
            statements_ok = statements_ok && compile_statements(statements);
            std::cout << "}\n";
        })))
    {
        return statements_ok;
    }

    if (is_match(statement, {"if", "?", "?", "?"},
        std::function([&](
            const expression& predicate,
            const std::vector<expression>& statements_if,
            const std::vector<expression>& statements_else){
            std::cout << "if";
            statements_ok = compile_expression(predicate);
            std::cout << ") {\n";
            statements_ok = statements_ok && compile_statements(statements_if);
            std::cout << "} else {\n";
            statements_ok = statements_ok && compile_statements(statements_else);
            std::cout << "}\n";
        })))
    {
        return statements_ok;
    }

    if (is_match(statement, "break",
        std::function([&](){
            std::cout << "break;\n";
        })))
    {
        return true;
    }

    bool expression_ok = false;
    if (is_match(statement, {"assign", "?", "?"},
        std::function([&](
            const expression& left,
            const expression& right){
            expression_ok = compile_expression(left);
            std::cout << "=";
            expression_ok = expression_ok && compile_expression(right);
            std::cout << ";\n";
        })))
    {
        return expression_ok;
    }

    if (is_match(statement, {"return", "?"},
        std::function([&](
            const expression& result){
            std::cout << "return ";
            expression_ok = compile_expression(result);
            std::cout << ";\n";
        })))
    {
        return expression_ok;
    }

    std::cerr << "malformed statement" << std::endl;
    return false;
}


int main(int argc, char** argv) {
    compile_statement({"while", "true", {{"if", "true", {{"assign", "a", "b"}, "break"}}, {"return", "0"}}});
    return 0;
}