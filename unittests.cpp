/*=========================================================================

    Author:     Nova Systems UK
    Date:       03/12/2019
    Program:    Phoenix Paraview project
    Module:     unittests.cpp
    
    Copyright (c) 2019 Nova Systems UK. All rights reserved.

=========================================================================*/
#include<string>
#include<cassert>
#include<iostream>
#include<sstream>
#include<vector>

#include "extended_variant.h"

void testing(const std::string& header){
    std::cout << "*** Testing " << header << " ***"  << '\n';
}

void test_passed(){
    std::cout << "*** Passed! ***" << '\n' << '\n';
}

int main(){
    using namespace std::string_literals;

    extended_variant<int, double, std::string> ev;

    testing("default construction");
    {
        static_assert(std::is_default_constructible_v<extended_variant<float, char, std::string>>);
        static_assert(std::is_default_constructible_v<extended_variant<int, double, std::string>>);
        static_assert(std::is_default_constructible_v<extended_variant<std::vector<int>, short>>);
        struct st{
            int i;
            st() = delete;
        };
        static_assert(!std::is_default_constructible_v<extended_variant<st, int, double>>);
    }
    test_passed();

    testing("value construction");
    {
        using EV = extended_variant<int, double, std::string>;
        EV a{5};
        EV b{2.5};
        EV c{3.7f};
        EV d{'f'};
        EV e{0xab};
        EV f{"string literal"s};
        EV g{"c string"};
    }
    test_passed();

    testing("copy construction");
    {
        static_assert(std::is_copy_constructible_v<extended_variant<float, char, std::string>>);
        static_assert(std::is_copy_constructible_v<extended_variant<int, double, std::string>>);
        static_assert(std::is_copy_constructible_v<extended_variant<std::vector<int>, short>>);
        struct st{
            int i;
            st() = default;
            st(int _i) : i{_i} {};
            st(st &) = delete;
        };
        using nocpy = extended_variant<st, std::string, double>;
    }
    test_passed();

    testing("value assignment");
    {
        ev = 5.5;
        ev = 'a';
        ev = 2.3f;
        ev = "hello";
        ev = "string literal"s;
        ev = 100;
        ev = 0xff;
    }
    test_passed();

    testing("conversion");
    {
        int ii=3;
        ev = ii;
        assert(int(ev) == ii);
        assert(double(ev) == double(ii));
        assert(float(ev) == float(ii));
        assert(char(ev) == char(ii));
        assert(unsigned(ev) == unsigned(ii));
    }
    test_passed();

    testing("ostreaming");
    {
        ev = "ostream test";
        std::string s;
        std::stringstream ss;
        ss << ev;
        std::getline(ss, s);
        assert(s == "ostream test");
    }
    test_passed();

    testing("istreaming");
    {
        std::stringstream ss{"istream_test"};
        ss >> ev;
        std::string s = ev;
        assert(s == "istream_test");
    }
    test_passed();

    std::cout << "All tests passed" << '\n';

    return 0;
}