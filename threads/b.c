/*********************************************************************

Tested with:

2 flowers, 13 bees (passed)

*********************************************************************/

#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>


/** GLobal variables *************************************************/
int maxBees = 3;
int beeCapacity = 2;

/* Bee stuff **************************************************/

/* Create an object as the field of flowers. Limit the number of flower / number of bees accepted on the field. */

struct node {
   int polen; // Amount of polen
   int bee;   // Which bee
   struct node *next;
}; // Flower

struct node *head = NULL;
struct node *curr = NULL;
int t = 0;
int size = 0;

struct node* create_list(int polen)
{
    printf("\n[FIELD] Creating... \n");
    struct node *ptr = (struct node*)malloc(sizeof(struct node));

    if(NULL == ptr)
    {
        printf("\n [FIELD] Node creation failed :( \n");
        return NULL;
    }

    ptr->polen = polen;
    ptr->bee = -1;
    ptr->next = NULL;

    head = curr = ptr;
    return ptr;
}

struct node* add_to_list(int polen, int who, bool add_to_end)
{
    if(NULL == head)
    {
        return (create_list(polen));
    }

   /* if(add_to_end)
        printf("Adding %d to the end of the list... \n",polen);
    else
        printf("Adding %d to the beginning of the list...\n",polen); */

    struct node *ptr = (struct node*)malloc(sizeof(struct node));
    if (NULL == ptr)
    {
        printf("\n[FIELD] Node creation failed :( \n");
        return NULL;
    }

    ptr->polen = polen;
    ptr->bee = who;
    ptr->next = NULL;

    if(add_to_end)
    {
        curr->next = ptr;
        curr = ptr;
    }

    else
    {
        ptr->next = head;
        head = ptr;
    }
    return ptr;
}

struct node* search_flower(int polen, struct node **prev)
{
    struct node *ptr = head;
    struct node *tmp = NULL;
    bool found = false;

//    printf("[FIELD] Searching for %d... \n",polen);

    while(ptr != NULL)
    {
        if(ptr->polen == polen)
        {
            found = true;
            break;
        }
        else
        {
            tmp = ptr;
            ptr = ptr->next;
        }
    }

    if(true == found)
    {
        if(prev)
            *prev = tmp;
        return ptr;
    }
    else
    {
        return NULL;
    }
}

struct node* search_bee(int bee, struct node **prev)
{
    struct node *ptr = head;
    struct node *tmp = NULL;
    bool found = false;

//    printf("[FIELD] Searching for Bee(%d)... \n",bee);

    while(ptr != NULL)
    {
        if(ptr->bee == bee)
        {
            found = true;
       //   printf("[FIELD] Bee(%d) found!\n", bee);
            break;
        }
        else
        {
            tmp = ptr;
            ptr = ptr->next;
        }
    }

    if(true == found)
    {
        if(prev)
            *prev = tmp;
        return ptr;
    }
    else
    {
        return NULL;
    }
}

int delete_from_list(int polen)
{
    struct node *prev = NULL;
    struct node *del = NULL;

 //   printf("[FIELD] Deleting %d... \n", polen);

    del = search_flower(polen,&prev);
    if(del == NULL)
    {
        return -1;
    }
    else
    {
        if(prev != NULL)
            prev->next = del->next;

        if(del == curr)
        {
            curr = prev;
        }
        else if (del == head)
        {
            head = del->next;
        }

    }

    free(del);
    del = NULL;

    return 0;
}

void assign_bee(int bee)
{
	struct node *x = NULL;
	x = search_bee(-1, NULL); // search for available flower
	if (NULL == x) // All flowers occupied - bye
        {
		printf("[DRONE] All flowers occupied, no room for Bee(%d)!\n", bee);
	}
	else
	{
		x->bee = bee;
		printf("[DRONE] Bee(%d) found a flower. \n", bee);
	}
}

void fly_bee_out(int bee)
{
	struct node *x = NULL;
	x = search_bee(bee, NULL);
	if (NULL == x) // bee not in, - OK
        {
		printf("[DRONE] Bee(%d) not here!\n", bee);
	}
	else
	{
		x->bee = -1;
	}
}

void print_list(void)
{
    struct node *ptr = head;
 
    if (size == 0)
    {
	printf("[FIELD] List is empty.\n");
	return;
    }

    printf("[FIELD] Printing... \nList = { ");
    while(ptr != NULL)
    {
        printf("\n(polen = %d; bee = %d) ",ptr->polen, ptr->bee);
        ptr = ptr->next;
    }
    printf("} \n");

    return;
}

/* Thread stuff *******************************************************/


// Mutex for field access -> RWlock may be used here
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Mutex and cond for max bees accessing the field at one time -> sem
pthread_mutex_t mutexMaxBees = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condMaxBees = PTHREAD_COND_INITIALIZER;

// Mutex and cond for master thread -> sem
pthread_cond_t condBadFlowers = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexBadFlowers = PTHREAD_MUTEX_INITIALIZER;
int in = 0;

