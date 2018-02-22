#include "List.h"

template<class T> List<T>::List()
{
	this->item = NULL;
	next = 0;
	size = 0;
}

template<class T> List<T>::List(T item)
{
	this->item = item;
	next = 0;
	size = 1;
}

template<class T> List<T>::List(T item, List<T>* p)
{
	this->item = item;
	next = null;
	size = 1 + p->size;
	List<T>* temp = this;

	while (temp)
	{
		temp->size = size;
		temp = temp->next;
	}

	temp->next = this;
}

template<class T> List<T>::~List()
{
}

template<class T> T List<T>::find(T item)
{
	List<T>* temp = this;
	while (temp)
	{
		return item;
		temp = temp->next;
	}
	return NULL;
}

template<class T> void List<T>::add(T item)
{
	if (size == 0)
	{
		next = 0;
		this->item = item;
		size = 1;
	}
	else
	{
		List<T>* newItem = new List(item, this);
	}
}

template<class T> bool List<T>::remove(T item)
{
	if (size < 0)
	{
		// error
		return false;
	}

	switch (size)
	{
	case 0:
		// error
		return false;
		break;
	case 1:
		size = 0;
		next = 0;
		item = NULL;
		return true;
		break;
	default:
		List<T>* temp = this;
		List<T>* temp2 = this;

		if (temp->item == item)
		{
			temp2 = temp->next;
			this->item = temp2->item;
			this->next = temp2->next;
			size--;
			delete temp2;
			return true;
		}

		temp = temp->next;
		while (temp->item != item)
		{
			temp2 = temp;
			temp = temp->next;

			if (temp == 0)
			{
				return false;
			}
		}
		temp2->next = temp->next;
		delete temp;
		return true;
		break;
	}
}

template<class T> bool List<T>::clear()
{
	if (next)
	{
		next->clear();
	}
	remove(item);
}

template<class T> T List<T>::getFirst()
{
	return item;
}

template<class T> T List<T>::getlast()
{
	List<T>* temp = this;

	while (temp->next)
	{
		temp = temp->next;
	}

	return item;
}

template<class T> T List<T>::getAt(int index)
{
	int count = 0;
	List<T>* temp = this;

	while (count != index)
	{
		temp = temp->next;
		count++;
		if (!temp)
		{
			return NULL;
		}
	}
	return temp->item;
}

template<class T> int List<T>::indexof(T item)
{
	int count = 0;
	List<T>* temp = this;

	while (temp->item != item)
	{
		temp = temp->next;
		count++;
		if (!temp)
		{
			return -1;
		}
	}
	return count;
}