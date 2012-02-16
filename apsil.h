#include<string.h>
#define TRUE 1
#define FALSE 0

int m=-1,m2=-1,m3=-1; //m-variable type, m2-argtype, m3-returntype of function
int chkret=-1;
struct tree *funcid=NULL;
int memcount=0,regcount=0,datacount=512;
FILE *fp;

struct ArgStruct{
        char* name;
        int type;//0-integer,1-boolean, 3-string
        int passtype;//0-by value,1-by reference
        struct ArgStruct* next;
};
struct DataStruct{
	char *string;
	struct DataStruct *next;
};
struct DataStruct *Droot=NULL;
struct Lsymbol
{
	char *name;
	int type;	//integer-0, boolean-1, 3-string
	int *binding;
	int bind;
	struct Lsymbol *next;
};
struct Gsymbol
{
	char *name;
	int type;	//integer-0, boolean-1, 3-string
	int size;
	int *binding;
	int bind;
	struct ArgStruct *arglist;
	struct Gsymbol *next;
};
struct Gsymbol *root=NULL;
struct Lsymbol *Lroot=NULL;

struct tree
{
	int type;		//0-integer , 1-boolean , 2-void, 3-string
	char nodetype;		/*	+,-,*,/,%,=,<,>,!
					?-if statement
					c-number,	i-identifier,	r-read,		p-print,
					n-nonterminal,	e-double equals,	l-lessthan or equals
					g-greaterthan or equals		w-while		s-string
					b-boolean constants,	f-function,	u-return
					a-AND		o-OR		x-NOT
							*/
	char *name;
	int value;
	struct Gsymbol *Gentry;
	struct Lsymbol *Lentry;
	struct tree *argument;
	struct tree *ptr1,*ptr2,*ptr3;
};

