#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 50
#define MAX_CANIDATE_TABLE_SIZE 100

typedef struct node{
    char* word;
    struct node* next;
    struct node* prev;
}node;

/*
    burst is how long the process is
    quantum is how long the processor spends on each process before it moves on
    after a quantum is over move onto next available process
    if idle, time does not increase when new process arrives
    add quantum to time per process
    remember that if burst reaches 0 before quantum just add time spent on process to time
*/

void insertNode(node** head, node** tail);
node* startsWithLetter(FILE* input, char firstLetter, node** tail);
//3 functions for Shortest Job First Algorithm
void runSJF(node* head);
int findShortestBurst(int* burstArr, int length, int* arrivalArr, int* shortest, int time);
int decrementBurstArr(int* burstArr, int index);

//functions for Roundrobin
void runRr(node *head);


//int i=0;

int main(int argc,char* argv[]){

    FILE *inputFile = fopen(argv[1], "r");


  /*char fileInput[5000];
    while(fgets(fileInput,5000, inputFile))
        printf("%s",fileInput);
    printf("\n");
    */

    //initialize nodes
    node* tail=NULL;
    node* head=NULL;

    //creates first node
    insertNode(&head, &tail);

    char firstC;
    firstC=fgetc(inputFile);

    while(firstC!=EOF){
        //if letter adds entire word to a node then gets then next char
        if(isalpha(firstC)){
            tail=startsWithLetter(inputFile, firstC, &tail);
            insertNode(&head,&tail);
            firstC=fgetc(inputFile);
        }
        //if number adds entire word to a node then gets then next char
        else if(isdigit(firstC)){
            tail=startsWithLetter(inputFile, firstC, &tail);
            insertNode(&head,&tail);
            firstC=fgetc(inputFile);
        }
         //if symbol adds entire word to a node then gets then next char
        else if(ispunct(firstC)){
            if(firstC!='#'){
                tail=startsWithLetter(inputFile, firstC, &tail);
                insertNode(&head,&tail);
                firstC=fgetc(inputFile);
            }
            else{
              //  firstC=fgetc(inputFile);
                while(firstC!='\n'){
                    firstC=fgetc(inputFile);
                }
            }
        }
        else{
         //if whitespace, prints it out to keep everything in order
            //printf("%c",firstC);
            firstC=fgetc(inputFile);
        }
    }



    //int i=0;

/*    while(head->next!=NULL){
        printf("%s\n",head->word);
        head=head->next;
    }
*/
    fclose(inputFile);

	//used for testing runSJF in development
    //runSJF(head);
	//runRr(head);
    return 0;

}

