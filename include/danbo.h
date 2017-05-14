/*
 * danbo.h
 *
 *  Created on: Mar 30, 2017
 *      Author: chris13524
 */

#ifndef INCLUDE_DANBO_H_
#define INCLUDE_DANBO_H_

#include <feta.h>
#include <thirdparty/map.h>

namespace danbo {

template<typename T>
struct Result {
	feta::UInteger position;
	T* tree;
};

enum ParseStatus {
	OK, FAIL
};

template<typename T>
struct ParseResult {
	ParseStatus status;
	union {
		T* tree;
		feta::UInteger failedAt;
	};
};

template<typename T>
ParseResult<T> parse(Result<T> (*parseFunction)(feta::String, feta::UInteger),
		feta::String code) {
	Result<T> result = parseFunction(code, 0);
	ParseResult<T> status;
	if (result.tree == NULL) {
		// parsing failed
		status.status = ParseStatus::FAIL;
		status.failedAt = result.position;
	} else if (result.position < feta::stringLength(code)) {
		// we still have unparsed bytes (even though parsing passed), fail the whole parse
		delete result.tree;
		status.status = ParseStatus::FAIL;
		status.failedAt = result.position;
	} else {
		// parsing passed
		status.status = ParseStatus::OK;
		status.tree = result.tree;
	}
	return status;
}

// parses the given string and returns a tree
#define D_PARSE(name, string) \
		danbo::parse(D_SP(name)::parse, string)

// references the symbol
#define D_ST(me) _danbotree_##me // AST tree node
#define D_SP(me) _danboparser_##me // grammar parser

struct SymbolTree {
	virtual ~SymbolTree() {
	}
};

// forward declare symbols
#define D_FDEF(me) \
	struct D_ST(me); \
	namespace D_SP(me) { \
		danbo::Result<D_ST(me)> parse(String text, UInteger position); \
	}

// match char
#define D_DEFCHAR(me, c) \
	struct D_ST(me) : danbo::SymbolTree { \
	}; \
	namespace D_SP(me) { \
		danbo::Result<D_ST(me)> parse(String text, UInteger position) { \
			if (text[position] == c) { \
				D_ST(me)* tree = new D_ST(me); \
				return {position + 1, tree}; \
			} else { \
				return {position, NULL}; \
			}; \
		} \
	};

// match each symbol exactly
// storage
#define _DEFLITERALT_EACH1(name, nested) D_ST(nested)* name = NULL;
#define _DEFLITERALT_EACH(data, x) _DEFLITERALT_EACH1 x
// destructor
#define _DEFLITERALT_D_EACH1(name, nested) \
	if (name != NULL) { \
		delete (danbo::SymbolTree*) name; \
	}
#define _DEFLITERALT_D_EACH(data, x) _DEFLITERALT_D_EACH1 x
// parser
#define _DEFLITERALP_EACH1(name, nested) { \
		danbo::Result<D_ST(nested)> status = D_SP(nested)::parse(text, position); \
		if (status.tree != NULL) { \
			position = status.position; \
			result.tree->name = status.tree; \
		} else { \
			delete (danbo::SymbolTree*) tree; \
			return {position, NULL}; \
		} \
	}
#define _DEFLITERALP_EACH(data, x) _DEFLITERALP_EACH1 x

// name the part of the literal
#define D_N(name, nested) (name, nested)
// don't give it a specific name (one will be generated)
#define D_U(nested) (MACRO_CONCAT(z_unnamed, __COUNTER__), nested)
#define D_DEFLITERAL(me, ...) \
	struct D_ST(me) : danbo::SymbolTree { \
		MAP(_DEFLITERALT_EACH, (), __VA_ARGS__) \
		~D_ST(me)() { \
			MAP(_DEFLITERALT_D_EACH, (), __VA_ARGS__) \
		} \
	}; \
	namespace D_SP(me) { \
		danbo::Result<D_ST(me)> parse(String text, UInteger position) { \
			D_ST(me)* tree = new D_ST(me); \
			danbo::Result<D_ST(me)> result = {0, tree}; \
			MAP(_DEFLITERALP_EACH, (), __VA_ARGS__) \
			result.position = position; \
			return result; \
		} \
	};

// match one of the symbols
#define _DEFSWITCH_T_ENUM(data, nested) C_##nested,
#define _DEFSWITCH_T_UNION(data, nested) D_ST(nested)* nested;
// destructor
#define _DEFSWITCH_T_D_EACH(data, nested) \
	case C_##nested: \
		delete (danbo::SymbolTree*) nested; \
		break;

