#include <iostream>
#include <iomanip>

//void C(bool f0, bool f1, bool f2, bool f3)
//{
//	std::cout << std::boolalpha << f0 << " " << f1 << " " << f2 << " " << f3
//			<< " \t\t  =>  " << ((f0 == f1) && (f1 == f2) && (f2 == f3))
//			<< std::endl;
//
//	;
//}
//
//int main(int argc, char** argv)
//{
//	C(true, true, true, true);
//	C(false, false, false, false);
//	C(true, true, true, false);
//	C(true, true, false, true);
//	C(true, false, true, true);
//	C(false, true, true, true);
//
//	C(false, true, true, false);
//	C(true, false, false, true);
//	C(true, false, true, false);
//	C(false, false, true, true);
//	C(true, true, false, false);
//
//}

#include <cstddef>
#include <limits>
#include <string>

struct Foo
{
	template<typename ...TI>
	int operator[](TI const & ...i)
	{
		return 10;
	}
}
;

int main()
{
	Foo foo;
	std::cout << foo[2.0, 3.0, 5.0] << std::endl;
}

// vim:et