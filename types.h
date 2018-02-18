
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

typedef struct Register
{
    int regVal:16;
};

typedef struct Address
{
    int b:16;
}
