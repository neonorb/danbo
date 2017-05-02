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
#define PARSE(name, string) \
		danbo::parse(SP(name)::parse, string)

// references the symbol
#define ST(me) _danbotree_##me // AST tree node
#define SP(me) _danboparser_##me // grammar parser

struct SymbolTree {
	virtual ~SymbolTree() {
	}
};

// forward declare symbols
#define FDEF(me) \
	struct ST(me); \
	namespace SP(me) { \
		danbo::Result<ST(me)> parse(String text, UInteger position); \
	}

// match char
#define DEFCHAR(me, c) \
	struct ST(me) : danbo::SymbolTree { \
	}; \
	namespace SP(me) { \
		danbo::Result<ST(me)> parse(String text, UInteger position) { \
			if (text[position] == c) { \
				ST(me)* tree = new ST(me); \
				return {position + 1, tree}; \
			} else { \
				return {position, NULL}; \
			}; \
		} \
	};

// match each symbol exactly
// storage
#define _DEFLITERALT_EACH1(name, nested) ST(nested)* name = NULL;
#define _DEFLITERALT_EACH(data, x) _DEFLITERALT_EACH1 x
// destructor
#define _DEFLITERALT_D_EACH1(name, nested) \
	if (name != NULL) { \
		delete (danbo::SymbolTree*) name; \
	}
#define _DEFLITERALT_D_EACH(data, x) _DEFLITERALT_D_EACH1 x
// parser
#define _DEFLITERALP_EACH1(name, nested) { \
		danbo::Result<ST(nested)> status = SP(nested)::parse(text, position); \
		if (status.tree != NULL) { \
			position = status.position; \
			result.tree->name = status.tree; \
		} else { \
			delete (danbo::SymbolTree*) tree; \
			return {position, NULL}; \
		} \
	}
#define _DEFLITERALP_EACH(data, x) _DEFLITERALP_EACH1 x

#define N(name, nested) (name, nested)
#define DEFLITERAL(me, ...) \
	struct ST(me) : danbo::SymbolTree { \
		MAP(_DEFLITERALT_EACH, (), __VA_ARGS__) \
		~ST(me)() { \
			MAP(_DEFLITERALT_D_EACH, (), __VA_ARGS__) \
		} \
	}; \
	namespace SP(me) { \
		danbo::Result<ST(me)> parse(String text, UInteger position) { \
			ST(me)* tree = new ST(me); \
			danbo::Result<ST(me)> result = {0, tree}; \
			MAP(_DEFLITERALP_EACH, (), __VA_ARGS__) \
			result.position = position; \
			return result; \
		} \
	};

// match one of the symbols
#define _DEFSWITCH_T_ENUM(data, nested) C_##nested,
#define _DEFSWITCH_T_UNION(data, nested) ST(nested)* nested;
// destructor
#define _DEFSWITCH_T_D_EACH(data, nested) \
	case C_##nested: \
		delete (danbo::SymbolTree*) nested; \
		break;

#define _DEFSWITCH_P(me, nested) { \
		danbo::Result<ST(nested)> status = SP(nested)::parse(text, position); \
		if (status.tree != NULL) { \
			ST(me)* tree = new ST(me); \
			tree->choice = ST(me)::C_##nested; \
			tree->nested = status.tree; \
			return {status.position, tree}; \
		} \
	}
#define DEFSWITCH(me, ...) \
	struct ST(me) : danbo::SymbolTree { \
		enum { \
			MAP(_DEFSWITCH_T_ENUM, (), __VA_ARGS__) \
		} choice; \
		union { \
			MAP(_DEFSWITCH_T_UNION, (), __VA_ARGS__) \
		}; \
		~ST(me)() { \
			switch (choice) { \
				MAP(_DEFSWITCH_T_D_EACH, (), __VA_ARGS__) \
			} \
		} \
	}; \
	namespace SP(me) { \
		danbo::Result<ST(me)> parse(String text, UInteger position) { \
			MAP(_DEFSWITCH_P, me, __VA_ARGS__) \
			return {position, NULL}; \
		} \
	};

// match this symbol as many times as possible (including 0)
#define DEFVARIABLE(me, nested) \
	struct ST(me) : danbo::SymbolTree { \
		List<ST(nested)*> symbols; \
		~ST(me)() { \
			Iterator<ST(nested)*> iterator = symbols.iterator(); \
			while (iterator.hasNext()) { \
				delete (danbo::SymbolTree*) iterator.next(); \
			} \
		} \
	}; \
	namespace SP(me) { \
		danbo::Result<ST(me)> parse(String text, UInteger position) { \
			ST(me)* tree = new ST(me); \
			danbo::Result<ST(me)> result = {0, tree}; \
			while (true) { \
				danbo::Result<ST(nested)> status = SP(nested)::parse(text, position); \
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
#define DEFMANY(me, nested) \
	struct ST(me) : danbo::SymbolTree { \
		List<ST(nested)*> symbols; \
		~ST(me)() { \
			Iterator<ST(nested)*> iterator = symbols.iterator(); \
			while (iterator.hasNext()) { \
				delete (danbo::SymbolTree*) iterator.next(); \
			} \
		} \
	}; \
	namespace SP(me) { \
		danbo::Result<ST(me)> parse(String text, UInteger position) { \
			ST(me)* tree = new ST(me); \
			danbo::Result<ST(me)> result = {0, tree}; \
			while (true) { \
				danbo::Result<ST(nested)> status = SP(nested)::parse(text, position); \
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
#define DEFOPTIONAL(me, nested) \
	struct ST(me) : danbo::SymbolTree { \
		Boolean exists; \
		ST(nested)* tree; \
		~ST(me)() { \
			if (exists) { \
				delete (danbo::SymbolTree*) tree; \
			} \
		} \
	}; \
	namespace SP(me) { \
		danbo::Result<ST(me)> parse(String text, UInteger position) { \
			ST(me)* tree = new ST(me); \
			danbo::Result<ST(me)> result = {0, tree}; \
			danbo::Result<ST(nested)> status = SP(nested)::parse(text, position); \
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
