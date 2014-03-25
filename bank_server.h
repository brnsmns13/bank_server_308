void worker_routine();
void process_request();
char* get_user_command();

static int INPUT_LENGTH = 1024;

typedef struct {
    pthread_mutex_t lock;
    int account_id;
} account;

typedef struct {
    char command[1024];
    struct timeval time_start;
    int request_id;
} request;

typedef struct {
	request req;
	struct node *next_node;
} node;