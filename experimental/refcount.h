/*
 * refcount.h
 *
 * \date  2013-9-24
 *      \author  salmon
 */

#ifndef REFCOUNT_H_
#define REFCOUNT_H_

struct RefCount
{
	int * count_;

	RefCount() :
			count_(new int(0))
	{
	}
	RefCount(const RefCount & other) :
			count_(other.count_)
	{
		++*count_;
	}
	~RefCount()
	{
		--*count_;
		if (*count_ <= 0)
		{
			delete count_;
		}
	}
	bool unique() const
	{
		return (*count_ == 0);
	}
	int use_count() const
	{
		return *count_;
	}

};

#endif /* REFCOUNT_H_ */
