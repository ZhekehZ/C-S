#include <cassert>
#include <string>
#include <iostream>

#include "whatever.hpp"

using utils::whatever; 
using utils::whatever_cast; 
using utils::bad_whatever_cast; 
using std::string; 
using std::cout; 
using std::cerr; 
using std::endl; 

void contruct_test() 
{ 
    whatever def; 
    whatever copy_on_type(42); 
    def = 3.14; 
    def = string("2.71"); 
    whatever def_copy(def); 
    def = copy_on_type; 
    whatever e; 
    assert(e.empty()); 
} 

    template<class VALUE_OR_REF, class WHATEVER_TYPE>
void check_cast(WHATEVER_TYPE &a, bool should_throw) 
{
    bool thrown = false;
    try
    {
        VALUE_OR_REF res = whatever_cast<VALUE_OR_REF>(a);
        std::cout << res << std::endl;
    }
    catch(bad_whatever_cast const& err)
    {
        thrown = true;
        std::cerr << err.what() << std::endl;
    }
    assert(should_throw == thrown);
}

template<class VALUE_TYPE, class WHATEVER_PTR_TYPE> 
void check_ptr_cast(WHATEVER_PTR_TYPE a, bool should_fail)
{ 
    VALUE_TYPE *res = whatever_cast<VALUE_TYPE>(a);
    std::cout << res << std::endl;
    assert((should_fail && res == nullptr) ||
        (!should_fail && res != nullptr));
}

void retrieve_value_test() 
{ 
    whatever ia(42); 
    auto res = whatever_cast<double>(&ia); 
    assert(res == nullptr); 
    check_cast<double>(ia, true);
    check_cast<const double>(ia, true);
    check_cast<double&>(ia, true);
    check_cast<const double&>(ia, true);
    check_ptr_cast<double>(&ia, true);
    check_ptr_cast<const double>(&ia, true);

    check_cast<int> (ia, false);
    check_cast<const int> (ia, false);
    check_cast<int&> (ia, false);
    check_cast<const int&> (ia, false);
    check_ptr_cast<int>(&ia, false);
    check_ptr_cast<const int> (&ia, false);

    const whatever iac(ia);
    check_cast<double>(iac, true);
    check_cast<const double>(iac, true);
    check_cast<const double&>(iac, true);
    check_ptr_cast<const double>(&ia, true);

    check_cast<int>(iac, false);
    check_cast<const int>(iac, false);
    check_cast<const int&>(iac, false);
    check_ptr_cast<const int>(&iac, false);

    //Should fail in compile time:
//    check_cast<double&>(iac, true);
//    check_ptr_cast<double>(&iac, true);
//    check_cast<int&>(iac, true);
//    check_ptr_cast<int>(&iac, true);
}

void swap_test() 
{ 
    whatever a(5), b(string("6")); 
    swap(a, b);
    int &bref = whatever_cast<int&>(b);
    std::string &aref = whatever_cast<std::string&>(a);
    assert(bref == 5);
    assert(aref == "6");
    std::cout << bref << " " << aref << std::endl;
}

int main()
{
    contruct_test();
    retrieve_value_test();
    swap_test();

    {
        utils::whatever w1(42);
        utils::whatever w2;
        assert(*utils::whatever_cast<int>(&w1) == 42);
        assert(utils::whatever_cast<int>(&w2) == nullptr);
        w1 = w2;
        assert(utils::whatever_cast<int>(&w1) == nullptr);
        assert(utils::whatever_cast<int>(&w2) == nullptr);
    }

    {

        {
            utils::whatever const w(1);
//            (void)utils::whatever_cast<int&>(w);

            return 0;
        }

    }

    return 0;
}

