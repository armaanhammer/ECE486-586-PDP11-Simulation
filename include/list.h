#ifndef LIST_H_20180221
#define LIST_H_20180221
template <class T> class List
{
public:
	List<T>();
	List(T item);
	List(T item, List* p);
	~List();
	T find(T item);
	void add(T item);
	bool remove(T item);
	bool clear();
	T getFirst();
	T getlast();
	T getAt(int index);
	int indexof(T item);

private:
	List<T>* next;
	T item;
	int size;
};
#endif
