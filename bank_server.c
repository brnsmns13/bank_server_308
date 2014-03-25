#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "Bank.c"
#include "bank_server.h"

node *root;
node *last;
pthread_mutex_t list_mutex;
int run = 1;

int main(int argc, char const *argv[])
{
    int num_worker_threads, num_accounts, request_number;
    char *output_file_name;
    FILE *output_file;
    pthread_t threads[num_worker_threads];
    account *accounts;

    // Check correct args
    if (argc != 4)
    {
        printf("Usage: bank_server <# worker threads> <# accounts> <output file>\n");
        return 0;
    }

    // Get args from command line
    num_worker_threads = atoi(argv[1]);
    num_accounts = atoi(argv[2]);
    output_file_name = (char *) malloc(sizeof(char *) * 128);
    strcpy(output_file_name, argv[3]);
    output_file = fopen(output_file_name, "w+");

    // Init bank accounts
    if(!initialize_accounts(num_accounts))
    {
        printf("- Error creating accounts.\n");
        return 0;
    }
    else 
    {
        printf("* Created %d accounts\n", num_accounts);
    }

    // Create mutex for each account
    int i;
    accounts = (int *) malloc(sizeof(account) * num_accounts);
    for(i = 1; i <= num_accounts; i++)
    {
        account acct;
        pthread_mutex_init(&acct.lock, NULL);
        acct.account_id = i;
        accounts[i-1] = acct;
    }
    // Init worker threads
    for(i = 0; i < num_worker_threads; i++)
    {
        // Create new thread
        pthread_create(&threads[i], 0, worker_routine, NULL);
    }

    // Enter main thread loop
    request_number = 1;
    // root = (node *) malloc(sizeof(node));
    // last = (node *) malloc(sizeof(node));
    root = NULL;
    last = NULL;
    root = last;
    pthread_mutex_init(&list_mutex, NULL);
    while(run)
    {
        // Get user input
        char *command;
        command = get_user_command();

        // Check for exit
        if(!strcmp("EXIT", command))
        {
            run = 0;
            break;
        }

        if(!strcmp("REM", command))
        {
            if(root != NULL)
            {
                printf("* Removing root node: ID %d, Command, %s\n", root->req.request_id, root->req.command);
                root = root->next_node;
            }
            continue;
        }

        // Create a new request
        request req;
        gettimeofday(&req.time_start, NULL);
        strcpy(req.command, command);
        req.request_id = request_number++;

        // Push request to queue
        pthread_mutex_lock(&list_mutex);

        if(root == NULL)
        {
            printf("* Creating new root\n");
            root = (node *) malloc(sizeof(node));
            root->next_node = NULL;
            root->req = req;
            last = root;
        }
        else if(last->next_node == NULL)
        {
            printf("* Adding node to end\n");
            node *new_node;
            new_node = (node *) malloc(sizeof(node));
            new_node->req = req;
            new_node->next_node = NULL;
            last->next_node = new_node;
            last = new_node;

        }
        pthread_mutex_unlock(&list_mutex);

        node *iter;
        iter = (node *) malloc(sizeof(node));
        iter = root;
        while(iter != NULL)
        {
            printf("ID: %d\tCommand: %s\n", iter->req.request_id, iter->req.command);
            iter = iter->next_node;
        }
    }

    // Do cleanup


    return 0;
}

// routine for worker threads to run
void worker_routine()
{
    // Wait for tasks to be pushed to queue

    // Pick up task

}

// Take a single request
void process_request(request req)
{

}

// Read line from STDIN and return
char* get_user_command()
{
    char user_in[INPUT_LENGTH];
    fgets(user_in, INPUT_LENGTH, stdin);
    user_in[strlen(user_in)-1] = 0;
    return user_in;

}