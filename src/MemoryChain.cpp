#include "stdafx.h"
#include "MemoryChain.h"


class Ty
{
	int i;
};

static MemoryChain<Ty, 2> s_chain;


void Test()
{
	Ty* t1 = s_chain.Allocate();
	Ty* t2 = s_chain.Allocate();
	Ty* t3 = s_chain.Allocate();
	Ty* t4 = s_chain.Allocate();
	s_chain.Free(t4);
	s_chain.Free(t1);
	t1 = s_chain.Allocate();
	s_chain.Free(t2);
	s_chain.Free(t1);
	t2 = s_chain.Allocate();
	s_chain.DeleteFree();
}

