#include "mmath.h"

#include <cmath>

namespace rpulp { namespace tempest {

int Math::gcd(int a, int b) {
	for (;;) {
		if (a == 0) {
			return b;
		}
		b %= a;
		if (b == 0) {
			return a;
		}
		a %= b;
	}
}

int Math::lcm(int a, int b) {
	return abs(a * b) / Math::gcd(a, b);
}

Rational& Rational::operator=(Rational const& that) {
	this->num_ = that.num_;
	this->den_ = that.den_;
	return *this;
}

Rational Rational::operator+(Rational const& that) const{
	Rational sum = Rational(*this);
	sum += that;
	return sum;
}

void Rational::operator+=(Rational const& that) {
	int lden = Math::lcm(this->den_, that.den_);
	const int d0 = lden / this->den_;
	const int d1 = lden / that.den_;
	this->num_ = (this->num_ * d0) + (that.num_ * d1);
	this->den_ = lden;
}

}}