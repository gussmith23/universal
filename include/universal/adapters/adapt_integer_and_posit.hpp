﻿#pragma once
// adapt_integer_and_posit.hpp: adapter functions to convert integer<size> type and posit<nbits,es> types
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <iostream>

// include this adapter before the src/tgt types that you want to connect

// if included, set the compilation flag that will enable the operator=(const TargetType&) in the SourceType.
#ifndef ADAPTER_POSIT_AND_INTEGER
#define ADAPTER_POSIT_AND_INTEGER 1
#else
#define ADAPTER_POSIT_AND_INTEGER 0
#endif // ADAPTER_POSIT_AND_INTEGER

namespace sw {
namespace unum {

// forward references
template<size_t nbits> class bitblock;
template<size_t nbits> class value;
template<size_t nbits, size_t es> class posit;
template<size_t nbits, size_t es> int scale(const posit<nbits, es>&);
template<size_t nbits, size_t es, size_t fbits> bitblock<fbits+1> significant(const posit<nbits, es>&);
template<size_t nbits> class integer;

/*
  Why is the convert function not part of the Integer or Posit types?
  It would tightly couple the types, which we want to avoid.
  If we want to productize these convertions we would need a new
  layer in the module design that sits above the Universal types. TODO
 */

// convert a Posit to an Integer
template<typename Integer, typename Posit>
inline void convert_p2i(const Posit& p, Integer& v) {
	// get the scale of the posit value
	int scale = sw::unum::scale(p);
	if (scale < 0) {
		v = 0;
		return;
	}
	if (scale == 0) {
		v = 1;
	}
	else {
		// gather all the fraction bits
		// sw::unum::bitblock<p.fhbits> significant = sw::unum::significant<p.nbits, p.es, p.fbits>(p);
                sw::unum::bitblock<Posit::fhbits> significant = sw::unum::significant<Posit::nbits, Posit::es, Posit::fbits>(p);
		// the radix point is at fbits, to make an integer out of this
		// we shift that radix point fbits to the right.
		// that is equivalent to a scale of 2^fbits
		v.clear();
		int msb = (v.nbits < p.fbits + 1) ? v.nbits : p.fbits + 1;
		for (int i = msb-1; i >= 0; --i) {
			v.set(i, significant[i]);
		}
		int shift = scale - p.fbits;  // if scale > fbits we need to shift left
		v <<= shift;
		if (p.isneg()) {
			v.flip();
			v += 1;
		}
	}
}

/////////////////////////////////////////////////////////////////////////
// convert an Integer to a Posit
template<typename Integer, typename Posit>
inline void convert_i2p(const Integer& w, Posit& p) {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t ibits = w.nbits;

	bool sign = w < 0;
	bool isZero = w == 0;
	bool isInf = false;
	bool isNan = false;
	long _scale = scale(w);
	Integer w2 = sign ? twos_complement(w) : w;
	int msb = findMsb(w2);
	bitblock<ibits> fraction_without_hidden_bit;
	int fbit = ibits - 1;
	for (int i = msb - 1; i >= 0; --i) {
		fraction_without_hidden_bit.set(fbit, w2.at(i));
		--fbit;
	}
	value<ibits> v;
	v.set(sign, _scale, fraction_without_hidden_bit, isZero, isInf, isNan);
	p = v;
}

} // namespace unum
} // namespace sw
