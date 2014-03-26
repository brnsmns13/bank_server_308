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
FILE *output_file;
account *accounts;

int main(int argc, char const *argv[])
{
    int num_worker_threads, num_accounts, request_number;
    char *output_file_name;
    pthread_t threads[num_worker_threads];

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
    output_file = fopen(output_file_name, "w");

    if(output_file == NULL)
    {
        printf("- Error creating output file\n");
    }

    // Init bank accounts
    if(!initialize_accounts(num_accounts))
    {
        printf("- Error creating accounts.\n");
        return 0;
    }
    else 
    {
        //printf("* Created %d accounts\n", num_accounts);
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
        if(!strcmp("END", command))
        {
            printf("END %d\n", request_number);
            run = 0;
            break;
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
            //printf("* Creating new root\n");
            root = (node *) malloc(sizeof(node));
            root->next_node = NULL;
            root->req = req;
            last = root;
        }
        else if(last->next_node == NULL)
        {
            //printf("* Adding node to end\n");
            node *new_node;
            new_node = (node *) malloc(sizeof(node));
            new_node->req = req;
            new_node->next_node = NULL;
            last->next_node = new_node;
            last = new_node;

        }
        pthread_mutex_unlock(&list_mutex);

        // node *iter;
        // iter = (node *) malloc(sizeof(node));
        // iter = root;
        // while(iter != NULL)
        // {
        //     printf("ID: %d\tCommand: %s\n", iter->req.request_id, iter->req.command);
        //     iter = iter->next_node;
        // }
    }

    // Do cleanup

    fclose(output_file);
    return 0;
}

// routine for worker threads to run
void worker_routine()
{
    // Wait for tasks to be pushed to queue
    while(run || root != NULL)
    {
        pthread_mutex_lock(&list_mutex);
        request req;
        if(root != NULL)
        {
            req = root->req;
            root = root->next_node;
            // struct node* temp;
            // temp = root;
            // req = root->req;
            // root = root->next_node;
            // free(temp);
            pthread_mutex_unlock(&list_mutex);
            //printf("THREAD - ID: %d\tCommand: %s\n", req.request_id, req.command);
            process_request(req);
        }
        else
        {
            pthread_mutex_unlock(&list_mutex);
        }
    }
}

// Process a request in the worker thread
void process_request(request req)
{
    //fprintf(output_file, "ID: %d\tCOMMAND: %s\n", req.request_id, req.command);
    printf("ID %d\n", req.request_id);
    char *cmd;
    char *tok;
    cmd = malloc(sizeof(char) * INPUT_LENGTH);
    strcpy(cmd, req.command);
    tok = strsep(&cmd, " ");
    //printf("CMD: %s\tARGS: %s\n", tok, cmd);

    // Run associated function
    if(!strcmp("TRANS", tok))
    {
        do_transaction(req.request_id, cmd, req.time_start);
    }
    else if(!strcmp("CHECK", tok))
    {
        do_balance(req.request_id, atoi(cmd), req.time_start);
    }
}

// Handle transaction requests
void do_transaction(int request_id, char *transaction_str, struct timeval time_start)
{
    char *temp;
    int trans_accounts[10] = {0};
    int trans_amounts[10] = {0};
    int ISF = 0;

    // Process transaction command string
    int trans_count = 0;
    while((temp = strsep(&transaction_str, " ")) != NULL)
    {
        trans_accounts[trans_count] = atoi(temp);
        temp = strsep(&transaction_str, " ");
        if(temp == NULL)
            break;
        trans_amounts[trans_count++] = atoi(temp);
    }

    // Lock each account mutex
    int i;
    for (i = 0; i < trans_count; i++)
    {
        account acct = accounts[trans_accounts[i]-1];
        pthread_mutex_lock(&acct.lock);
    }

    // Check sufficient funds for each account
    for (i = 0; i < trans_count; i++)
    {
        account acct = accounts[trans_accounts[i]-1];
        int balance = read_account(acct.account_id);
        if(balance + trans_amounts[i] < 0)
        {
            ISF = 1;
            break;
        }
    }
    if(ISF)
    {
        // write status to file
        struct timeval time_end;
        gettimeofday(&time_end, NULL);
        fprintf(output_file, "%d ISF %d TIME %d.%06d %d.%06d\n", request_id, trans_accounts[i], time_start.tv_sec, time_start.tv_usec, time_end.tv_sec, time_end.tv_usec);
    }

    // Do the actual transactions
    else {
        for (i = 0; i < trans_count; i++)
        {
            account acct = accounts[trans_accounts[i]-1];
            int balance = read_account(acct.account_id);
            write_account(acct.account_id, balance + trans_amounts[i]);
        }

        // write status to file
        struct timeval time_end;
        gettimeofday(&time_end, NULL);
        fprintf(output_file, "%d OK TIME %d.%06d %d.%06d\n", request_id, time_start.tv_sec, time_start.tv_usec, time_end.tv_sec, time_end.tv_usec);
    }

    // unlock accounts
    for (i = 0; i < trans_count; i++)
    {
        account acct = accounts[trans_accounts[i]-1];
        pthread_mutex_unlock(&acct.lock);
    }

}

// Handle balance requests
void do_balance(int request_id, int account_id, struct timeval time_start)
{
    int balance;
    account acct;

    acct = accounts[account_id-1];
    pthread_mutex_lock(&acct.lock);
    balance = read_account(acct.account_id);
    pthread_mutex_unlock(&acct.lock);
    struct timeval time_end;
    gettimeofday(&time_end, NULL);

    // Write status to file
    fprintf(output_file, "%d BAL %d TIME %d.%06d %d.%06d\n", request_id, balance, time_start.tv_sec, time_start.tv_usec, time_end.tv_sec, time_end.tv_usec);
    //printf("%d BAL %d TIME %d.%06d %d.%06d\n", request_id, balance, time_start.tv_sec, time_start.tv_usec, time_end.tv_sec, time_end.tv_usec);
}

// Read line from STDIN and return
char* get_user_command()
{
    char user_in[INPUT_LENGTH];
    fgets(user_in, INPUT_LENGTH, stdin);
    user_in[strlen(user_in)-1] = 0;
    return user_in;
}