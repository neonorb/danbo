/*
 * danbo.h
 *
 *  Created on: Mar 30, 2017
 *      Author: chris13524
 */

#ifndef INCLUDE_DANBO_H_
#define INCLUDE_DANBO_H_

#include <feta.h>

namespace danbo {

struct Symbol {
	enum {
		CHAR, LITERAL, GROUP, VARIABLE, MANY, NOT
	} type;
	union {
		char c;
		struct {
			Symbol* symbols;
			feta::uinteger symbolsLength;
		} symbols;
		Symbol* wrappedSymbol;
	};
};

#define SINGLE_ARG(...) __VA_ARGS__

// references the symbol
#define REFS(name) \
		_danbo_##name

// references the symbol, but adds a comma
#define S(name) REFS(name) ,

#define DEFCHAR(name, c) \
	danbo::Symbol _danbo_##name = {danbo::Symbol::CHAR, c};

#define _SYMBOLS(type, name, asymbols) \
		danbo::Symbol _danbo_##name##_symbols[] = {asymbols}; \
		danbo::Symbol _danbo_##name = { danbo::Symbol::type, {symbols: {_danbo_##name##_symbols, ARRAY_SIZE(_danbo_##name##_symbols)}} };

// match each symbol exactly
#define DEFLITERAL(name, symbols) _SYMBOLS(LITERAL, name, SINGLE_ARG(symbols))

// match one of the symbols
#define DEFGROUP(name, symbols) _SYMBOLS(GROUP, name, SINGLE_ARG(symbols))

// match this symbol as many times as possible (including 0)
#define DEFVARIABLE(name, symbol) \
	danbo::Symbol _danbo_##name = { danbo::Symbol::VARIABLE, {wrappedSymbol: &symbol} };

// match this symbol at least once
#define DEFMANY(name, symbol) \
	danbo::Symbol _danbo_##name = { danbo::Symbol::MANY, {wrappedSymbol: &symbol} };

// this symbol should not be matched (used in groups)
#define DEFNOT(name, symbol) \
	danbo::Symbol _danbo_##name = { danbo::Symbol::NOT, {wrappedSymbol: &symbol} };

}

#endif /* INCLUDE_DANBO_H_ */
