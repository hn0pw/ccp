
int socket2;
char* fileContents[255];
static pthread_mutex_t line_locking_mutex[255];
//volatile char* line_locking[255];
int answerSent;
int error;

void *start(void *socket_pointer);
