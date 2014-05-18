#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
int n = 0;
char s[20];

printf("[USER] Insert number of bees: ");
gets(s);
n = atoi(s);

int i = 0;

for (i=0;i<n;i++)
{
 	if(system("./c"))
	{
	  	perror("\n");
 	}
}
}