#define _DEFSWITCH_P(me, nested) { \
		danbo::Result<D_ST(nested)> status = D_SP(nested)::parse(text, position); \
		if (status.tree != NULL) { \
			D_ST(me)* tree = new D_ST(me); \
			tree->choice = D_ST(me)::C_##nested; \
			tree->nested = status.tree; \
			return {status.position, tree}; \
		} \
	}
#define D_DEFSWITCH(me, ...) \
	struct D_ST(me) : danbo::SymbolTree { \
		enum { \
			MAP(_DEFSWITCH_T_ENUM, (), __VA_ARGS__) \
		} choice; \
		union { \
			MAP(_DEFSWITCH_T_UNION, (), __VA_ARGS__) \
		}; \
		~D_ST(me)() { \
			switch (choice) { \
				MAP(_DEFSWITCH_T_D_EACH, (), __VA_ARGS__) \
			} \
		} \
	}; \
	namespace D_SP(me) { \
		danbo::Result<D_ST(me)> parse(String text, UInteger position) { \
			MAP(_DEFSWITCH_P, me, __VA_ARGS__) \
			return {position, NULL}; \
		} \
	};

// match this symbol as many times as possible (including 0)
#define D_DEFVARIABLE(me, nested) \
	struct D_ST(me) : danbo::SymbolTree { \
		List<D_ST(nested)*> symbols; \
		~D_ST(me)() { \
			Iterator<D_ST(nested)*> iterator = symbols.iterator(); \
			while (iterator.hasNext()) { \
				delete (danbo::SymbolTree*) iterator.next(); \
			} \
		} \
	}; \
	namespace D_SP(me) { \
		danbo::Result<D_ST(me)> parse(String text, UInteger position) { \
			D_ST(me)* tree = new D_ST(me); \
			danbo::Result<D_ST(me)> result = {0, tree}; \
			while (true) { \
				danbo::Result<D_ST(nested)> status = D_SP(nested)::parse(text, position); \
				if (status.tree == NULL) { \
					break; \
				} \
				position = status.position; \
				tree->symbols.add(status.tree); \
			} \
			result.position = position; \
			return result; \
		} \
	};

// match this symbol at least once
#define D_DEFMANY(me, nested) \
	struct D_ST(me) : danbo::SymbolTree { \
		List<D_ST(nested)*> symbols; \
		~D_ST(me)() { \
			Iterator<D_ST(nested)*> iterator = symbols.iterator(); \
			while (iterator.hasNext()) { \
				delete (danbo::SymbolTree*) iterator.next(); \
			} \
		} \
	}; \
	namespace D_SP(me) { \
		danbo::Result<D_ST(me)> parse(String text, UInteger position) { \
			D_ST(me)* tree = new D_ST(me); \
			danbo::Result<D_ST(me)> result = {0, tree}; \
			while (true) { \
				danbo::Result<D_ST(nested)> status = D_SP(nested)::parse(text, position); \
				if (status.tree == NULL) { \
					break; \
				} \
				position = status.position; \
				tree->symbols.add(status.tree); \
			} \
			if (result.tree->symbols.size() == 0) { \
				delete (danbo::SymbolTree*) tree; \
				return {position, NULL}; \
			} \
			result.position = position; \
			return result; \
		} \
	};

// match this symbol, or don't
#define D_DEFOPTIONAL(me, nested) \
	struct D_ST(me) : danbo::SymbolTree { \
		Boolean exists; \
		D_ST(nested)* tree; \
		~D_ST(me)() { \
			if (exists) { \
				delete (danbo::SymbolTree*) tree; \
			} \
		} \
	}; \
	namespace D_SP(me) { \
		danbo::Result<D_ST(me)> parse(String text, UInteger position) { \
			D_ST(me)* tree = new D_ST(me); \
			danbo::Result<D_ST(me)> result = {0, tree}; \
			danbo::Result<D_ST(nested)> status = D_SP(nested)::parse(text, position); \
			if (status.tree != NULL) { \
				tree->exists = true; \
				tree->tree = status.tree; \
				result.position = status.position; \
			} else { \
				tree->exists = false; \
				tree->tree = NULL; \
				result.position = position; \
			} \
			return result; \
		} \
	};

}

#endif /* INCLUDE_DANBO_H_ */