int labelcount=0;
struct label
{
	int i;
	struct label *next; 
};
struct label *top=NULL;
void push()
{
	struct label *temp;
	temp=malloc(sizeof(struct label));
	temp->i=labelcount;
	temp->next=top;
	top=temp; 
	labelcount++;
}
int pop()
{
	int i;
	struct label *temp;
	temp=top;
	top=top->next;
	i=temp->i;
	free(temp);
	return i;
}
struct tree * makedata(struct tree *a)
{
	struct DataStruct *temp;
	temp=malloc(sizeof(struct DataStruct));
	a->value=datacount;
	datacount++;
	temp->string=a->name;
	temp->next=Droot;
	Droot=temp;
	return(a);
}
void intodataarea(struct DataStruct *t, FILE *fp)
{
	if(t->next!=NULL)
		intodataarea(t->next,fp);
	fprintf(fp,"%s\n",t->string);
}
void filearea()
{	
	FILE *fp2,*fp3;
	char *bufftemp=(char *) malloc (16);
	int linecount=0;
	size_t nbytes;
	fp3=fopen("./ap.sim","a");
	fp2=fopen("./ap.sim","r");
	while(!feof(fp2))
	{
		getline(&bufftemp, &nbytes,fp2);
		linecount++;
	}
	while(linecount<513)
	{
		fprintf(fp3,"\n");
		linecount++;
	}
	intodataarea(Droot,fp3);		
}
/*void displaysymbols()
{
	struct symbol *temp=root;
	printf("\n");
	while(temp!=NULL)
	{
		printf("%s ",temp->name);
		temp=temp->next;
	}
	printf("\n");
}*/
int evaluate(struct tree * root)
{
	int n;
	if(root==NULL)
		return;
	switch(root->nodetype)
	{
		case '<':
			if(evaluate(root->ptr1) < evaluate(root->ptr2))
				return TRUE; 
			else
				return FALSE;
			break;
		case '>':
			if(evaluate(root->ptr1) > evaluate(root->ptr2))
				return TRUE; 
			else
				return FALSE;
			break;
		case 'e':
			if(evaluate(root->ptr1) == evaluate(root->ptr2))
				return TRUE; 
			else
				return FALSE;
			break;
		case 'l':
			if(evaluate(root->ptr1) <= evaluate(root->ptr2))
				return TRUE; 
			else
				return FALSE;
			break;
		case 'g':
			if(evaluate(root->ptr1) >= evaluate(root->ptr2))
				return TRUE; 
			else
				return FALSE;
			break;
		case '!':
			if(evaluate(root->ptr1) != evaluate(root->ptr2))
				return TRUE; 
			else
				return FALSE;
			break;
		case 'b':    //for boolean constants
			return (root->value);
			break;
		case 'a':	//AND operator
			return((evaluate(root->ptr1)) && (evaluate(root->ptr2)));
			break;
		case 'o':	//OR operator
			return((evaluate(root->ptr1)) || (evaluate(root->ptr2)));
			break;
		case 'x':	//NOT operator
			return((evaluate(root->ptr1))-(evaluate(root->ptr2)));
			break;
		
		case 'n':	//statement list
			evaluate(root->ptr1);			
			evaluate(root->ptr2);
			break;
		case '+':
			return( evaluate(root->ptr1) + evaluate(root->ptr2) );
			break;
		case '-':
			return( evaluate(root->ptr1) - evaluate(root->ptr2) );
			break;
		case '*':
			return( evaluate(root->ptr1) * evaluate(root->ptr2) );
			break;
		case '/':
			n=evaluate(root->ptr2);
			if(n!=0)
				return( evaluate(root->ptr1) / n );
			else
			{
				printf("\n Division by zero !!\n");
				exit(0);
			}
			break;
		case '%':
			n=evaluate(root->ptr2);
			if(n!=0)
				return( evaluate(root->ptr1) % n );
			else
			{
				printf("\n Division by zero !!\n");
				exit(0);
			}	
			break;
		case '=':
			if(root->ptr1->Gentry!=NULL)
				*( (root->ptr1->Gentry->binding) + evaluate(root->ptr1->ptr1) )=evaluate(root->ptr2);
			else
				*( (root->ptr1->Lentry->binding) + evaluate(root->ptr1->ptr1) )=evaluate(root->ptr2);
			break;
		case 'r':	//READ
			if(root->ptr1->type==1)
			{
				char a[10];
				scanf("%s",a);
				if(strcmp(a,"true")==0)
					n=1;
				else if(strcmp(a,"false")==0)
					n=0;
				else
				{
					printf("\n boolean variables must be true or false !!\n ");
					exit(0);
				}			
			}
			else
				scanf("%d" , &n );
			if(root->ptr1->Gentry!=NULL)
				*((root->ptr1->Gentry->binding)+evaluate(root->ptr1->ptr1))=n;
			else
				*((root->ptr1->Lentry->binding)+evaluate(root->ptr1->ptr1))=n;
			break;
		case 'p':	//PRINT
			if(root->ptr1->type==1)
			{
				if(evaluate(root->ptr1)==0)
					printf("false\n");
				else
					printf("true\n");
			}
			else
				printf("%d\n", evaluate(root->ptr1));
			break;
		case 'c':	//constants
			return root->value;
			break;
		case 'i':			//variables and array variables
			if(root->Gentry!=NULL)
				return *((root->Gentry->binding)+evaluate(root->ptr1));
			else
				return *((root->Lentry->binding)+evaluate(root->ptr1));
			break;
		case '?':	//IF statement , IF-ELSE statements
			if(evaluate(root->ptr1)==TRUE)
				evaluate(root->ptr2);
			else if(root->ptr3!=NULL)
				evaluate(root->ptr3);
			break;
		case 'w':	//WHILE loop
			while(evaluate(root->ptr1)==TRUE)
				evaluate(root->ptr2);
			break;
		default:
			return;
	}
}
void pushargs(struct tree *);
void popargs(struct tree *,struct ArgStruct *,int);
void endfn();
void codegen(struct tree * root)
{
	int n;
	if(root==NULL)
		return;										
	switch(root->nodetype)
	{
		case '<':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"LT R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case '>':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"GT R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case 'e':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"EQ R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case 'l':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"LE R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case 'g':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"GE R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case '!':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"NE R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case 'b':    //for boolean constants
			fprintf(fp,"MOV R%d,%d\n",regcount,root->value);
			regcount++;
			break;
		case 'a':	//AND operator
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"MUL R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case 'o':	//OR operator
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"ADD R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case 'x':	//NOT operator
			codegen(root->ptr1);
			fprintf(fp,"MOV R%d,1\n",regcount);
			regcount++;
			fprintf(fp,"SUB R%d,R%d\n",regcount-1,regcount-2);
			fprintf(fp,"MOV R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;		
		case 'n':	//statement list
			codegen(root->ptr1);
			codegen(root->ptr2);			
			break;
		case '+':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"ADD R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case '-':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"SUB R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case '*':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"MUL R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case '/':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"DIV R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case '%':
			codegen(root->ptr1);			
			codegen(root->ptr2);
			fprintf(fp,"MOD R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
			break;
		case '=':			
			if(root->ptr1->Gentry!=NULL)
				fprintf(fp,"MOV R%d,%d\n",regcount,root->ptr1->Gentry->bind);
			else
			{
				fprintf(fp,"MOV R%d,%d\n",regcount,root->ptr1->Lentry->bind);
				fprintf(fp,"MOV R%d,BP\n",regcount+1);
				fprintf(fp,"ADD R%d,R%d\n",regcount,regcount+1);
			}			
			regcount++;
			if(root->ptr1->ptr1!=NULL)
			{
				codegen(root->ptr1->ptr1);
				fprintf(fp,"ADD R%d,R%d\n",regcount-2,regcount-1);
				regcount--;
			}
			codegen(root->ptr2);
			if((root->ptr1->Gentry==NULL)?root->ptr1->Lentry->type:root->ptr1->Gentry->type==3)
				fprintf(fp,"STRCPY R%d,R%d\n",regcount-2,regcount-1);
			else
				fprintf(fp,"MOV [R%d],R%d\n",regcount-2,regcount-1);
			regcount=regcount-2;
			break;
		case 'r':	//READ
			if(root->ptr1->Gentry!=NULL)
				fprintf(fp,"MOV R%d,%d\n",regcount,root->ptr1->Gentry->bind);
			else
			{
				fprintf(fp,"MOV R%d,%d\n",regcount,root->ptr1->Lentry->bind);
				fprintf(fp,"MOV R%d,BP\n",regcount+1);
				fprintf(fp,"ADD R%d,R%d\n",regcount,regcount+1);
			}
			regcount++;
			if(root->ptr1->ptr1!=NULL)
			{
				codegen(root->ptr1->ptr1);
				fprintf(fp,"ADD R%d,R%d\n",regcount-2,regcount-1);
				regcount--;
			}
			if((root->ptr1->Gentry==NULL)?root->ptr1->Lentry->type:root->ptr1->Gentry->type==3)
			{
				fprintf(fp,"SIN R%d\n",regcount-1);
				regcount--;
			}
			else
			{
				fprintf(fp,"IN R%d\n",regcount);
				regcount++;
				fprintf(fp,"MOV [R%d],R%d\n",regcount-2,regcount-1);
				regcount=regcount-2;
			}
			break;
		case 'p':	//PRINT
			codegen(root->ptr1);
			if(root->ptr1->type==3)
				fprintf(fp,"SOUT R%d\n",regcount-1);
			else
				fprintf(fp,"OUT R%d\n",regcount-1);
			regcount--;
			break;
		case 'c':	//constants
			fprintf(fp,"MOV R%d,%d\n",regcount,root->value);
			regcount++;
			break;
		case 's':	//string constants
			fprintf(fp,"MOV R%d,%d\n",regcount,root->value);
			regcount++;
			break;
		case 'i':			//variables and array variables
			if(root->Gentry!=NULL)
				fprintf(fp,"MOV R%d,%d\n",regcount,root->Gentry->bind);
			else
			{
				fprintf(fp,"MOV R%d,%d\n",regcount,root->Lentry->bind);
				fprintf(fp,"MOV R%d,BP\n",regcount+1);
				fprintf(fp,"ADD R%d,R%d\n",regcount,regcount+1);
			}
			regcount++;
			if(root->ptr1!=NULL)
			{
				codegen(root->ptr1);
				fprintf(fp,"ADD R%d,R%d\n",regcount-2,regcount-1);
				regcount--;
			}
			if(root->type!=3)			
				fprintf(fp,"MOV R%d,[R%d]\n",regcount-1,regcount-1);
			break;
		case '?':	//IF statement , IF-ELSE statements
			push();
			codegen(root->ptr1);
			fprintf(fp,"JZ R%d,la%d\n",regcount-1,top->i);
			regcount--;
			codegen(root->ptr2);
			fprintf(fp,"JMP lb%d\n",top->i);
			fprintf(fp,"la%d:\n",top->i);
			codegen(root->ptr3);
			fprintf(fp,"lb%d:\n",pop());
			break;
		case 'w':	//WHILE loop
			push();
			fprintf(fp,"la%d:\n",top->i);
			codegen(root->ptr1);
			fprintf(fp,"JZ R%d,lb%d\n",regcount-1,top->i);
			regcount--;
			codegen(root->ptr2);
			fprintf(fp,"JMP la%d\n",top->i);
			fprintf(fp,"lb%d:\n",pop());
			break;
		case 'f':
			n=regcount;
			while(regcount>0)
			{
				fprintf(fp,"PUSH R%d\n",regcount-1);
				regcount--;
			}
			pushargs(root->ptr1);
			fprintf(fp,"PUSH R0\nCALL fn%d\n",root->Gentry->bind);
			fprintf(fp,"POP R%d\n",n);
			if(root->Gentry->type==3)
			{
				fprintf(fp,"MOV R%d,1\n",n);
				fprintf(fp,"MOV R%d,SP\n",n+1);
				fprintf(fp,"ADD R%d,R%d\n",n,n+1);
			}			
			popargs(root->ptr1,root->Gentry->arglist,n);
			while(regcount<n)
			{
				fprintf(fp,"POP R%d\n",regcount);
				regcount++;
			}
			regcount++;			
			break;	
		case 'u':
			if(funcid!=NULL) //NOT MAIN
			{
				codegen(root->ptr1);
				fprintf(fp,"MOV R%d,-2\nMOV R%d,BP\nADD R%d,R%d\n",regcount,regcount+1,regcount,regcount+1);
				regcount++;
				if(funcid->type==3)
					fprintf(fp,"STRCPY R%d,R%d\n",regcount-1,regcount-2);
				else
					fprintf(fp,"MOV [R%d],R%d\n",regcount-1,regcount-2);
				regcount=regcount-2;
				endfn();
			}
			else
				fprintf(fp,"HALT\n");
			break;
		default:
			return;
	}
}
void pushargs(struct tree *a)
{
	if(a==NULL)
		return;
	if(a->ptr3!=NULL)
		pushargs(a->ptr3);
	codegen(a);
	fprintf(fp,"PUSH R%d\n",regcount-1);
	if(a->type==3)
	{
		fprintf(fp,"MOV R%d,SP\n",regcount);
		fprintf(fp,"STRCPY R%d,R%d\n",regcount,regcount-1);
	}
	regcount--;	
}
void popargs(struct tree *a,struct ArgStruct *arg,int n)
{	
	int regcount=n+1;
	if(a==NULL)
		return;
	fprintf(fp,"POP R%d\n",regcount);
	//printf(" %s %s-%d\n",a->name,arg->name,arg->passtype);
	if(arg->passtype==1)	//call by reference
	{
		regcount++;
		if(a->Gentry!=NULL)
			fprintf(fp,"MOV R%d,%d\n",regcount,a->Gentry->bind);
		else
		{
			fprintf(fp,"MOV R%d,%d\n",regcount,a->Lentry->bind);
			fprintf(fp,"MOV R%d,BP\n",regcount+1);
			fprintf(fp,"ADD R%d,R%d\n",regcount,regcount+1);
		}			
		regcount++;
		if(a->ptr1!=NULL)
		{
			codegen(a->ptr1);
			fprintf(fp,"ADD R%d,R%d\n",regcount-2,regcount-1);
			regcount--;
		}
		if(arg->type==3)
		{
			fprintf(fp,"MOV R%d,1\n",regcount);
			fprintf(fp,"MOV R%d,SP\n",regcount+1);
			fprintf(fp,"ADD R%d,R%d\n",regcount,regcount+1);
			fprintf(fp,"STRCPY R%d,R%d\n",regcount-1,regcount);
		}
		else
			fprintf(fp,"MOV [R%d],R%d\n",regcount-1,regcount-2);
		regcount=regcount-2;	
	}	
	popargs(a->ptr3,arg->next,n);
}
void endfn()
{
	struct Lsymbol *temp;
	temp=Lroot;
	while(temp!=NULL)
	{
		
		//printf("%s-%d\n",temp->name,temp->bind);
		if(temp->bind>0)
			fprintf(fp,"POP R%d\n",regcount);
		temp=temp->next;
	}
	fprintf(fp,"POP BP\nRET\n");	
}
void inorder(struct tree* root)
{
	if(root==NULL)
		return;
	if(root->nodetype!='c'&&root->nodetype!='i')
		printf("(");
	inorder(root->ptr1);
	
	
	if(root->nodetype=='c')
		printf("%d",root->value);
	
	if(root->nodetype=='i'||root->nodetype=='n')
		printf(" %s ",root->name);
	else
		printf(" %c ",root->nodetype);
		
	inorder(root->ptr2);
	if(root->nodetype!='c'&&root->nodetype!='i')
		printf(")");
}

struct Gsymbol * lookup(char * name)
{
	struct Gsymbol *temp=root;	
	while(temp!=NULL)
	{
		if(strcmp(name,temp->name)==0)
			return temp;
		temp=temp->next;
	}
	return NULL;
}

struct Lsymbol * Llookup(char * name)
{
	struct Lsymbol *temp=Lroot;	
	while(temp!=NULL)
	{
		if(strcmp(name,temp->name)==0)
			return temp;
		temp=temp->next;
	}
	return NULL;
}

struct tree * nontermcreate(char *name,struct tree *a,struct tree *b)
{
	struct tree *temp=malloc(sizeof(struct tree));
	temp->type=2;
	temp->nodetype='n';
	temp->name=name;
	temp->Gentry=NULL;
	temp->Lentry=NULL;
	temp->ptr1=a;
	temp->ptr2=b;
	temp->ptr3=NULL;
	return temp;
}
struct tree* maketree(struct tree *a,struct tree *b,struct tree *c,struct tree *d)
{
	//printf("%c\n",a->nodetype);
	if(a->nodetype=='i')		//checks in symbol table for variable and assigns pointer to it.
	{
		if(a->Gentry==NULL && a->Lentry==NULL)			
		{
			if(a->ptr1==NULL)
				a->Lentry=Llookup(a->name);
			if(a->Lentry==NULL)
				a->Gentry=lookup(a->name);
			if(a->Gentry==NULL && a->Lentry==NULL)
			{
				printf("\n Undeclared variable %s!!\n",a->name);
				exit(0);
			}
			if(a->Gentry!=NULL)
				a->type=a->Gentry->type;
			else
				a->type=a->Lentry->type;
			
		}
		if(b!=NULL)			//array operations
		{
			if(b->type==0)
				a->ptr1=b;
			else
			{
				printf("\n Array error !!\n");
				exit(0);
			}
		}
		return a;
	}
	
	if(a->type==2)		//for void type nodes (READ,PRINT,IF,WHILE,ASG,RETURN)	
	{
		if(c==NULL && d==NULL)		//for READ and PRINT - assigns the variable to ptr1. Also Return
		{
			a->ptr1=b;			
			if(a->nodetype=='u')	//RETURN
			{
				if(funcid!=NULL)	//NOT MAIN
				{					
					if(a->ptr1->type!=m3)
					{
						printf("\n Wrong type of return value in function %s !!\n",funcid->name);
						exit(0);
					}
				}
			}
			return a;
		}
		else			
		{
			if(b->nodetype=='i')	//for assignment statements
			{
				if(b->type==c->type)	
				{
					a->ptr1=b;
					a->ptr2=c;
					a->ptr3=d;
					return a;
				}
				else
				{
					printf("\n Type Error!!\n");
					exit(0);
				}
			}
			if(b->type==1)		//for IF statement and IF-ELSE statements and WHILE.
			{
				a->ptr1=b;
				a->ptr2=c;
				a->ptr3=d;
				return a;
			}
			else
			{
				printf("\n IF condition must have logical expression!!\n");
				exit(0);
			}
		}
	}
	else
	if(b!=NULL && c!=NULL && a->type==b->type && b->type==c->type)	//type checking of expressions and assigns b and c to ptr1 and ptr2 of a
	{
		a->ptr1=b;
		a->ptr2=c;
		a->ptr3=NULL;
		return a;
	}
	else if(c==NULL && a->type==b->type)		//for unary plus minus and NOT operator
	{
		struct tree* temp=malloc(sizeof(struct tree));
		temp->type=0;
		temp->nodetype='c';
		if(temp->type==0)
			temp->value=0;
		else
			temp->value=1;
		temp->ptr1=NULL;
		temp->ptr2=NULL;
		temp->ptr3=NULL;
		a->ptr1=temp;
		a->ptr2=b;
		a->ptr3=NULL;
		return a;
	}
	else if(a->type==1 && b->type==c->type)		//type checking of logical expression 
	{
		a->ptr1=b;
		a->ptr2=c;
		a->ptr3=NULL;
		return a;
	}
	else
	{
		printf("\n Type Error !!\n ");
		exit(0);
	}
}
void finstall(struct tree *node,int type,struct ArgStruct *arg)
{
	struct Gsymbol * temp;
	temp=lookup(node->name);
	if(temp==NULL)
	{
		temp=malloc(sizeof(struct Gsymbol));
		temp->name=node->name;
		temp->type=type;
		node->type=type;
		temp->arglist=arg;		
		//temp->binding=malloc(sizeof(int)*size);
		//temp->size=size;
		//temp->bind=memcount;
		//memcount=memcount+size;
		temp->next=root;
		root=temp;
	}
	else
	{
		printf("\nMultiple declaration of identifier %s !!\n",temp->name);
		exit(0);
	}
		/*struct ArgStruct *temp2;
		temp=lookup(node->name);
		temp2=temp->arglist;
		while(temp2!=NULL)
		{
			printf(" %s-%d\n",temp2->name,temp2->type);
			temp2=temp2->next;
		}*/
}
void install(struct tree *node,int type,int size)
{
	int i=0;
	struct Gsymbol * temp;
	temp=lookup(node->name);
	if(temp==NULL)
	{
		temp=malloc(sizeof(struct Gsymbol));
		temp->name=node->name;
		temp->type=type;
		node->type=type;
		temp->binding=NULL;
		temp->size=size;
		temp->bind=memcount;
		while(i<size)
		{
			fprintf(fp,"PUSH R0\n");
			i++;
		}
		memcount=memcount+size;
		temp->next=root;
		root=temp;
	}
	else
	{
		printf("\nMultiple declaration of variable %s !!\n",temp->name);
		exit(0);
	}
}
void Linstall(struct tree *node,int type,int size)
{
	struct Lsymbol * temp;
	temp=Llookup(node->name);
	if(temp==NULL)
	{
		temp=malloc(sizeof(struct Lsymbol));
		temp->name=node->name;
		temp->type=type;
		node->type=type;
		temp->binding=NULL;
		fprintf(fp,"PUSH R0\n");
		temp->bind=memcount;
		memcount=memcount+size;
		temp->next=Lroot;
		Lroot=temp;
	}
	else
	{
		printf("\nMultiple declaration of variable %s !!\n",temp->name);
		exit(0);
	}
}
struct ArgStruct * makearg(char *name, int type, int p)
{	
	struct ArgStruct *temp;
	temp=malloc(sizeof(struct ArgStruct));
	temp->name=name;
	temp->type=type;
	temp->passtype=p;
	temp->next=NULL;
	return temp;
}
struct ArgStruct * makeargtree(struct ArgStruct *a,struct ArgStruct *b)
{
	struct ArgStruct *temp=a;
	/*temp=b;
	while(temp->next!=NULL)
		temp=temp->next;
	temp->next=a;
	return(b);*/
	while(a->next!=NULL)
	{
		if(strcmp(a->name,b->name)==0)
		{
			printf("\n Argument declared more than once in function!!\n");
			exit(0);
		}
		a=a->next;	
	}
	a->next=b;
	return(temp);
}
void fdefcheck(struct tree *a,struct ArgStruct *b,int type)
{
	struct ArgStruct *temp;
	a->Gentry=lookup(a->name);
	if(a->Gentry==NULL)
	{
		printf("\n Undeclared function %s defined!!\n",a->name);
		exit(0);
	}
	if(a->Gentry->type!=type)
	{
		printf("\n Wrong return type of function %s !!\n",a->name);
		exit(0);
	}	
	temp=a->Gentry->arglist;
	while(temp!=NULL&&b!=NULL)
	{	//printf(" %s-%d %s-%d\n",temp->name,temp->type,b->name,b->type);
		if(temp->type!=b->type)
		{
			printf("\n Argument type mismatch in function %s !!\n",a->name);
			exit(0);
		}
		if(strcmp(temp->name,b->name)!=0)
		{
			printf("\n Arguments mismatch in function %s !!\n",temp->name,b->name,a->name);
			exit(0);
		}
		temp=temp->next;
		b=b->next;
	}
	if(temp!=NULL||b!=NULL)
	{
		printf("\n Arguments mismatch in function %s !!\n",a->name);
		exit(0);
	}
	fprintf(fp,"fn%d:\n",labelcount);
	a->Gentry->bind=labelcount;
	labelcount++;
	fprintf(fp,"PUSH BP\n");
	fprintf(fp,"MOV BP,SP\n"); 
}
void arglistinstall(struct tree *a)
{
	struct ArgStruct *temp;
	struct Lsymbol * temp2;
	int i=-3;
	temp=a->Gentry->arglist;
	while(temp!=NULL)
	{
		temp2=Llookup(temp->name);
		if(temp2==NULL)
		{
			temp2=malloc(sizeof(struct Lsymbol));
			temp2->name=temp->name;
			temp2->type=temp->type;
			//temp2->binding=malloc(sizeof(int));
			temp2->bind=i;
			i--;
			temp2->next=Lroot;
			Lroot=temp2;
		}
		else
		{
			printf("\nMultiple declaration of variable %s !!\n",temp2->name);
			exit(0);
		}
		temp=temp->next;
	}		
}
struct tree * makeparam(struct tree *a,struct tree *b)
{
	struct tree *temp;
	if(b->nodetype=='i')
	{
		if(b->Gentry==NULL && b->Lentry==NULL)			
		{
			if(b->ptr1==NULL)
				b->Lentry=Llookup(b->name);
			if(a->Lentry==NULL)
				b->Gentry=lookup(b->name);
			if(b->Gentry==NULL && b->Lentry==NULL)
			{
				printf("\n Undeclared variable %s as function parameter!!\n",b->name);
				exit(0);
			}
			if(a->Gentry!=NULL)
				b->type=b->Gentry->type;
			else
				b->type=b->Lentry->type;
			
		}
	}
	temp=a;
	while(temp->ptr3!=NULL)
		temp=temp->ptr3;
	temp->ptr3=b;
	return a;
}

struct tree * functioncall(struct tree * a, struct tree * b)
{
	struct ArgStruct *temp1;
	struct tree *temp2;
	a->Gentry=lookup(a->name);
	if(a->Gentry==NULL)
	{
		printf("\n Undeclared function %s!!\n",a->name);
		exit(0);
	}
	a->type=a->Gentry->type;
	temp1=a->Gentry->arglist;
	temp2=b;
	while(temp1!=NULL && temp2!=NULL)
	{
		//printf(" %s-%d %s-%d\n",temp1->name,temp1->type,temp2->name,temp2->type);
		if(temp2->type!=temp1->type)
		{
			printf("\n Type mismatch in call to function %s!!\n",a->name);
			exit(0);
		}
		if(temp1->passtype==1 && temp2->nodetype!='i')
		{
			printf("\n Call by reference need a variable in call to function %s!!\n",a->name);
			exit(0);
		}
		temp1=temp1->next;
		temp2=temp2->ptr3;
	}
	a->type=a->Gentry->type;
	a->nodetype='f';
	a->argument=b;
	a->ptr1=b;
	return(a);
}
