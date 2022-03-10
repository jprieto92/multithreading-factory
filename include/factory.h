
#define MAX_BELT 8
#define MAX_DATABASE 16




//Non modifiable definitions
void * transporter(void);
void * receiver(void);
void * inserter(void * data);


int init_factory(char * file);
int close_factory();
