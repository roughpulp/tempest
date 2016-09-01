#include <iostream>
#include <string>

namespace rpulp { namespace tempest {

void mmath_ut();
}}

using namespace rpulp::tempest;

int main()
{
	std::cout << "tempest-codec UTs" << std::endl;
	mmath_ut();
}
