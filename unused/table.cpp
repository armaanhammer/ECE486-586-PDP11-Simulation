#include "table.h"

template<class KEY, class VALUE> Table<KEY, VALUE>::Table()
{
	size = 0;
}

template<class KEY, class VALUE> Table<KEY, VALUE>::~Table()
{
	clear();
}

template<class KEY, class VALUE> VALUE Table<KEY, VALUE>::find(KEY key)
{
	int indexKey = this->key.indexof(key);
	VALUE ret = value.getAt();
	return (ret != 0) ? ret : invalid;
}

template<class KEY, class VALUE> void Table<KEY, VALUE>::add(KEY key, VALUE value)
{
	if (this->key.find(key))
	{
		this->value = value;
	}
	else
	{
		this->key.add(key);
		this->value.add(value);
		size++;
	}
}

template<class KEY, class VALUE> bool Table<KEY, VALUE>::remove(KEY keyToRemove)
{
	int indexToRemove = key.indexof(key);
	VALUE valToRemove = value.getAt(indexToRemove);
	this->value.remove(valToRemove);
	this->key.remove(keyToRemove);
}

template<class KEY, class VALUE> bool Table<KEY, VALUE>::clear()
{
	key.clear();
	value.clear();
	size = 0;
}

template<class KEY, class VALUE> VALUE& Table<KEY, VALUE>::operator[] (KEY k)
{
	return this->find(k);
}
