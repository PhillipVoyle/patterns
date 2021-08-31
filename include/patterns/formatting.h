#pragma once
#include "patterns.h"
#include <iostream>

namespace pvoyle {
    namespace patterns {
        class expression_printer {
            std::ostream& os_;
        public:
            expression_printer(std::ostream& os): os_(os) {
            }

            void operator()(const std::vector<expression>& l)  {
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

            void operator()(int n) {
                os_ << n;
            }
        };

        std::ostream& operator<<(std::ostream& os, const expression& expr) {
            expression_printer printer(os);
            expr.visit(printer);
            return os;
        }
    }
}