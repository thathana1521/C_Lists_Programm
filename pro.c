#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define ARGUMENTS_ALLOWED 4
#define WORDLEN 30

typedef struct list {
	enum {sorted, not_sorted} type;
	union {
		int words;
		//char *word; Kati phgaine lathos kai den mporousa na xrhsimopoihsw swsta to list->fields.word
	}fields;
 	char *word;
	struct list *next;
	struct list *prev;
}list_T;


list_T *init_not_sortedlist();
list_T *init_sorted_list();
int arguments(int args, char *arg[]);
char *getWord(char *filename);  
void insert_node(list_T *head, char *word); 
void print_list (list_T *head);
list_T *construct_same_length (int length, char *dictionary, list_T *same_length);
char *random_word (list_T *head, int random_number);
list_T *construct_diff_letter (int length, list_T *same, list_T *head, char *word);
bool find_word(char *word, list_T *head);
void destroy_list (list_T *head);
int winner_check(char *chosen_word, char *target_word, list_T *path, int steps);


	
int main (int argc, char *argv[] ) {
	
	list_T *same_length, *diff_letter, *path_list;
	int steps, random_number_start, random_number_target;
	char *starting_word, *target_word, *word, *chosen_word;
	char play_again;
	
	
	arguments(argc, argv);
	
	/*initialization of same_length list */
	same_length=init_not_sortedlist();
	same_length=construct_same_length( atoi(argv[1]) , argv[3], same_length);
	
	if (same_length==NULL) {
		
		printf("No words with length=%d in this dictionary.\n",atoi(argv[1]) );
		return 0;
	}
	
	srand ((int)time (NULL) );
	do {
		steps=atoi(argv[2]);
		/* Picking starting and target word randomly */
		random_number_start=rand() % (same_length->fields.words);
		starting_word=strdup( random_word (same_length, random_number_start) );
		
		if ( strcmp (starting_word,"") == 0 ) {
			printf("Problem at creating random word.\n");
			exit (1);
		}
		
		/*If random_number_start == random_number_target pick another number. */
		do {
			random_number_target=rand() % (same_length->fields.words);
		}
		while ( random_number_start == random_number_target );
		
		
		target_word=strdup( random_word(same_length, random_number_target) );
		
		if ( strcmp (starting_word,"") == 0 ) {
			printf("Problem at creating random word.\n");
			exit (1);
		}
		
		/*Game begins.*/
		printf("\tStarting word: %s\n",starting_word);
		printf("\tTarget word: %s\n",target_word);
		printf("Try to go from %s to %s in no more than %d steps\n", starting_word, target_word, atoi(argv[2]));
		
		/*Initializing and constructing diff_letter list. */
		diff_letter=init_sorted_list();
		word= (char *)malloc(sizeof(char)*( atoi(argv[1])+1 ) );
		strcpy(word, starting_word);
		diff_letter=construct_diff_letter( atoi(argv[1]), same_length, diff_letter, word );  
		
		/*Initialize and start constructing path list. */
		path_list=init_not_sortedlist();
		insert_node(path_list, starting_word);
		
		/*Loop when the user starts entering words. Terminates if steps==0 OR the target_word is found*/
		do {
			
			/*Loop checking if the user entered a word that exists in the list. */
			do{
				printf("\n");
				printf("Progress so far: ");
				print_list(path_list);
				printf("Remaining steps: %d\n",steps);
				printf("Target: %s\n",target_word);
				printf("\n");
				printf("Choose one of the following words:\n");
				if ( diff_letter != NULL ) {
					print_list(diff_letter);
				}
				else {
					printf("There are no words to choose from. Sorry!\n");
					return 0;
				}
				printf("\n");
				printf("Enter selection: ");
				chosen_word= (char *)malloc(sizeof(char)*( atoi(argv[1])+1 ) );
				scanf("%s",chosen_word);
			}
				
			/*If the word that the user selected does not exist in the list. */
			while(find_word(chosen_word,diff_letter)!=true);
			
			insert_node(path_list, chosen_word);
			
			/*Destroying, initializing and constructind the diff_letter list for the next round.*/
			destroy_list(diff_letter);
			diff_letter=init_sorted_list();
			diff_letter=construct_diff_letter( atoi(argv[1]), same_length, diff_letter, chosen_word ); 
			steps--;
		
		}
		while (winner_check(chosen_word, target_word, path_list, steps) ==0);
	
		printf("Play again? (y/n):");
		scanf(" %c", &play_again);
			
		while (play_again!='y' && play_again!='n' && play_again!='N' && play_again!='Y') {
		
			printf("Please enter y for yes or n for no. :");
			scanf(" %c", &play_again);
		}
		
	}
	while(play_again=='y' || play_again=='Y');
	
	/*Freeing allocated memory.*/
	destroy_list(diff_letter);
	destroy_list(same_length);
	destroy_list(path_list);

	return 0;
}

