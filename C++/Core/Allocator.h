#pragma once

struct Blk
{
	void* ptr;
	size_t length;
};

template <class Primary, class Fallback>
class FallbackAllocator : private Primary, private Fallback
{
public:
	Blk allocate(size_t);
	void deallocate(Blk);
}

template <class P, class F>
Blk FallbackAllocator<P, F>::allocate(size_t n)
{
	Blk r = P::allocate(n);
	if (!r.ptr)
	{
		r = F::allocate(n);
	}

	return r;
}

template <class P, class F>
void FallbackAllocator<P, F>::deallocate(Blk b)
{
	if (P::owns(b))
	{
		P::deallocate(b);
	}
	else
	{
		F::deallocate(b);
	}
}

template <class P, class F>
bool FallbackAllocator::owns(Blk b)
{
	return P::owns(b) || F::owns(b);
}

/*
stack allocator
*/
template <size_t s> class StackAllocator
{
	char d_[s];
	char* p_;
public:
	StackAllocator() : p_(d_) {};

	Blk allocate(size_t n)
	{
		auto n1 = roundToAligned(n);
		// overflow
		if (n1 > (d_ + s) - p_)
		{
			return {nullptr, 0};
		}

		Blk result = {p_, n};
		p_ += n1;
		return result;
	}

	void deallocate(Blk b)
	{
		if (b.ptr + roundToAligned(n) == p_)
		{
			p_ = b.ptr;
		}
	}

	bool owns(Blk b)
	{
		return b.ptr >= d_ && b.ptr < d_ + s;
	}

	void deallocate()
	{
		p_ = d_;
	}
};

class Mallocator
{
public:
	Blk allocate(size_t bytes)
	{
		return Blk{malloc(bytes), bytes};
	}

	void deallocate(Blk blk)
	{
		free(blk.ptr);
		blk.ptr = nullptr;
		blk.length = 0;
	}

};

using Localloc = FallbackAllocator<StackAllocator<16284>, Mallocator>;

/*
Freelist
*/
template <class A, size_t s>
class FreeList
{
	A parent_;
	struct Node
	{
		Node* next;
	} root_;

public:
	Blk allocate(size_t n)
	{
		if (n == s && root_)
		{
			Blk b = {root_, n};
			root_ = root_.next;
			return b;
		}
		return parent_.allocate(n);
	}

	bool owns(Blk b)
	{
		return b.length == s || parent_.owns(b);
	}

	void deallocate(Blk)
	{
		if (b.length != s)
		{
			return parent_.deallocate(b);
		}
		auto p = (Node*)b.ptr;
		p.next = root_;
		root_ = p;
	}
}

/*
Affix allocator
*/
template <class A, class Prefix, class Suffix = void>
class AffixAllocator;

/*
Allocator with status
*/
template <class A, ulong flags>
class AllocatorWithStatus;

/*
Bitmapped block
*/
template <class A, size_t blockSize>
class BitmappedBlock;

/*
*/
template <class Creator>
class CascadingAllocator;

/*
Segregator
*/
template <size_t threshold, class SmallAllocator, class LargeAllocator>
struct Segregator
{
	Blk allocate(size_t s)
	{
		return s <= threshold ? _small.allocate(s) : _large.allocate(s);
	}
};