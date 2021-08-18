#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <pthread.h>
#include <fnmatch.h>
#include <signal.h> 
#include <dirent.h>
#define MAX_THREADS 300


pthread_t threads[MAX_THREADS];
int num_of_threads;
int count = 0;
int count_threads = 0;
size_t error_threads = 0;
char* search_str;
pthread_mutex_t  lock;
pthread_cond_t not_empty;

struct queue
{
    char *path;
    struct queue *next;

};

typedef struct queue queue;

queue *head;
queue *tail;



void release_queue()
{
    while (head)
    {
        queue *tmp = head -> next;
        free (head -> path);
        free (head);
        head = tmp;
    }
}

void cancel_handler(int signal)
{
    for (size_t i = 0; i < num_of_threads; ++i)
    {
        if (pthread_self() != threads[i])
            pthread_cancel(threads[i]);

    }

    pthread_mutex_destroy(&lock);

    printf(count);
    if(search_str)
    {
        free(search_str);
        search_str=NULL;
    }

    exit(0);
}

void sigint_handler(int signal)
{
    for (size_t i = 0; i < num_of_threads; ++i)
    {
        pthread_cancel(threads[i]);
    }
    release_queue();

    pthread_mutex_destroy(&lock);
    if(search_str)
    {
        free(search_str);
        search_str=NULL;
    }

    printf(count);

    exit(0);
}

char* dequeue()
{
    pthread_mutex_lock( &lock );

    while (!head)
        pthread_cond_wait(&not_empty ,&lock);
    ++count_threads;
    queue *tmp = head;
    head = head -> next;
    pthread_mutex_unlock( &lock );
    char* directory = tmp -> path;
    if(tmp)
    {
        free(tmp);
        tmp=NULL;
    }

    return directory;
}

void enqueue(char *folder)
{
    pthread_mutex_lock( &lock );
    queue * new_queue=malloc(sizeof(queue));
    new_queue->path=malloc(strlen(folder)+1);
    strcpy(new_queue->path,folder);
    new_queue->next=NULL;
    if (!head)
    {
        head = new_queue;
        tail = new_queue;
    }
    else
    {
        tail->next = new_queue;
        tail = tail->next;
    }
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock( &lock );
}

void treat_file(const char* path)
{

    if (fnmatch(search_str ,strrchr(path ,'/') + 1 ,0) == 0)
    {
        printf("%s\n", path);
        __sync_fetch_and_add(&count, 1);
    }
}

void exit_browse()
{
    perror("Error in opening directory");
    __sync_fetch_and_sub(&error_threads, 1);
    if (error_threads == num_of_threads)
    {
        release_queue();
        pthread_mutex_destroy(&lock);
        if(search_str)
        {
            free(search_str);
            search_str=NULL;
        }

        printf(count);


        exit(1);
    }
    pthread_exit(NULL);
}

void* browse()
{

    while (1)
    {
        char *path = dequeue();
        DIR *dir = opendir(path);
        if (dir == NULL)
        {
        exit_browse();
        }

        struct dirent *entry;
        struct stat dir_stat;
        while((entry = readdir(dir)) != NULL)
        {
            char buff[strlen(path)+strlen(entry -> d_name) + 2];
            sprintf(buff,"%s/%s", path, entry->d_name);
            stat(buff,&dir_stat);

            if(strcmp(entry->d_name,"..") != 0 && strcmp(entry->d_name, ".") != 0)
            {
                if((dir_stat.st_mode & __S_IFMT) == __S_IFDIR)
                    enqueue(buff);
                else
                    treat_file(buff);
            }
        }
        if(path)
        {
            free(path);
            path=NULL;
        }


        closedir(dir);
        __sync_fetch_and_sub(&count_threads, 1);
        if (!head && !count_threads)
        {
            raise (SIGUSR1);
        }
    }

}


int main(int argc, char **argv)
{
    struct sigaction cancel_sa;
    cancel_sa.sa_handler = cancel_handler;
    sigaction(SIGUSR1, &cancel_sa, NULL);
    struct sigaction sigint_sa;
    sigint_sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sigint_sa, NULL);

    if(argc != 4)
    {
        perror("Missing or extra arguments. mfind requieres exactly three arguments\n") ;
        printf("%d",argc);
        exit(1);
    }
    search_str = malloc(strlen(argv[2] + 3));
    sprintf(search_str, "%c%s%c", '*', argv[2], '*');
    num_of_threads = atoi(argv[3]);
    enqueue(argv[1]);
    tail = head;
    int rc = pthread_mutex_init( &lock, NULL );
    if( rc )
    {
        printf("ERROR in pthread_mutex_init(): "
               "%s\n", strerror(rc));
        exit(-1);
    }
    for (size_t i = 0; i < num_of_threads; ++i)
    {
        int rc = pthread_create(&threads[i], NULL, browse, (void *)i);
        if (rc)
        {
            printf("ERROR in pthread_create(): %s\n", strerror(rc));
            exit(-1);
        }
    }
    for (size_t i = 0; i < num_of_threads; ++i)
        pthread_join(threads[i], NULL);
    if(search_str)
    {
        free(search_str);
        search_str=NULL;
    }

    exit(0);
}
