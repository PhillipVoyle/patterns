# A C++ pattern matching library

baseline repository here [https://github.com/PhillipVoyle/patterns](https://github.com/PhillipVoyle/patterns)

I made this library as an experiment in pattern matching. It's a header only library that will perform a unification-like structural pattern match on a string and list based expression tree, and give you back the variables that are matched. This is currently highly WIP, and I'd suggest you don't use it, but if you feel up to working with something that may contain problems, read on.

## C++ Version requirements
Currently this version requires a C++17 compiler. I haven't done any serious compatibilty testing but on my machine I'm running gnu g++ version 9.3.0

## Using the library in your project
First add the include directory to your project include paths, then include the header like so


```cpp
    #include <patterns/patterns.h>

    using namespace pvoyle::patterns;

```

## Creating expressions and patterns
Expressions and patterns share the same type `pvoyle::patterns::expression`, which can be constructed using the following mechanisms:

```cpp
    expression string_expr = "test"; // an expresson which is just a string
    expression list_expr = {"test", "test"}; // an expression which is a list containing two strings
    expression list_list_expr = {{"test", "test"}, {"test", "test"}}; // A more complicated list
```

Patterns can be made the same way, but if you use the special string `"var"` the pattern will attempt to match an expression at that node.

```cpp
    expression string_pattern = "test"; // a pattern that expects an exact match
    expression list_pattern = {"test", "var"}; // a pattern that allows one variable
```

## Performing matches
Matches of specific variables or strings are possible. There are two things to consider here, the parts of the structure you want to match, and whether you want to match a string, or a list. When calling match, you must specify a callback function which will be called in the event that your expression matches.

```cpp
    match({"a", "b", "c"}, {"a", "b", "var"}, std::function([&](const std::string& s) { std::cout << s << std::endl;})); //prints "c"
    match({"a", "b", "c"}, {"a", "b", "var"}, std::function([&](const std::vector<expression>& e) {})); //does not match

```

## Patches and pull requests

