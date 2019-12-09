#include<string>
#include<cassert>
#include<iostream>
#include<sstream>
#include<vector>

#include "extended_variant.h"

#include<array>

void testing(const std::string& header){
    std::cout << "*** Testing " << header << " ***"  << '\n';
}

void test_passed(){
    std::cout << "*** Passed! ***" << '\n' << '\n';
}

int main(){
    using namespace std::string_literals;

    evariant<int, double, std::string> ev;

    testing("default construction");
    {
        static_assert(std::is_default_constructible_v<evariant<float, char, std::string>>);
        static_assert(std::is_default_constructible_v<evariant<int, double, std::string>>);
        static_assert(std::is_default_constructible_v<evariant<std::vector<int>, short>>);
        struct st{
            int i;
            st() = delete;
        };
        static_assert(!std::is_default_constructible_v<evariant<st, int, double>>);
    }
    test_passed();

    testing("value construction");
    {
        using EV = evariant<int, double, std::string>;
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
        static_assert(std::is_copy_constructible_v<evariant<float, char, std::string>>);
        static_assert(std::is_copy_constructible_v<evariant<int, double, std::string>>);
        static_assert(std::is_copy_constructible_v<evariant<std::vector<int>, short>>);
        struct st{
            int i;
            st() = default;
            st(int _i) : i{_i} {};
            st(st &) = delete;
        };
        using nocpy = evariant<st, std::string, double>;
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

    testing("comparison operators");
    {
        using EV = evariant<int, double, std::string>;
        EV a{5};
        EV b{5};
        assert(a==b);
        EV c{2.5};
        assert(a>2);
    }
    test_passed();

    testing("member functions");
    {
        std::stringstream ss;
        evariant<int, std::string> isev{"hello"};
        isev.visit([&](auto &&arg){ ss << arg;});
        std::string s;
        ss >> s;
        assert(s=="hello");

        evariant<int, double, char> idcev{'a'};
        assert(idcev.index() == 2);
        idcev = 2;
        assert(idcev.index() == 0);
        idcev = 2.5;
        assert(idcev.index() == 1);

        evariant<float, std::string> fsev{"hello"};
        assert(!fsev.template holds<float>());
        fsev = 2.2;
        assert(fsev.template holds<float>());

        evariant<long, std::string> lsev{1000l};
        assert(lsev.get_if<std::string>() == nullptr);
        assert(lsev.get_if<long>());

        evariant<short, const char*> scaev = "hello";
        assert(std::string{scaev.get<const char*>()} == "hello");
        bool errorThrown=false;
        try{
            scaev.get<short>();
        } catch(const std::bad_variant_access&) {
            errorThrown=true;
        }
        assert(errorThrown);

        evariant<int, double, std::string> idseva = "some string";
        evariant<int, double, std::string> idsevb = 2.5;
        idseva.swap(idsevb);
        assert(double(idseva) == 2.5);
        assert(std::string(idsevb) == "some string");
    }
    test_passed();

    testing("non-member functions");
    {
        ev = "visit_test";
        std::string s = std::visit([&](auto &&arg) -> std::string{
            std::stringstream ss;
            ss << arg;
            std::string sout;
            ss >> sout;
            return sout;
        }, ev);
        assert(s=="visit_test");
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

    testing("compile-time support");
    {
        constexpr evariant<int, double> a{5};
        constexpr evariant<int, double> b{5};
        static_assert(a == b);
        constexpr int c = a;
        static_assert(c == 5);
        constexpr double d = a;
        static_assert(d == 5.0);
    }
    test_passed();

    testing("h-containers");
    {
        hvector<int,double,std::string> hv = {
                2321,
                "string",
                328.32
        };

        int a = hv[0];
        std::string b = hv[1];
        double c = hv[2];

        assert(a == 2321);
        assert(b == "string");
        assert(c == 328.32);

        harray<3, long, std::string, char> ha = {
                12345l,
                "another string",
                'c'
        };

        int d = ha[0];
        std::string e = ha[1];
        char f = ha[2];

        assert(d == 12345);
        assert(e == "another string");
        assert(f == 'c');

    }
    test_passed();

    std::cout << "All tests passed" << '\n';

    return 0;
}