void runSJF(node* head)
{
    //get process count
	head = head->next;
	int pCount = atoi(head->word);	

	//get runfor
	head = head->next->next;
	int runFor = atoi(head->word);
	
	//skip "use sjf"
	head = head->next->next;

	//declare array for names, arrival time, burst time
	//	arrival time, and finish time(used for calculating turnaround
	char nameArr[pCount][30];
	int arrivalArr[pCount];
	int burstArr[pCount];
	int *waitArr = calloc(pCount, sizeof(int));
	int finArr[pCount];
		

	//get process names, arrival, burst times
	int i;
	for(i=0;i<pCount;i++)
	{
		//get process name
		head = head->next->next->next;
		strcpy(nameArr[i], head->word);
		
		//get arrival time
		head = head->next->next;
		arrivalArr[i] = atoi(head->word);

		//get burst time
		head = head->next->next;
		burstArr[i] = atoi(head->word);
	}

	//print first 3 lines
	printf("%d processes\n",pCount);
	printf("Using Shortest Job First (Pre)\n\n");

	int shortest=-1;
	int time = 0;
	int finished;

	while(time <= runFor)
	{
		//reset finished checker
		finished = 0;		

		//check for arrivals
		for(i=0;i<pCount;i++)
		{
			if(arrivalArr[i] == time)
			{
				printf("Time %d: %s arrived\n", time, nameArr[i]);
			}
		}

		//get shortest burst time
		int prevShortest = shortest;
		int found = findShortestBurst(burstArr, pCount, arrivalArr, &shortest, time);
		//printf("Shortest is: %d \n",shortest);
		
		//if no process to run is found print idle
		if(found == -1 && time < runFor)
		{
			printf("Time %d: IDLE\n",time);
			time++;
		}

		//if still running the same process as before, don't print anything
		else if(prevShortest == shortest)
		{
			//run the same process and add to the turnaround time
			finished = decrementBurstArr(burstArr, shortest);
			if(finished == 1)
			{
				printf("Time %d: %s finished\n", time+1, nameArr[shortest]);
				finArr[shortest] = time+1;
			}

			//increase the wait time for other processes (not finished)
			for(i=0;i<pCount;i++)
			{
				if(i!=shortest && burstArr[i] > 0 && arrivalArr[i] <= time)
				{
					waitArr[i]++;
				}
			}
			time++;
		}

		//if new process is selected
		else if(prevShortest !=shortest)
		{
			//select and run new process and add to the turnaround time
			printf("Time %d: %s selected (burst %d)\n", time, nameArr[shortest], burstArr[shortest]);
			finished = decrementBurstArr(burstArr, shortest);
			if(finished == 1)
			{
				printf("Time %d: %s finished\n", time+1, nameArr[shortest]);
				finArr[shortest] = time+1;
			}

			//increase the wait time for other processes (not finished)
			for(i=0;i<pCount;i++)
			{
				if(i!=shortest && burstArr[i] > 0 && arrivalArr[i] <= time)
				{
					waitArr[i]++;
				}
			}
			time++;
		}
	}

	printf("Finished at time %d\n\n",runFor);
	for(i=0;i<pCount;i++)
	{
		printf("%s wait %d turnaround %d\n", nameArr[i], waitArr[i], (finArr[i]-arrivalArr[i]));
	}
}

int findShortestBurst(int* burstArr, int length, int* arrivalArr, int* shortest, int time)
{
	int tempShortest = 123456789;
	int found = 0;
	int i;
	//find shortest
	for(i=0;i<length;i++)
	{
		if((burstArr[i] < tempShortest) && (burstArr[i] > 0) && (arrivalArr[i] <= time))
		{
			tempShortest = burstArr[i];
			*shortest = i;
			found = 1;
		}
	}
	if(found == 0)
	{
		return -1;
	}
	return 1;
}

int decrementBurstArr(int* burstArr, int index)
{
	burstArr[index]--;
	if(burstArr[index] == 0)
	{
		return 1;
	}
	return 0;
}

