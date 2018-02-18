#define MEMORYLENGTH 36000
#define NUMGENERALREGISTERS 6

// cannot do unsigned int I:0 = 1;
typedef struct StatusRegister
{
    unsigned int I:1;
    unsigned int T:1;
    unsigned int N:1;
    unsigned int Z:1;
    unsigned int V:1;
    unsigned int C:1;
};

typedef struct Word
{
    int value:18;
};

typedef void(*AddressMode)(int);
typedef void(*SingleOperand)(int);
typedef void(*DoubleOperand)(int, int);
typedef void(*Branch)(int);
typedef void(*ProcStatusWordInstruct)();









template <class T> class List
{
public:
	List<T>();
	List(T item);
	List(T item, List* p);
	~List();
	T find(T item);
	bool add(T item);
	bool remove(T item);
	T pop();
	bool clear();
	T getFirst();
	T getlast();

private:
	List<T>* next;
	T item;
	int size;
};

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
	next = p;
	size = 1 + p->size;
}

template<class T> List<T>::~List()
{
}

template<class T> T List<T>::find(T item)
{
}

template<class T> bool List<T>::add(T item)
{
}

template<class T> bool List<T>::remove(T item)
{
}

template<class T> T List<T>::pop()
{
}

template<class T> bool List<T>::clear()
{
}

template<class T> T List<T>::getFirst()
{
}

template<class T> T List<T>::getlast()
{
}










template <class T> class Table
{
public:
	Table();
	~Table();

private:

};

template<class T>Table::Table()
{
}

template<class T>Table::~Table()
{
}