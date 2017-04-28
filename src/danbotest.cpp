/*
 * test.cpp
 *
 *  Created on: Oct 21, 2016
 *      Author: chris13524
 */

#ifdef ALLOW_TEST

#include <danbo.h>
#include <feta.h>
#include <danbotest.h>
#include <fetatest.h>

using namespace feta;
using namespace danbo;

namespace danbotest {

#include "danbotest-grammar"

	static void danbo() {
#ifdef MEMORY_LOG
		// get allocated count
		uint64 origionalAllocatedCount = getAllocatedCount();
#endif
		
		ParseResult<ST(letter)> status = PARSE(letter, "a");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->choice == ST(letter)::C_la, "wrong choice");
		delete status.tree;

#ifdef MEMORY_LOG
		// get allocated count
		uint64 laterAllocatedCount = getAllocatedCount();

		// confirm no memory leaks
		if(origionalAllocatedCount != laterAllocatedCount) {
			//dumpAllocated();
		}
		assert(origionalAllocatedCount == laterAllocatedCount, "memory leak");
#endif
	}

	void test() {
		log(" - danbo");
		danbo();
	}
}

#endif