/* Initialization of list with type=not_sorted */

list_T *init_not_sortedlist() {
	list_T *head;
	head = (list_T *)malloc(sizeof(list_T));
	if (head == NULL) {
		printf("Memory allocation error in initialization\n");
		exit(1);
	}
	
	head->type=not_sorted;
	head->fields.words=0;
	
	/* Both pointers of sentinel should point to itself on 
	 * initialization, as the list should behave as circular
	 * no matter whether you scan it forward (using the next
	 * pointers) or backwards (using the prev pointers) */
	head->next = head;
	head->prev = head;
	return head;
}

/* Initialization of list with type=sorted */

list_T *init_sorted_list() {
	list_T *head;
	head = (list_T *)malloc(sizeof(list_T));
	if (head == NULL) {
		printf("Memory allocation error in initialization\n");
		exit(1);
	}
	
	head->type=sorted;
	head->fields.words=0;
	
	/* Both pointers of sentinel should point to itself on 
	 * initialization, as the list should behave as circular
	 * no matter whether you scan it forward (using the next
	 * pointers) or backwards (using the prev pointers) */
	head->next = head;
	head->prev = head;
	return head;
}

/* Boolean function to find a word at the diff_letter list. Returns true if found. */
bool find_word(char *word, list_T *head) {
	list_T *current;
	
	for (current=head->next; current!=head; current=current->next) {
	
		if ( strcmp(word, current->word)==0 ) {
			return true;
		}
	}
	printf("\n");
	printf("ERROR: Your selection is not available. Try again:");
	
	return false;
}

/* Argument checking. */
int arguments(int args, char *arg[]) {
	
	if (args<ARGUMENTS_ALLOWED) {
	
		printf("Insufficient arguments.\n");
		printf("%s LENGTH STEPS DICTIONARY\n",arg[0]);
		printf("\tLENGTH: requested length of word.\n");
		printf("\tSTEPS: requested number of steps. Must be >0\n");
		printf("\tDICTIONARY: name of dictionary file\n");
		
		return 0;
	}
	else if(args>ARGUMENTS_ALLOWED) {
		
		printf("Too many arguments.\n");
		printf("%s LENGTH STEPS DICTIONARY\n",arg[0]);
		printf("\tLENGTH: requested length of word.\n");
		printf("\tSTEPS: requested number of steps. Must be >0\n");
		printf("\tDICTIONARY: name of dictionary file\n");
		
		return 0;
	}
    
	while ( atoi(arg[2]) <= 0) {
		
		
		printf("You may not have fewer than one steps. Specify a different number of steps: ");
		
		scanf("%s",arg[2]);
	}
	
	while ( atoi(arg[1])<=0) {
		
		printf("You may not have fewer than one length. Specify a different number of steps: ");
		scanf("%s",arg[1]);
	}
		
	return 1;
}

/* Given the name of a file, read and return the next word from it, 
or NULL if there are no more words */

char *getWord(char *filename)  {
	char formatstr[15], *word;
	static FILE *input;
	static int firstTime = 1;
	if (firstTime) { 
		input = fopen(filename, "r");
		if (input == NULL) {
			printf("ERROR: Could not open file \"%s\"\n", filename);
			exit(1);
		}
		firstTime = 0;
	}
	word = (char*)malloc(sizeof(char)*WORDLEN);
	if (word == NULL) {
		printf("ERROR: Memory allocation error in getWord\n");
		exit(1);
	}
	sprintf(formatstr, "%%%ds", WORDLEN-1);
	fscanf(input, formatstr, word);
	if (feof(input)) {
		fclose(input);
		firstTime = 1;
		return NULL;
	}
	return word;
}


/* Inserting nodes in the list depending on the type of the list. */