void* polen(void *arg)
{
    int id, j, rc;
    struct node *x = NULL;
    id = *((int*) arg);
    struct node *ptr = head;

    while (1)
    {  
        x = search_bee(id, NULL);
        if (NULL == x)
        {
            printf("[DRONE] Bee(%d) searches for flower. %d bees in.\n", id, in);
	}
	else
	{
	       printf("[DRONE] Bee(%d) already here. %d bees in.\n", id, in);
	       break;
	}

        pthread_mutex_lock(&mutexMaxBees);
        while (in >= maxBees)
	{
		printf("[EAGER BEE] There are %d bees in. Bee(%d) is waiting...\n",in,id);		
		rc = pthread_cond_wait(&condMaxBees, &mutexMaxBees);
	}
		assign_bee(id);
	        in++;
		x = search_bee(id, NULL);
		if (NULL == x)
        		{
          		  break;
			}
		//printf("[BUSY BEE] Bee(%d) flies in. %d in.\n", id, in);
	
	pthread_mutex_unlock(&mutexMaxBees);

	printf("[DRONE] Bee(%d) is on a flower!\n", id);

	pthread_mutex_lock(&mutex);
	if (ptr == NULL) 
       // list is empty
	{
		printf("[DRONE] Bee(%d) flies out.\n", id);
		fly_bee_out(id);
		pthread_mutex_unlock(&mutex);
		break;
	}

	int v = x-> polen - beeCapacity;

	// printf("[BUSY BEE] Collecting pollen %d by Bee(%d) -> %d...\n", x->polen, x->bee, v);
	x->polen = v;
	
	if (v < beeCapacity)
	{
 		t++;
		printf("[DRONE] There are %d bad flowers.\n",t);
	}
	
	ptr = head;

	if (t >= 1 || size < 2)
	{
		printf("[DRONE] Warning BEEKEEPER. %d flowers left.\n",size);
	        t = 5;
		pthread_cond_broadcast(&condBadFlowers);
	}

	pthread_mutex_unlock(&mutex);

	printf("[DRONE] Bee(%d) flies out.\n",id);
	fly_bee_out(id);

	pthread_mutex_lock(&mutexMaxBees);
	in--;
	printf("[DRONE] There are %d bees in.\n",in);
	pthread_mutex_unlock(&mutexMaxBees);

	if (in < maxBees)
	{
		printf("[DRONE] BEES can come now.\n");
		pthread_cond_broadcast(&condMaxBees);
	      //  if (size == 0) exit(0);
	}

	sleep(1+rand()%5);	

    }
} 

void* elim (void* arg)
{
	int d;
while(1)
{
	
	if (size == 0)
	{
		printf("[BEEKEEPER] BEEKEEPER thread has no reason to wait.\n");
		break;
	}	

	pthread_mutex_lock(&mutexBadFlowers);
        while (t < 5)
	{
	        printf("[BEEKEEPER] Waiting (Snoring)... %d flowers left.\n", size);		
		pthread_cond_wait(&condBadFlowers, &mutexBadFlowers);
	}
		printf("[BEEKEEPER] What? I'm awake!\n");
	pthread_mutex_unlock(&mutexBadFlowers);

	pthread_mutex_lock(&mutex);

	struct node *del = head;
        while(del != NULL)
        {
        if (del->polen < beeCapacity)
	{
        /*
	   d = delete_from_list(del->polen);
	   
           if (d == 0) 
	   { 
 	   size--;
	   printf("[BEEKEEPER] Job done. Must rest now.\n");
 	   }
		else printf("[BEEKEEPER] Failed miserably.\n");
	
        */

	   del->polen += beeCapacity * 1.25; // added more polen to the flower
	   printf("[BEEKEEPER] Fed flower.\n");
	}
	del = del->next; 
        }

	t = 0;
	
	sleep(5); // slow things down so behaviour can be observed.
	pthread_mutex_unlock(&mutex);

	printf("[BEEKEEPER] Fed all hungry flowers. Must rest now.\n");
}
}

int main() 
{
char s[128];
int nrf, nrb;

printf("[USER] Insert number of flowers:\n");
gets(s);
nrf = atoi(s);
size = nrf;
printf("[USER] Insert number of bees:\n");
gets(s);
nrb = atoi(s);

maxBees = nrf;
beeCapacity = 10;

printf("\n[USER] Given %d flowers and %d bees.\nA bee can carry %d amount of polen, and only %d bees can be in the field at one time.\n", nrf, nrb, beeCapacity, maxBees);
gets(s);

// List operations

int j = 0, ret = 0; int who = 0;
struct node *ptr = NULL;

for(j = 0; j<nrf; j++)
{
   ret = abs(rand()%100);
   who = -1; // abs(rand()%nrb); - all flowers are free
   add_to_list(ret, -1, false);

}

print_list();

// Thread operations

pthread_t thr[nrb]; // one thread for each bee
pthread_t main_t;
int i;
int rc = 0;

srand(time(0));
for (i = 0; i<nrb; i++)
{
    pthread_create(&thr[i], 0, polen, &i);
    sleep(1);
}

pthread_create(&main_t, 0, elim, NULL);

// Clean up...

for(i=0; i<nrb; i++) 
{
	pthread_join(thr[i], NULL);
}

pthread_join(main_t, NULL);

print_list();

pthread_cond_destroy(&condMaxBees);
pthread_mutex_destroy(&mutexMaxBees);
pthread_mutex_destroy(&mutex);

return 0;
}
