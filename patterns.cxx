#include <iostream>
#include <tuple>
#include <variant>
#include <list>
#include <string>
#include <initializer_list>
#include <algorithm>
#include <functional>

const bool debug = false;

class expression {
    std::variant<std::list<expression>, std::string> v_;
public:
    expression(const std::list<expression>& v): v_(v) {
    }
    expression(const std::string& s): v_(s) {
    }
    expression(const char* szVariant): v_(std::string(szVariant)) {
    }
    expression(std::initializer_list<expression> l):v_(std::list<expression>(l)) {
    }


    template<typename T>
    void visit(T visitor) const {
        std::visit(visitor, v_);
    }
};


class expression_printer {
    std::ostream& os_;
public:
    expression_printer(std::ostream& os): os_(os) {
    }

    void operator()(const std::list<expression>& l)  {
        expression_printer prnt(os_);
        os_ << "(";
        bool first = true;
        for(auto v: l)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                os_ << " ";
            }
            v.visit(prnt);
        }
        os_ << ")";
    }
    void operator()(const std::string& s)  {
        os_ << s;
    }
};

std::ostream& operator<<(std::ostream& os, const expression& expr) {
    expression_printer printer(os);
    expr.visit(printer);
    return os;
}


class IMatcher
{
public:

    virtual void peel_arg(const std::string& arg, std::function<void(IMatcher& remainder)>) = 0;
    virtual void peel_arg(const std::list<expression>& arg, std::function<void(IMatcher& remainder)>) = 0;
    virtual void finish_if_done() = 0;
};

void walk_iterator(
    std::list<expression>::const_iterator expr_begin,
    std::list<expression>::const_iterator expr_end,
    std::list<expression>::const_iterator patt_begin,
    std::list<expression>::const_iterator patt_end,
    IMatcher& matches
);

void walk_hierarchy(
    const expression& expr,
    const expression& patt,
    IMatcher& matches
);


class iterator_matcher : public IMatcher
{
    std::list<expression>::const_iterator expr_begin_;
    std::list<expression>::const_iterator expr_end_;
    std::list<expression>::const_iterator patt_begin_;
    std::list<expression>::const_iterator patt_end_;
    IMatcher& matches_;

public:
    iterator_matcher(
    std::list<expression>::const_iterator expr_begin,
    std::list<expression>::const_iterator expr_end,
    std::list<expression>::const_iterator patt_begin,
    std::list<expression>::const_iterator patt_end,
    IMatcher& matches):
        expr_begin_(expr_begin),
        expr_end_(expr_end),
        patt_begin_(patt_begin),
        patt_end_(patt_end),
        matches_(matches)
    {
    }
        
    void peel_arg(const std::string& arg, std::function<void(IMatcher&)> remainder_function)
    {
        matches_.peel_arg(arg, std::function([&](IMatcher& remaining_args){

            iterator_matcher next(
                expr_begin_,
                expr_end_,
                patt_begin_,
                patt_end_,
                remaining_args);
            
            remainder_function(next);
        }));
    }

    void peel_arg(const std::list<expression>& arg, std::function<void(IMatcher& )> remainder_function)
    {
        matches_.peel_arg(arg, std::function([&](IMatcher& remaining_args){
            iterator_matcher next(
                expr_begin_,
                expr_end_,
                patt_begin_,
                patt_end_,
                remaining_args);
            
            remainder_function(next);
        }));
    }
    void finish_if_done()
    {
        expr_begin_ ++;
        patt_begin_ ++;
        walk_iterator(
            expr_begin_,
            expr_end_,
            patt_begin_,
            patt_end_,
            matches_);
    }
};


void walk_iterator(
    std::list<expression>::const_iterator expr_begin,
    std::list<expression>::const_iterator expr_end,
    std::list<expression>::const_iterator patt_begin,
    std::list<expression>::const_iterator patt_end,
    IMatcher& matches)
{
    if constexpr (debug) {
        std::cout <<"walk_iterator(";
        if (expr_begin == expr_end) {
            std::cout << "-";
        }
        else
        {
            std::cout << *expr_begin;
        }
        std::cout << ",";
        if (patt_begin == patt_end) {
            std::cout << "-";
        }
        else
        {
            std::cout << *patt_begin;
        }
        std::cout << ")" << std::endl;
    }

    if ((expr_begin == expr_end) && (patt_begin == patt_end)) {
        matches.finish_if_done();
    }
    else if ((expr_begin == expr_end) && (patt_begin == patt_end)) {
        return;
    }
    else {

        iterator_matcher m1(
            expr_begin,
            expr_end,
            patt_begin,
            patt_end,
            matches);

        walk_hierarchy(
            *expr_begin,
            *patt_begin,
            m1);
    }
}

