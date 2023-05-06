#ifndef Average_h
#define Average_h

#include <iostream>
#include <cmath>

class Average {

public:
  
  Average() {
    reset();
  }

  Average(unsigned long n, double a, double s) {
    _number=n;
    _sum=n*a;
    if(n>1) _sumOfSquares=(n-1.0)*s*s+n*a*a;
    else _sumOfSquares=_sum*_sum;
  }

  virtual ~Average() {
  }

  virtual std::ostream& print(std::ostream &o=std::cout) const {
    o << "Average::print()" << std::endl;
    o << " Number " << _number << ", sum " << _sum 
      << ", sum of squares " << _sumOfSquares << std::endl;
    o << " Average = " << average() << " +/- " << errorOnAverage() << std::endl;
    o << " Sigma = " << sigma() << " +/- " << errorOnSigma() << std::endl;
    return o;
  }

  virtual void reset() {
    _number=0;
    _sum=0.0;
    _sumOfSquares=0.0;
  }

  virtual void operator+=(double x) {
    _number++;
    _sum+=x;
    _sumOfSquares+=x*x;
  }

  virtual void operator-=(double x) {
    _number--;
    _sum-=x;
    _sumOfSquares-=x*x;
  }

  virtual void operator+=(const Average &a) {
    _number+=a._number;
    _sum+=a._sum;
    _sumOfSquares+=a._sumOfSquares;
  }

  virtual void event(double x) {
    operator+=(x);
  }

  virtual unsigned long number() const {
    return _number;
  }

  virtual double average() const {
    if(_number==0) return 0.0;
    return _sum/_number;
  }

  virtual double sigma() const {
    if(_number<=1) return -1.0;
    double dNumber(_number); // _number > 65k doesn't work due to being squared
    return sqrt((dNumber*_sumOfSquares-_sum*_sum)/(dNumber*(dNumber-1.0)));
  }

  virtual double errorOnAverage() const {
    if(_number<=1) return 0.0;
    double dNumber(_number);
    return sigma()/sqrt(dNumber);
  }

  virtual double errorOnSigma() const {
    if(_number<=1) return 0.0;
    double dNumber(_number);
    return sigma()/sqrt(2.0*(dNumber-1.0));
  }


private:
  unsigned long _number;
  double _sum;
  double _sumOfSquares;
};

#endif
