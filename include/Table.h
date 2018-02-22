#pragma once
#include "List.h"

#ifndef TABLE_H_20180221
#define TABLE_H_20180221
template <class KEY, class VALUE> class Table
{
public:
	Table();
	~Table();
	VALUE find(KEY key);
	void add(KEY key, VALUE value);
	bool remove(KEY key);
	bool clear();

private:
	List<KEY> key;
	List<VALUE> value;
	int size;
};
#endif