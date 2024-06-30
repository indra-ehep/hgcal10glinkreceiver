// From: https://gist.github.com/stbuehler (https://gist.github.com/stbuehler/883635)
// Author : Stefan Bühler

/* Batcher odd-even mergesort: http://en.wikipedia.org/wiki/Batcher_odd-even_mergesort */
/* O(n * (log n) * (log n)): inplace, and could run in O( (log n) * (log n) ) on n cores in parallel */

/* Most implemenations only handle power-of-two input sizes; this implementation
   appends "virtual" infinity items at the end of input (takes no space) */

#include <vector>
#include <cassert>
#include <cstddef>

/* Array abstraction so we don't have to copy arrays for odd/even/halves */

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

	bool empty() { return 0 == m_len; }

	slice<T> sub(size_t start, size_t step) {
		assert(step);
		if (start >= m_len) return slice<T>();
		size_t len = 1 + (m_len-start-1) / step;
		return slice<T>(m_data, m_start + start * m_step, m_step * step, len, false);
	}
	slice<T> sub(size_t start, size_t step, size_t len) {
		assert(step);
		if (start >= m_len || 0 == len) return slice<T>();
		len = std::min(len, 1 + (m_len-start-1) / step);
		return slice<T>(m_data, m_start + start * m_step, m_step * step, len, false);
	}

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

template<typename T>
void mergesort(slice<T> data) {
	if (data.size() <= 2) {
		sortTwo(data);
	} else {
		std::pair< slice<T>, slice<T> > halves = data.halves();
		/* Run this in parallel if you want: */
			mergesort(halves.first); /* first "half" always has a power-of-two size */
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
	std::vector<int> data(50);
	for (size_t i = 0; i < data.size(); i++) data[i] = i;
	std::random_shuffle(data.begin(), data.end());
	print(data);
	mergesort(data);
	print(data);
	for (int i = int(data.size())-1; i >= 0 ; i--)
	  std::cout << " " << data[i];
	std::cout << std::endl;
	return 0;
}
