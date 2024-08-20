//Comparison bather sorting for method explained in
//1. Wikipedia https://en.wikipedia.org/wiki/Batcher_odd%E2%80%93even_mergesort
//2. Stefan Bühler: https://gist.github.com/stbuehler/883635
//3. Danny Noonan: https://github.com/dnoonan08/ECONT_Emulator

// Author : Indranil Das

#include <vector>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <cmath>

/* Array abstraction so we don't have to copy arrays for odd/even/halves */

class TriggerCell{
public:
  TriggerCell () : energy(0), channel(0xffff) {};
  TriggerCell (uint16_t e, uint16_t ch) : energy(e), channel(ch) {};
  friend void swap(TriggerCell& lhs, TriggerCell& rhs){
    std::swap(lhs.energy, rhs.energy);
    std::swap(lhs.channel, rhs.channel);
  }
  friend bool operator<(const TriggerCell& lhs, const TriggerCell& rhs) {
    return lhs.energy < rhs. energy;
  }
  friend bool operator<=(const TriggerCell& lhs, const TriggerCell& rhs) {
    return lhs.energy <= rhs. energy;
  }
  friend bool operator>(const TriggerCell& lhs, const TriggerCell& rhs) {
    return lhs.energy > rhs. energy;
  }
  friend bool operator>=(const TriggerCell& lhs, const TriggerCell& rhs) {
    return lhs.energy > rhs. energy;
  }
  friend std::ostream& operator<<(std::ostream& os, TriggerCell const& atc){
    return os << "TriggerCell::energy =" << atc.getE() << ", TriggerCell::channel = " << atc.getCh() << std::endl;
  }
  void print(){
    std::cout << *this ;
  }
  uint16_t getCh() const {return channel;}
  uint16_t getE() const {return energy;}
  void setTc(uint16_t e, uint16_t ch) {energy = e; channel = ch;}
private:
  uint16_t energy, channel; 
};

class TriggerCellArray{
public:
  void print(){
    std::sort(tc.begin(), tc.end(), customLess);
    for(auto& itc: tc) itc.print();
  }
  TriggerCellArray () {tc.resize(0);}
  std::vector<TriggerCell>& getTc() {return tc;}
  TriggerCell& getTc(uint32_t i) {return tc.at(i);}
  void setTc(TriggerCell atc) {tc.push_back(atc);}
  void setTc(uint32_t energy, uint32_t channel) { TriggerCell atc ; atc.setTc(uint16_t(energy),uint16_t(channel)) ; tc.push_back(atc);}
  size_t size() {return tc.size();}
  void reset() {tc.resize(0);}
  TriggerCell& operator[](int index){
    if (index >= size()) {
      std::cout << "Array index out of bound, exiting";
      exit(0);
    }
    return tc[index];
  }
private:
  struct{
    bool operator()(TriggerCell& a, TriggerCell& b) const { return a.getCh() < b.getCh(); }
  }
  customLess;
  std::vector<TriggerCell> tc;
};

template<typename T>
class slice {
private:
  std::vector<T> *m_data;
  size_t m_start, m_step, m_len;
  bool m_is2pot; /* value may be false even if exists k: m_len = 2^k */

  slice(std::vector<T> *data, size_t start, size_t step, size_t len, bool is2pot)
    : m_data(data), m_start(start), m_step(step), m_len(len), m_is2pot(is2pot) {
  }

public:
  slice(std::vector<T> &data)
    : m_data(&data), m_start(0), m_step(1), m_len(data.size()), m_is2pot(false) {
  }

  slice()
    : m_data(0), m_start(0), m_step(0), m_len(0), m_is2pot(false) {
  }

  T& operator[](size_t i) {
    assert(m_data);
    assert(i < m_len);
    return (*m_data)[m_start + i*m_step];
  }

  const T& operator[](size_t i) const {
    assert(m_data);
    assert(i < m_len);
    return (*m_data)[m_start + i*m_step];
  }

  size_t size() { return m_len; }

  void print(){
    for(size_t is = 0 ; is < size() ; is++)
      (m_data->at(is)).print();
  }

  bool empty() { return 0 == m_len; }
  
  slice<T> even() {
    return slice<T>(m_data, m_start, 2*m_step, (m_len+1)/2, m_is2pot);
  }

  slice<T> odd() {
    return slice<T>(m_data, m_start+m_step, 2*m_step, m_len/2, m_is2pot);
  }
  
  /* first "half" has always a power-of-two size, otherwise oddevenmerge doesn't work */
  /* if input is guaranteed to have a power-of-two size this function can get a simpler implementation */
  std::pair< slice<T>, slice<T> > halves() {
    if (1 >= m_len) return std::make_pair(*this, slice<T>());
    if (m_is2pot) return std::make_pair( slice<T>(m_data, m_start, m_step, m_len/2, true), slice<T>(m_data, m_start + m_step * (m_len/2), m_step, m_len/2, true) );
    size_t len = 2;
    while (len < m_len) len <<= 1;
    len >>= 1;
    return std::make_pair( slice<T>(m_data, m_start, m_step, len, true), slice<T>(m_data, m_start + len*m_step, m_step, m_len - len, m_len == 2*len ) );
  }
};

