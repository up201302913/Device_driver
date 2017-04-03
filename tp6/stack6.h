struct stack_el {  // element of a stack
	char *str;
	int n;
};

struct stack {    // a stack
	struct stack_el **sp;  // pointer to array to store stack elements
	int size;             // size of the array
	int top;			  // array index of the element at the top of the stack,
						  //  i.e. the first free position on the stack -- initially 0
};

struct stack *init(int size);	// initialize stack. Returns pointer to
								//  to initialized stack of desired size
int push(struct stack *sp, struct stack_el *el); // returns 0 if the stack is full, and 1 otherwise
struct stack_el *pop(struct stack *sp); // returns NULL if the stack is empty
void show(struct stack *sp);
void sfree(struct stack *sp);	// frees all data structures allocated
	
