#include <patterns/patterns.h>
#include <patterns/formatting.h>

using namespace pvoyle::patterns;

template<typename...T>
bool test_match(const expression& expr, const expression& patt, std::function<void(const T&...)> func) {

    bool called = false;

    auto matcher = get_function_matcher(std::function([&](const T&...t){
        func(t...);

        called = true;
    }));

    walk_hierarchy(expr, patt, matcher);

    if (called)
    {
        std::cout << "match was called:" << expr << ":" << patt << std::endl;
        return true;
    }
    else
    {
        std::cout << "match was not called:" << expr << ":" << patt << std::endl;
        return false;
    }
}

int main(int argc, char** argv) {

    test_match("test", "test",  std::function([]() {
        std::cout << "ok" << std::endl;
    }));

    test_match("expr", "var",  std::function([](const std::string& v) {
        std::cout << "ok:" << v << std::endl;
    }));

    test_match("expr", "var", std::function([](const std::vector<expression>& v) {
        std::cout << "fail:" << std::endl;
    }));

    test_match("expr", {"var"}, std::function([](const std::string& v) {
        std::cout << "fail" << std::endl;
    }));

    test_match({"expr"}, "var", std::function([](const std::string& v) {
        std::cout << "fail" << std::endl;

    }));

    test_match({"expr"}, "var", std::function([](const std::vector<expression>& v) {
        std::cout << "ok" << std::endl;
    }));

    test_match({"id", "identifier"}, {"id", "var"}, std::function([](const std::string& id){
        std::cout << id << std::endl;
    }));

    test_match({{{{"id"}, "id"}}},{{{{"id"}, "id"}}}, std::function([](){}));

    test_match({{{{"id"}, "id"}}},{{{{"var"}, "var"}}}, std::function([](const std::string&, const std::string&){}));

    test_match({"test", {"test", "test"}}, {"var", "var"}, std::function([&](const expression& a, const expression& b) {
      std::cout << "matched:" << a << ", and " << b << std::endl;
    }));

    return 0;
}