void runRr(node* head)
{
    //get process count
	head = head->next;
	int pCount = atoi(head->word);
	printf("pCount: %d\n",pCount);	

	//get runfor
	head = head->next->next;
	int runFor = atoi(head->word);
	printf("runFor: %d\n",runFor);	

	//skip "use rr"
	head = head->next->next;

	//get quantum
	head = head->next->next;
	int quantum = atoi(head->word);
	printf("quantum: %d\n",quantum);

	//declare array for names, arrival time, burst time
	//	arrival time, and finish time(used for calculating turnaround
	char nameArr[pCount][30];
	int arrivalArr[pCount];
	int burstArr[pCount];
	int *waitArr = calloc(pCount, sizeof(int));
	int finArr[pCount];
	int *readyArr = calloc(pCount, sizeof(int));

	//get process names, arrival, burst times
	int i;
	for(i=0;i<pCount;i++)
	{
		//get process name
		head = head->next->next->next;
		strcpy(nameArr[i], head->word);
		printf("processname: %s\n",nameArr[i]);		

		//get arrival time
		head = head->next->next;
		arrivalArr[i] = atoi(head->word);
		printf("arrival time: %d\n",arrivalArr[i]);

		//get burst time
		head = head->next->next;
		burstArr[i] = atoi(head->word);
		printf("burst time: %d\n",burstArr[i]);
	}

	//print first 3 lines
	printf("%d processes\n",pCount);
	printf("Using Round-Robin\n");
	printf("Quantum %d\n\n", quantum);

	int time = 0;
	int readyCount = 0;
	int qCount = 1;
	int selected;
	int selectedIndex = 0;
	int finished;

	while(time<=runFor)
	{
		qCount--;

		//reset finished checker
		finished = 0;
		
		//check for arrivals
		for(i=0;i<pCount;i++)
		{
			if(arrivalArr[i] == time)
			{
				printf("Time %d: %s arrived\n", time, nameArr[i]);
			}
			
			//add the arrived process into the readyarray
			readyArr[readyCount++] = i;
		}
		
		//if ran for quantum time, pick new one
		if(qCount == 0)
		{
			selected = -1;
			//select if burst time not 0 (start from next available process)
			if(selectedIndex+1 >= pCount)
			{
				selectedIndex = 0;
			}
			else
			{
				selectedIndex++;
			}
			for(i=selectedIndex;i<readyCount;i++)
			{
				if(burstArr[readyArr[i]]>0 && arrivalArr[readyArr[i]] <= time)
				{
					selected = readyArr[i];
					selectedIndex = i;
					printf("Time %d: %s selected (burst %d)\n", time, nameArr[selected], burstArr[selected]);
					qCount = quantum;
					break;
				}
			}
			//if none selected previously, start from beginning of list
			if(selected == -1)
			{
				for(i=0;i<readyCount;i++)
				{
					if(burstArr[readyArr[i]]>0 && arrivalArr[readyArr[i]] <= time)
					{
						selected = readyArr[i];
						selectedIndex = i;
						printf("Time %d: %s selected (burst %d)\n", time, nameArr[selected], burstArr[selected]);
						qCount = quantum;
						break;
					}
				}
			}
			
			//if still not selected, show idle
			if(selected == -1)
			{
				printf("Time %d: IDLE\n",time);
			}
		}
		//decrement burst array for selected process
		if(selected != -1)
		{
			finished = decrementBurstArr(burstArr, selected);
			if(finished == 1)
			{
				printf("Time %d: %s finished\n", time+1, nameArr[selected]);
				qCount = 1;
				finArr[selected] = time+1;
			}
			for(i=0;i<pCount;i++)
			{
				if(i != selected && burstArr[i] != 0 && arrivalArr[i] <= time)
				{
					waitArr[i]++;
				}
			}
		}
		time++;
	}
		
	for(i=0;i<pCount;i++)
	{
		printf("%s wait %d turnaround %d\n",nameArr[i],waitArr[i],(finArr[i]-arrivalArr[i]));
	}

}


node* startsWithLetter(FILE* input, char firstLetter, node** tail){
    int nextLetter,len=30;
    char* word=calloc(len+1,sizeof(char));
    int i=1;

    word[0]=firstLetter;
    nextLetter=fgetc(input);

    //while next char is letter or number keeps scanning to make char array
    while(isalpha(nextLetter)||isdigit(nextLetter)||ispunct(nextLetter)){
        if(i>=len){
            len*=2;
            word=realloc(word,len+1);
        }

        word[i]=nextLetter;
        i++;
        nextLetter=fgetc(input);
    }

    if (nextLetter != EOF )
        fseek(input, -1, SEEK_CUR);

    //used to print input without comments
    //printf("%s ",word);
    (*tail)->word=word;

    return *tail;
}


void insertNode(node** head, node** tail){
    if(*head==NULL){
        *head=(node*)malloc(sizeof(node));
        (*head)->next=NULL;
        (*head)->prev=NULL;
        *tail=*head;
    }
    else{

        (*tail)->next=(node*)malloc(sizeof(node));
        (*tail)->prev=*tail;
        *tail=(*tail)->next;
        (*tail)->next=NULL;
    }
}
