#include <iostream>
#include <string>
#include <assert.h>

#include "mmath.h"

namespace rpulp { namespace tempest {

void rational_ut() {

	std::cout << "rational_ut ..." << std::endl;

	std::cout << "rational_ut == ..." << std::endl;
	{
		assert(Rational(45,10) == Rational(45,10));
		assert(Rational(46,10) != Rational(45,10));
		assert(Rational(4,8) == Rational(1,2));
	}
	
	std::cout << "rational_ut add ..." << std::endl;
	{
		assert( Rational(1, 1) + Rational(1, 1) == Rational(2,1) );
		assert( Rational(3, 4) + Rational(2, 3) == Rational(17, 12) );
	}
	
	std::cout << "rational_ut done" << std::endl;
}

void mmath_ut() {
	rational_ut();
}

}}