// cannot do unsigned int I:0 = 1;
typedef struct StatusRegister
{
    unsigned int I : 3;
    unsigned int T : 1;
    unsigned int N : 1;
    unsigned int Z : 1;
    unsigned int V : 1;
    unsigned int C : 1;
	// overload =
};

typedef struct Bit
{
	int b : 1;
};

typedef struct Word
{
    int value:WORD_OCTAL_LENGTH*3;
	bool overflow = false;
	Bit binary[WORD_OCTAL_LENGTH * 3];
	// overload ==
};

typedef struct OctalBit
{
	unsigned int b : 3;
	// overload ==
};

#pragma region OCTALWORD_CLASS
class OctalWord : public Word
{
public:
	OctalWord();
	OctalWord(int val);
	~OctalWord();
	OctalBit octbit[6];

	operator=();
	OctalWord operator+(const OctalWord& b);
	OctalWord operator-(const OctalWord& b);
	operator*();
	OctalWord operator--();
	OctalWord operator--(int);
	OctalWord operator++();
	OctalWord operator++(int);
	operator<<();
	operator>>();
	operator+=();
	operator-=();
private:
	void updateBits();
};

#pragma region OCTALWORD_FUNCTION_DEFINITIONS

OctalWord::OctalWord()
{
	value = 0; for (int i = 0; i < WORD_OCTAL_LENGTH; i++) { octbit[i].b = 0; }
}

OctalWord::OctalWord(int val)
{
	value = val;
	for (int i = 0; i < WORD_OCTAL_LENGTH; i++)
	{
		int leftshift = val << ((WORD_OCTAL_LENGTH - i - 1) * 3);
		octbit[i].b = leftshift >> ((WORD_OCTAL_LENGTH - 1) * 3);
	}
}

OctalWord::~OctalWord()
{
}

OctalWord OctalWord::operator+(const OctalWord& b)
{
	int newValue = this->value + b.value;
	this->overflow = (newValue > MAX_OCTAL_VALUE);
	this->value = newValue;
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator-(const OctalWord& b)
{
	int newValue = this->value - b.value;
	this->overflow = (newValue > MAX_OCTAL_VALUE);
	this->value = newValue;
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator++()
{
	int newValue = this->value + 1;
	this->overflow = (newValue > MAX_OCTAL_VALUE);
	this->value = newValue;
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator++(int)
{
	int newValue = this->value + 1;
	this->overflow = (newValue > MAX_OCTAL_VALUE);
	this->value = newValue;
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator--()
{
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator--(int)
{
	this->updateBits();
	return *this;
}

void OctalWord::updateBits()
{
	for (int i = 0; i < WORD_OCTAL_LENGTH; i++)
	{
		int leftshift = this->value << ((WORD_OCTAL_LENGTH - i - 1) * 3);
		this->octbit[i].b = leftshift >> ((WORD_OCTAL_LENGTH - 1) * 3);
	}
}
#pragma endregion

#pragma endregion

#pragma region LIST_CLASS
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

#pragma region LIST_FUNCTION_DEFINITIONS
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
#pragma endregion

#pragma endregion

#pragma region TABLE_CLASS
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

#pragma region TABLE_FUNCTION_DEFINITIONS
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
	return value.getAt();
}

template<class KEY, class VALUE> void Table<KEY, VALUE>::add(KEY key, VALUE value)
{
	this->key.add(key);
	this->value.add(value);
	size++;
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

#pragma endregion

#pragma endregion