void insert_node(list_T *head, char *word) {
	list_T *new_node, *current;
	
	if (head->type==not_sorted) {
		
		new_node = (list_T *)malloc(sizeof(list_T));
		new_node->word = (char *)malloc(sizeof(char)*(strlen(word)+1));
		if (new_node == NULL) {
			printf("Problem\n");
			exit(1);
		}
		
		/* Fix the fields of the new node */
		new_node->word=strdup(word);
		
		/* The next of the new node will be the next of the sentinel */
		new_node->next = head->next;
		/* The prev of the new node will be the sentinel itself */
		head->next->prev=new_node;
		/* Now fix the pointers of neighbouring nodes in the list */ 
		/* The prev of the next of the new node should point to the new node */
		head->next = new_node;
		new_node->prev = head;
	}
	else {
		
		new_node = (list_T *)malloc(sizeof(list_T));
		new_node->word = (char *)malloc(sizeof(char)*(strlen(word)+1));
		
		if (new_node == NULL) {
			printf("Problem\n");
			exit(1);
		}		new_node->word=strdup(word);
		
		if (head->next==head) {
			
				/* The next of the new node will be the next of the sentinel */
			new_node->next = head->next;
			/* The prev of the new node will be the sentinel itself */
			new_node->prev = head;
			
			/* Now fix the pointers of neighbouring nodes in the list */ 
			/* The prev of the next of the new node should point to the new node */
			head->next = new_node;
			head->prev = new_node;
		}
		else {
			
			current = head->next;
			
			/*Find the word that is less than the word_given and place it before the word.*/
			
			while ((strcmp( current->word, word) < 0 ) && current!=head) {
				
				 current = current->next;
				 
			}
			/*Fixing pointers*/
			new_node->next=current;
			new_node->prev=current->prev;
			new_node->prev->next= new_node;
			current->prev=new_node;
		}
	}
}


/* Printing list depending on the type of the list. */
void print_list (list_T *head) {
	
	list_T *current;
	int count=0;
	
	if (head->type==not_sorted) {
		
		for (current = head->prev; current != head; current = current->prev) {
			count++;
			
			if (count>1) {
				printf("-->");
			}
			printf("%s",current->word);
			if (count%4==0) {
				printf("\n");
			}
		}
		printf("\n");
	}
	else {
		for (current = head->next; current != head; current = current->next) {
	
			count++;
			printf("%s ",current->word);
			
			if (count%4==0) {
				printf("\n");
			}
		}
	}
}

/* Freeing any allocated memory of the list. */
void destroy_list (list_T *head) {
	list_T *current, *temp;
	
	current=head->next;
	while ( current!=head) {
		
		temp=current;
		current=current->next;
		free(temp->word);
		free (temp);
	}
	free (head);
}

/*Constructing same length list.*/
list_T *construct_same_length (int length, char *dictionary, list_T *head) {
	
	char *word;
	
	word=getWord(dictionary);
	
	while (word!=NULL) {
		
		if (strlen(word)==length) {
			insert_node(head,word);
			head->fields.words++;
		}
		
		word=getWord(dictionary);
	}
	
	if (head->fields.words>=2) {
		return head;
	}
	else {
		return NULL;
	}
}

/*Constructing diff letter list.*/
list_T *construct_diff_letter (int length, list_T *same, list_T *head, char *word ) {
	
	int i,count;
	list_T *current;
	
	current=same;
	
	for (current=same->next; current!=same; current=current->next) {
		count=0;
		
		for (i=0; i<length; i++) { 
			
			if (current->word[i]==word[i]) {
				count++;
			}
			
		}
		
		if (count==length-1) {
			insert_node(head,current->word);
			head->fields.words++;
		}
	}
	if (head->fields.words > 0) {
		return head;
	}
	else {
		return NULL;
	}	
}

/* winner_checking function. returns 1 if the player loses or wins. */
int winner_check(char *chosen_word, char *target_word, list_T *path, int steps) {
	
	if ( strcmp(chosen_word,target_word)==0 ) {
		printf("GOOD WORD! ");
		print_list(path);
		return (1);
	}
	
	if (steps==0) {
		printf("\n");
		printf("TOO BAD :(");
		print_list(path);
		printf("\n");
		printf("Target was %s",target_word);
		printf("\n");
		return (1);
	}
	
	return 0;
}

/*Random word choosing from same length list.*/
char *random_word (list_T *head, int random_number) {
	
	int count;
	list_T *current;
	
	count=0;
	
	for (current = head->next; current != head; current = current->next) {
		if ( count == random_number ) {
			return current->word;
		}
		count++;
	}
	
	return NULL;
}