template<typename T>
void sortTwo(slice<T> &data) {
  if (data.size() == 2) {
    if (data[0] > data[1]) {
      std::swap(data[0], data[1]);
    }
  }
}


/* assume input consists of two sorted "halves" (first half must be power of two) */
/* output is completely sorted */
template<typename T>
void oddevenmerge(slice<T> data) {
  if (data.size() <= 2) {
    sortTwo(data);
  } else {
    /* Run this in parallel if you want: */
    oddevenmerge(data.odd());
    oddevenmerge(data.even());
    /* Run this in parallel if you want: */
    for (size_t i = 2, len = data.size(); i < len; i += 2) {
      if (data[i-1] > data[i]) std::swap(data[i-1], data[i]);
    }
  }
}

int iloop = 0;
template<typename T>
void mergesort(slice<T> data) {
  if (data.size() <= 2) {
    sortTwo(data);
  } else {
    std::pair< slice<T>, slice<T> > halves = data.halves();
    /* Run this in parallel if you want: */
    mergesort(halves.first); /* first "half" always has a power-of-two size */
    iloop++;
    mergesort(halves.second); /* second is the remaining */
    oddevenmerge(data);
  }
}

template<typename T>
void mergesort(std::vector<T> &data) {
  slice<T> s(data);
  mergesort(s);
}

#include <algorithm>

#include <iostream>
template<typename T>
void print(T data) {
  std::cout << "Data:"; for (size_t i = 0; i < data.size(); i++) { std::cout << " " << data[i]; } std::cout << std::endl;
}

