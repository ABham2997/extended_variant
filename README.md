# extended_variant
A std::variant is a class template within the std namespace which represents a type-safe union. This means, at any given time, a std::variant can hold a value of one if its alternative types. Though inherently very useful, they are limited in their usefulness by the fact that the data can only be explicity accessed using non-member function templates such as std::get, and even then, std::get can only be used with the correct alternative type it holds without throwing an error:

```c++
#include<variant>
#include<string>

std::variant<std::string, int, double> var = 50; //initialise variant with int

int intData = std::get<int>(var); //OK, this works

std::string stringData = std::get<std::string>(var); //ERROR, this doesn't work, which is fair enough

double doubleData = std::get<double>(var); //ERROR, this doesn't work either, even though intuitively it should

float floatData = std::get<float>(var); //ERROR, similarly this doesn't work either, why not?

```
\
An extended_variant is a class template which seeks to overcome these limitation by allowing implicit conversion to any of its underlying types:
```c++

#include<string>

#include "extended_variant"

extended_variant<std::string, int, double> var = 25; //initialise variant with int

int intData = var; //OK, implicit conversion to int
std::cout << intData << '\n'; //prints 25

double doubleData = var; //Also OK, implicit conversion to double
std::cout << doubleData << '\n'; //prints 25

float floatData = var; //Also OK, implicit conversion to float
std::cout << floatData << '\n'; //prints 25

std::string stringData = var; //OK, but since std::strings can't be constructed from ints, stringData is default constructed

var = "this is a string";

stringData = var; //OK, implicit conversion to std::string
std::cout << stringData << '\n'; //prints "this is a string"

```
\
extended_variants are type safe and will prevent conversions to any type that cannot be constructed from the types it holds at compile-time:
```c++
#include<string>

#include "extended_variant"

extended_variant<int, double> var = 123; //initialise variant with double

std::string stringData = var; //ERROR, std::strings can't be constructed from ints or doubles -> compile-time error

char charData = var; //OK, a char can be constructed from an int
```

Within the extended_variant.h header file, the following useful template aliases have been declared:
```c++
#include<vector>
#include<array>
#include<string>

#include "extended_variant"

evariant<int, char, short> extendedVariant = 2.5;
//shorter, more convenient type alias for extended_variant

hvector<int, double, std::string> heterogeneousVector = {
    2.5, 42189, "string", "another string"
}; //alias for std::vector<extended_variant<int, double, std::string>>
//hvector is an incomplete type, requires vector header to be #include-d

std::string s1 = heterogeneousVector[3]; //OK, s1 holds "string"
std::string s2 = heterogeneousVector[2]; //OK, s2 holds "another string"
float f1 = heterogeneousVector[1]; //OK, f1 holds 42189
int i1 = heterogeneousVector[0]; //OK, i1 holds 2.5 (narrowing-conversion)

harray<5, long, char, std::string> heterogeneousArray = {
    123456l, 'c', "hello", 422, char(3)
}; //alias for std::array<extended_variant<long, char, std::string, 5>>
//harray, like hvector, is an incomplete type and required array header to be #include-d

```
\
Like std::variants, extended_variants have compile-time support:
```c++
#include<string_view>

#include "extended_variant"

constexpr evariant<int, double, std::string_view> var = "compile-time string";

constexpr std::string_view sv = var; //OK, sv holds "compile-time string"

var = 123.456;

constexpr int i = var;

static_assert(i==123); //OK

constexpr double d = var;

static_assert(d==123.456); //OK
```
