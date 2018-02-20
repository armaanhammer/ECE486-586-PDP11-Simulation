#pragma region DEFINE_CONSTANTS
#define MEMORYLENGTH 36000
#define NUMGENERALREGISTERS 6

// Processor Status Word Instructions Opcodes
#define NUM_PSW_INSTRUCTIONS 0300
#define CLC_OPCODE 000241
#define CLV_OPCODE 000242
#define CLZ_OPCODE 000244
#define CLN_OPCODE 000250
#define SEC_OPCODE 000261
#define SEV_OPCODE 000262
#define SEZ_OPCODE 000264
#define SEN_OPCODE 000270
#define CCC_OPCODE 000257
#define SCC_OPCODE 000277

// Single Operand Instructions Opcodes
#define NUM_SINGLE_OP_INSTRUCTIONS 0105
#define SWAB_OPCODE 0003
#define JSR_OPCODE 004
#define EMT_OPCODE 0104
#define CLR_OPCODE 0050
#define COM_OPCODE 0051
#define INC_OPCODE 0052
#define DEC_OPCODE 0053
#define NEG_OPCODE 0054
#define ADC_OPCODE 0055
#define SBC_OPCODE 0056
#define TST_OPCODE 0057
#define ROR_OPCODE 0060
#define ROL_OPCODE 0061
#define ASR_OPCODE 0062
#define ASL_OPCODE 0063
#define SXT_OPCODE 0067

// Double Operand Instructions Opcodes
#define NUM_DOUBLE_OP_INSTRUCTIONS 8
#define MOV_OPCODE 01
#define CMP_OPCODE 02
#define BIT_OPCODE 03
#define BIC_OPCODE 04
#define BIS_OPCODE 05
#define ADD_OPCODE 06
#define SUB_OPCODE 07

// Branch Instructions Opcodes
#define NUM_BRANCH_INSTRUCTIONS 0210
#define BR_OPCODE 0001
#define BNE_OPCODE 0002
#define BEQ_OPCODE 0003
#define BPL_OPCODE 0200
#define BMI_OPCODE 0201
#define BVC_OPCODE 0204
#define BHIS_OPCODE 0205
#define BCC_OPCODE 0206
#define BLO_OPCODE 0207
#define BCS_OPCODE 0207
#define BGE_OPCODE 0004
#define BLT_OPCODE 0005
#define BGT_OPCODE 0006
#define BLE_OPCODE 0007
#define BHI_OPCODE 0202
#define BLOS_OPCODE 0203

// Addressing Modes
#define NUM_ADDRESSING_MODES 18
#define REGISTER_CODE			00
#define REGISTER_DEFERRED_CODE	01
#define AUTOINC_CODE			02
#define AUTOINC_DEFERRED_CODE	03
#define AUTODEC_CODE			04
#define AUTODEC_DEFERRED_CODE	05
#define INDEX_CODE				06
#define INDEX_DEFFERRED_CODE	07
#define PC_IMMEDIATE_CODE			027
#define PC_ABSOLUTE_CODE			037
#define PC_RELATIVE_CODE			067
#define PC_RELATIVE_DEFERRED_CODE	077
#define SP_DEFERRED_CODE			016
#define SP_AUTOINC_CODE				026
#define SP_AUTOINC_DEFERRED_CODE	036
#define SP_AUTODEC_CODE				046
#define SP_INDEX_CODE				066
#define Sp_INDEX_DEFFERRED_CODE		076

#pragma endregion

// cannot do unsigned int I:0 = 1;
typedef struct StatusRegister
{
    unsigned int I:3;
    unsigned int T:1;
    unsigned int N:1;
    unsigned int Z:1;
    unsigned int V:1;
    unsigned int C:1;
	// overload =
};

#define WORD_OCTAL_LENGTH 6
#define MAX_OCTAL_VALUE 0177777
typedef struct Word
{
    int value:WORD_OCTAL_LENGTH*3;
};

typedef struct OctalBit
{
	int b : 3;
};

class OctalWord : public Word
{
public:
	OctalWord() { value = 0; for (int i = 0; i < WORD_OCTAL_LENGTH; i++) { octbit[i].b = 0; } };
	OctalWord(int val)
	{
		value = val;
		for (int i = 0; i < WORD_OCTAL_LENGTH; i++)
		{
			int leftshift = val << ((WORD_OCTAL_LENGTH - i - 1) * 3);
			octbit[i].b = leftshift >> ((WORD_OCTAL_LENGTH - 1) * 3);
		}
	};
	~OctalWord() {};
	OctalBit octbit[6];
	// overload =
	// overload +
	// overload -
	// overload <<
	// overload >>

};


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