int main() {
	// std::vector<int> data(50);
	// for (size_t i = 0; i < data.size(); i++) data[i] = i;
	// std::random_shuffle(data.begin(), data.end());
	// print(data);
	// mergesort(data);
	// print(data);
	// for (int i = int(data.size())-1; i >= 0 ; i--)
	//   std::cout << " " << data[i];
	// std::cout << std::endl;
	
	const uint32_t nofTcs = 48;
	
	// const uint32_t TcECh[nofTcs][2] = { {13, 88},
	// 				    {32, 87},
	// 				    {2, 87},
	// 				    {34, 87},
	// 				    {9, 87},
	// 				    {18, 86},
	// 				    {11, 86},
	// 				    {27, 86},
	// 				    {25, 86},
	// 				    {43, 86},
	// 				    {4, 86},
	// 				    {0, 86},
	// 				    {36, 85},
	// 				    {41, 85},
	// 				    {29, 85},
	// 				    {20, 85},
	// 				    {45, 85},
	// 				    {16, 85},
	// 				    {12, 73},
	// 				    {10, 65},
	// 				    {24, 65},
	// 				    {33, 64},
	// 				    {35, 64},
	// 				    {1, 64},
	// 				    {39, 64},
	// 				    {5, 64},
	// 				    {26, 63},
	// 				    {40, 63},
	// 				    {38, 63},
	// 				    {14, 63},
	// 				    {8, 62},
	// 				    {46, 62},
	// 				    {44, 62},
	// 				    {42, 62},
	// 				    {37, 62},
	// 				    {31, 62},
	// 				    {30, 61},
	// 				    {28, 61},
	// 				    {15, 60},
	// 				    {19, 60},
	// 				    {7, 60},
	// 				    {3, 60},
	// 				    {47, 60},
	// 				    {17, 59},
	// 				    {22, 59},
	// 				    {6, 58},
	// 				    {21, 58},
	// 				    {23, 56}};
	
	const uint32_t TcECh[nofTcs][2] = { {18, 87},
					    {2, 87},
					    {16, 87},
					    {0, 87},
					    {4, 86},
					    {34, 86},
					    {32, 86},
					    {27, 86},
					    {9, 85},
					    {25, 85},
					    {11, 85},
					    {43, 85},
					    {45, 85},
					    {29, 85},
					    {20, 85},
					    {36, 84},
					    {13, 84},
					    {41, 84},
					    {10, 63},
					    {1, 63},
					    {35, 62},
					    {19, 62},
					    {12, 62},
					    {31, 61},
					    {21, 61},
					    {17, 61},
					    {37, 61},
					    {14, 61},
					    {7, 61},
					    {3, 61},
					    {24, 60},
					    {30, 60},
					    {26, 60},
					    {22, 60},
					    {15, 60},
					    {5, 60},
					    {6, 60},
					    {33, 59},
					    {23, 59},
					    {38, 59},
					    {46, 59},
					    {44, 59},
					    {42, 59},
					    {28, 58},
					    {8, 58},
					    {40, 58},
					    {39, 58},
					    {47, 57}};
	
	TriggerCellArray tcArray;
	for(uint32_t itc=0;itc<nofTcs;itc++) tcArray.setTc(TcECh[itc][1], TcECh[itc][0]);
	tcArray.setTc(0, 48);
	tcArray.print();
	// mergesort(tcArray.getTc());
	// print(tcArray);
	// tcArray.print();
	
	// ///////wikipedia
	// // // 
	// // // for p = 1, 2, 4, 8, ... # as long as p < n;
	// // // for k = p, p/2, p/4, p/8, ... # as long as k >= 1;
	// // // for j = mod(k,p) to (n-1-k) with a step size of 2k;
	// // // for i = 0 to min(k-1, n-j-k-1) with a step size of 1;
        // // // if floor((i+j) / (p*2)) == floor((i+j+k) / (p*2));
	// // // compare and sort elements (i+j) and (i+j+k);
	// // // return 0;

	// std::vector<TriggerCell>& tc = tcArray.getTc();
	// print(tc);
	// for(uint32_t ip=0;ip<6;ip++){
	//   uint32_t p = uint32_t(pow(2,ip));
	//   std::cout << "ip: " << ip << ", p: "<<p<<std::endl;
	//   for(uint32_t ik=0, k=p; k>=1 ; ik++,k = k/pow(2,ik)){
	//     std::cout << "  ik: " << ik << ", k: "<<k<<std::endl;
	//     for(uint32_t j=k%p; j<=(nofTcs-1-k) ; j+=2*k){
	//       std::cout << "\tj: " <<j<<std::endl;
	//       for(uint32_t i=0; i<=std::min((k-1),(nofTcs-j-k-1)) ; i++){
	// 	std::cout << "\t  i: " <<i<<std::endl;
	// 	if (floor((i+j)/(p*2)) == floor((i+j+k)/(p*2))){
	// 	  if(tc[i+j] > tc[i+j+k]) std::swap(tc[i+j],tc[i+j+k]);
	// 	}
	//       }//iloop
	//     }//jloop
	//   }//ik loop
	// }//ip loop

	// print(tc);

	//// The following is from https://github.com/dnoonan08/ECONT_Emulator/blob/c71cfa2acb7dea9b0b025cc8ac1a1eb845d0e71d/ASICBlocks/bestchoice.py#L15 and https://github.com/dnoonan08/ECONT_Emulator/blob/c71cfa2acb7dea9b0b025cc8ac1a1eb845d0e71d/ASICBlocks/bestchoice.py#L94 and 
	// //def sorter(ar, adr):
	// // N = int(ar.shape[0])
	// // t = int(m.ceil(m.log(N)/m.log(2)))
	// // p = int(2**(t-1))
	// // while p>=1:
	// //     q = 2**(t-1)
	// //     r = 0
	// //     d = p
	// //     while q>=p:
	// //         for i in range(N-d):
	// //             if i & p != r: continue
	// //             if ar[i] < ar[i+d]:
	// //                 ar[i+d], ar[i] = ar[i], ar[i+d]
	// //                 adr[i+d], adr[i] = adr[i], adr[i+d]
	// //         d = q - p
	// //         q = q//2
	// //         r = p
	// //     p = p//2
	// // return ar, adr
	
	// for(uint32_t itc=0;itc<nofTcs;itc++) std::cout<<tcArray[itc].getE()<<", ";
	// std::cout<<std::endl;
	
	std::vector<TriggerCell>& tc = tcArray.getTc();
	uint32_t N = uint32_t(tc.size());
	uint32_t t = uint32_t(ceil(log(N)/log(2)));
	uint32_t p = uint32_t(pow(2,(t-1)));
	std::cout <<"N : " << N << ", t: " << t << ", p : " << p << std::endl;
	while(p>=1){
	  uint32_t q = uint32_t(pow(2,(t-1)));
	  uint32_t r = 0;
	  uint32_t d = p;
	  while (q>=p){
	    for(uint32_t i=0; i<(N-d) ; i++){
	      //uint32_t ip =  (i & p) ; 
	      //std::cout <<"\t before i : " << i << ", p: " << p << ", r: "<< r << ", ip: "<<ip<< std::endl;
	      if ((i & p) != r) continue;
	      //std::cout <<"\t after i : " << i << ", p: " << p << ", r: "<< r << std::endl;
	      if (tc[i] < tc[i+d]) std::swap(tc[i], tc[i+d]);
	    }
	    d = q - p;
	    q = floor(q/2);
	    r = p;
	    //std::cout <<"    d : " << d << ", q: " << q << ", r: "<< r << std::endl;
	  }
	  p = floor(p/2);
	  //std::cout <<"  q : " << q << ", r: " << r << ", d: "<< d << ", p : " << p << std::endl;
	}
	print(tc);
}