void walk_hierarchy(
    const expression& expr,
    const expression& patt,
    IMatcher& matches
)
{
    if constexpr (debug) {
        std::cout <<"walk_heirarchy(" << expr << "," << patt << ")" << std::endl;
    }
    expr.visit([&](auto&& typed_expr) {
        patt.visit([&](auto&& typed_patt) {
            using TExpr = std::decay_t<decltype(typed_expr)>;
            using TPatt = std::decay_t<decltype(typed_patt)>;
            if constexpr (std::is_same_v<TPatt, std::string>)
            {
                if (typed_patt == "var")
                {
                    matches.peel_arg(typed_expr, [](auto && remainder) {
                        remainder.finish_if_done();
                    });
                }
                else if constexpr(std::is_same_v<TExpr, std::string>)
                {
                    if (typed_expr == typed_patt)
                    {
                        matches.finish_if_done();
                    }
                }
            }
            else if constexpr(std::is_same_v<TPatt, std::list<expression>> && std::is_same_v<TExpr, std::list<expression>>)
            {
                walk_iterator(
                    typed_expr.begin(), typed_expr.end(),
                    typed_patt.begin(), typed_patt.end(),
                    matches
                );
            }
        });
    });
}


template<typename T, typename...TS>
class function_matcher_var;

class function_matcher;

function_matcher get_function_matcher(std::function<void()> func);

template<typename T, typename...TS>
function_matcher_var<T, TS...> get_function_matcher(std::function<void(const T&, const TS&...)> func);

class function_matcher: public IMatcher
{
    bool fail_ = false;
    std::function<void()> function_;
public:

    function_matcher(std::function<void()> function): function_(function)
    {
    }
    void peel_arg(const std::string &expr, std::function<void(IMatcher& )> remainder)
    {
        fail_ = true;
    }

    void peel_arg(const std::list<expression> &expr, std::function<void(IMatcher& remainder)> remainder)
    {
        fail_ = true;
    }

    void finish_if_done()
    {
        if(fail_)
            return;
        function_();
    }
};

template<typename T, typename...TS>
class function_matcher_var : public IMatcher
{
    bool fail_ = false;
    std::function<void(const T& ,const TS&...)> function_;
public:

    function_matcher_var(std::function<void(const T& ,const TS&...)> function): function_(function)
    {

    }
    void peel_arg(const std::string &expr, std::function<void(IMatcher&)> remainder)
    {
        if constexpr (std::is_same_v<std::string, T>)
        {
            auto matcher = get_function_matcher(std::function([&](const TS&...ts) {
                function_(expr, ts...);
            }));
            remainder(matcher);
        }
        else
        {
            fail_ = true;
        }
    }
    void peel_arg(const std::list<expression> &expr, std::function<void(IMatcher&)> remainder)
    {
        if constexpr (std::is_same_v<std::list<expression>, T>)
        {
            auto matcher = get_function_matcher(std::function([&](const TS&...ts) {
                function_(expr, ts...);
            }));
            remainder(matcher);
        }
        else
        {
            fail_ = true;
        }
    }


    void finish_if_done()
    {
        fail_ = true;
    }
};

function_matcher get_function_matcher(std::function<void()> func)
{
    return function_matcher(func);
}

template<typename T, typename...TS>
function_matcher_var<T, TS...> get_function_matcher(std::function<void(const T&, const TS&...)> func)
{
    return function_matcher_var<T, TS...>(func);
}


template<typename...T>
void match(const expression& expr, const expression& patt, std::function<void(const T&...)> func) {
    auto matcher = get_function_matcher(func);
    walk_hierarchy(expr, patt, matcher);
}

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

    test_match("expr", "var", std::function([](const std::list<expression>& v) {
        std::cout << "fail:" << std::endl;
    }));

    test_match("expr", {"var"}, std::function([](const std::string& v) {
        std::cout << "fail" << std::endl;
    }));

    test_match({"expr"}, "var", std::function([](const std::string& v) {
        std::cout << "fail" << std::endl;

    }));

    test_match({"expr"}, "var", std::function([](const std::list<expression>& v) {
        std::cout << "ok" << std::endl;
    }));

    test_match({"id", "identifier"}, {"id", "var"}, std::function([](const std::string& id){
        std::cout << id << std::endl;
    }));

    test_match({{{{"id"}, "id"}}},{{{{"id"}, "id"}}}, std::function([](){}));

    test_match({{{{"id"}, "id"}}},{{{{"var"}, "var"}}}, std::function([](const std::string&, const std::string&){}));

    return 0;
}
