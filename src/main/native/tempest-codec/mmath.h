#ifndef RPULP_TEMPEST_MMATH_H
#define RPULP_TEMPEST_MMATH_H

namespace rpulp { namespace tempest {

class Math {
public:
	static int gcd(int a, int b);
	static int lcm(int a, int b);
};

class Rational {
public:
	Rational(): num_(0), den_(1) {}
		
	Rational(const Rational& that): num_(that.num_), den_(that.den_) {}
	
	Rational(int num, int denom) {
		int gcd = Math::gcd(num, denom);
		this->num_ = num / gcd;
		this->den_ = denom / gcd;
	}
	
	int num() const { return num_; }
	int den() const { return den_; }
	
	Rational& operator=(const Rational& that);

	bool operator==(const Rational& that) const {
		return this->num_ == that.num_ && this->den_ == that.den_;
	}
	
	bool operator!=(const Rational& that) const {
		return !(*this == that);
	}
	
	Rational operator+(const Rational& that) const;
	
	void operator+=(const Rational& that);

private:
	int num_;
	int den_;
};

}}

#endif