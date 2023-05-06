#ifndef Hgcal10gLinkReceiver_HgcrocHalf_h
#define Hgcal10gLinkReceiver_HgcrocHalf_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cassert>

#include "HgcrocWord.h"

namespace Hgcal10gLinkReceiver {

  class HgcrocHalf {
  
  public:
    HgcrocHalf() {
    }
    
    void initialize(TRandom3 *r) {
      _rndm=r;
      
      _cmNoise=1.5;
      _cmSigma=1.0;
      _channelSigma[0]=1.5;
      _channelSigma[1]=1.0;

      for(unsigned i(0);i<2;i++) {
	_cmPedestal[i]=_rndm->Gaus(100.0,10.0);
      }

      for(unsigned i(0);i<37;i++) {
	_channelPedestal[i]=_rndm->Gaus(0.0,10.0);
      }
    }
    
    void simulate(bool start) {
      for(unsigned i(0);i<37;i++) {
	_channelSignal[i]=0.0;
      }

      double cmTrue;
      double cmValue[2];

      for(unsigned e(0);e<(start?2:1);e++) {
      cmTrue=_rndm->Gaus(0.0,_cmNoise);

      for(unsigned i(0);i<2;i++) {
	cmValue[i]=_cmPedestal[i]+cmTrue+_rndm->Gaus(0.0,_cmSigma);

	if(cmValue[i]<0.0) _commonMode[i]=0;
	else if(cmValue[i]>1023.0) _commonMode[i]=1023;
	else _commonMode[i]=uint16_t(cmValue[i]+0.5);
      }

      double channelValue[37];

      for(unsigned i(0);i<37;i++) {
	_hgcrocWord[i].setAdcM(_hgcrocWord[i].adc());

	double sigma((i==8 || i==17 || i==18 || i=19 || i==28)?_channelSigma[1]:_channelSigma[0]);
	channelValue[i]=_channelSignal[i]+_channelPedestal[i]
	  +0.5*(_cmPedestal[0]+_cmPedestal[1])+cmTrue+_rndm->Gaus(0.0,sigma);

	if(channelValue[i]<0.0) _hgcrocWord[i].setAdc(0);
	else if(channelValue[i]>1023.0) _hgcrocWord[i].setAdc(1023);
	else _hgcrocWord[i].setAdc(uint16_t(channelValue[i]+0.5));
      }
    }
    }

    uint16_t commonMode(unsigned c) const {
      if(c>=2) return 0;
      return _commonMode[c];
    }

    const uint32_t* hgcrocWords() const {
      return (const uint32_t*)_hgcrocWord;
    }

    /*    
    void setAdc(uint16_t a) {
      _data&=0xc00fffff;
      _data|=uint32_t(a&0x3ff)<<10;
    }

    void setToa(uint16_t a) {
      _data&=0xcffffc00;
      _data|=uint32_t(a&0x3ff);
    }

    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "HgcrocHalf::print() TP = " << tp() << ", TC = " << tc()
	<< ", ADC-1 = " << std::setw(4) << adcM()
	<< ", ADC = " << std::setw(10) << adc() 
	<< ", TOA = "<< std::setw(10) << toa() << std::endl;
    }
    */
  private:
    TRandom3 *_rndm;

    double _cmPedestal[2];
    double _channelPedestal[37];
    double _channelSignal[37];

    double _cmNoise;
    double _cmSigma;
    double _channelSigma[2];
    
    uint16_t _commonMode[2];
    HgcrocWord _hgcrocWord[37];
  };

}

#endif
