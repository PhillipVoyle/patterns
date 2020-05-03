#include <patterns/patterns.h>
#include <patterns/formatting.h>
#include <sstream>
#include <map>

using namespace pvoyle::patterns;

std::stringstream out;

int label_id = 0;

std::string allocate_label() {
    return "xx_" + std::to_string(++label_id);
}

int stack_offset = 0;
std::map<std::string, int> variables;

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

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool compile_lexpression(const expression& expr) {

    bool is_ok = false;
    if(is_match(expr, "?", std::function([&](const std::string& s){        
        if (variables.find(s) == variables.end())
        {
            variables[s] = ++stack_offset;
            out << "\t/* allocate " << s << " */" << std::endl;
        }
        out << "\tmov %eax, " << variables[s] << "(%ebp)" << std::endl;
        is_ok = true;
    }))) {
        return is_ok;
    }
    return false;
}


bool compile_rexpression(const expression& expr) {

    if (is_match(expr, "true", std::function([&]() {
        out << "\tmov $1, %eax" << std::endl;
    }))) {
        return true;
    }

    if (is_match(expr, "false", std::function([&]() {
        out << "\tmov $0, %eax" << std::endl;
    }))) {
        return true;
    }

    bool is_ok = false;
    if (is_match(expr, "?", std::function([&](const std::string& s){        

        if (is_number(s))
        {
            out << "\tmov $" << s <<", %eax" << std::endl;
            is_ok = true;
            return;
        }

        if (variables.find(s) == variables.end())
        {
            std::cerr << "undefined variable: " << s << std::endl;
            return ;
        }
        int offs = variables[s];
        out << "\tmov " << offs <<"(%ebp), %eax" << std::endl;
        is_ok = true;
    }))) {
        return is_ok;
    }
    return false;
}

std::string while_bottom = "";

bool compile_statement(const expression& statement) {
    bool statements_ok = false;
    if (is_match(statement, {"while", "?", "?"},
        std::function([&](
            const expression& predicate,
            const std::vector<expression>& statements){

            std::string top = allocate_label() + "_while_top";
            std::string bottom = allocate_label() + "_while_bottom";

            while_bottom = bottom;
            out << top << ":" << std::endl;
            statements_ok = compile_rexpression(predicate);
            out << "\ttestl %eax, $1" << std::endl;
            out << "\tjne " << bottom << std::endl;
            statements_ok = statements_ok && compile_statements(statements);
            out << "\tjmp " << top << std::endl;
            out << bottom << ":" << std::endl;
            while_bottom = "";
        })))
    {
        return statements_ok;
    }
    if (is_match(statement, {"if", "?", "?"},
        std::function([&](
            const expression& predicate,
            const std::vector<expression>& statements){
            std::string bottom = allocate_label() + "_if_bottom";

            statements_ok = compile_rexpression(predicate);
            out << "\ttestl %eax, $1" << std::endl;
            out << "\tjne " << bottom << std::endl;
            statements_ok = statements_ok && compile_statements(statements);
            out << bottom << ":" << std::endl;
        })))
    {
        return statements_ok;
    }

    if (is_match(statement, {"if", "?", "?", "?"},
        std::function([&](
            const expression& predicate,
            const std::vector<expression>& statements_if,
            const std::vector<expression>& statements_else){
            std::string if_bottom = allocate_label() + "_if_bottom";
            std::string if_else = allocate_label() + "_if_else";
            statements_ok = compile_rexpression(predicate);
            out << "\ttestl %eax, $1" << std::endl;
            out << "\tjne " << if_else << std::endl;

            statements_ok = statements_ok && compile_statements(statements_if);
            out << "\tjmp " << if_bottom << std::endl;
            statements_ok = statements_ok && compile_statements(statements_else);
            out << if_bottom << ":" << std::endl;
        })))
    {
        return statements_ok;
    }

    if (is_match(statement, "break",
        std::function([&](){
            out << "\tjmp " << while_bottom << std::endl;
        })))
    {
        return (while_bottom != "");
    }

    bool expression_ok = false;
    if (is_match(statement, {"assign", "?", "?"},
        std::function([&](
            const expression& left,
            const expression& right){
            expression_ok = compile_rexpression(right);
            expression_ok = expression_ok && compile_lexpression(left);
        })))
    {
        return expression_ok;
    }

    if (is_match(statement, {"return", "?"},
        std::function([&](
            const expression& result){
            expression_ok = compile_rexpression(result);
            out << "\tret" << std::endl;
        })))
    {
        return expression_ok;
    }

    std::cerr << "malformed statement" << std::endl;
    return false;
}

bool compile_program(const std::vector<expression>& expr)
{
    out << ".global start" << std::endl;
    out << "start:" << std::endl;
    out << "\tmov %esp, %ebp" << std::endl;
    if (!compile_statements(expr))
    {
        return false;
    }

    out << "end:" << std::endl;
    out << ".extern exit" << std::endl;
    out << "\tmov $0, eax" << std::endl;
    out << "\tcall exit" << std::endl;
    return true;
}

int main(int argc, char** argv) {
    if (compile_program({{"assign", "b", "1"}, {"while", "true", {{"if", "true", {{"assign", "a", "b"}, "break"}}}}, {"return", "0"}}))
    {
        std::cout << out.str();
    }
    return 0;
}