/*********************************************************************

Tested with:

3 flowers, 15 bees (passed)

*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

pid_t pid;
sem_t *field;

/** Bee stuff ******************************************************/
void flyBeeIn()
{

    int pid = getpid();
    printf("[BUSY BEE] Bee(%i), I want POLEN.\n", pid);
    sem_wait(field);

    printf("[DRONE] Bee(%i) flies in.\n", pid);
    sleep(rand()%2); // Yeah, it does.

    printf("[DRONE] Bee(%i) is on a flower.\n", pid);
    sleep(3);   // Bee collecting pollen

    sem_post(field);
    printf("[DRONE] Bee(%i) left.\n", pid);
}

/** Field stuff **************************************************/
int main()
{
	int i;
	char s[128];
	int nrf, nrb;

	printf("[USER] Insert number of flowers:\n");
	gets(s);
	nrf = atoi(s);

	printf("[USER] Insert number of bees:\n");
	gets(s);
	nrb = atoi(s);

	printf("\n[USER] Given %d flowers and %d bees.\nOnly %d bees can be in the field at one time.\n", nrf, nrb, nrf);
	gets(s);

	// Initialize the field as shared memory
	field = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if( field == MAP_FAILED )
	{
    	    printf("Error creating shm.\n");
    	    exit(1);
	}
	
	// Semaphore to restrict the number of bees on the field at one time
	if(sem_init(field, 1, nrf) < 0)
    	{
        	perror("Error creating semaphore.\n");
        	exit(1);
	}

	srand(time(NULL));

    	// Create bees.. One process per bee
	for (i = 0; i < nrb; i++)
    	{
        	sleep((rand()%2 + 1));
        	pid = fork();

        	if(pid < 0)
        	{
            	perror("Error fork.\n");
            	exit(1);
        	}

        	if(pid == 0)
        	{
           	flyBeeIn();
            	_exit(0);
        	}
    	}	

	// Clean up...
    	while (waitpid(-1, NULL, 0))
        	if (errno == ECHILD) // no children to wait for
            	break;

    	sem_destroy(field);
    	munmap(field, sizeof(sem_t));

    return 0;
}


