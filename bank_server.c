#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "Bank.c"

int main(int argc, char const *argv[])
{
    int num_worker_threads, num_accounts;
    char *output_file_name;
    FILE *output_file;

    // Check correct args
    if (argc != 4)
    {
        printf("Usage: bank_server <# worker threads> <# accounts> <output file>\n");
        return 0;
    }

    // Get args from command line
    num_worker_threads = atoi(argv[1]);
    num_accounts = atoi(argv[2]);
    strcpy(output_file_name, argv[3]);
    output_file = fopen(output_file_name, "w");

    // Init bank accounts
    if(!initialize_accounts(num_accounts))
    {
        printf("Error creating accounts.\n");
    }
    else 
    {
        printf("* Created %d accounts\n", num_accounts);
    }

    // Init worker threads
    for(int i = 0; i < num_worker_threads; i++)
    {
        // Create new thread
    }

    // Enter main thread loop
    while(1)
    {
        // Get user input

        // Add task to worker queue

    }


    return 0;
}

// routine for worker threads to run
void worker_routine()
{
    // Wait for tasks to be pushed to queuq

    // Pick up task

}
