#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <cstdlib>
using namespace std;
void sem_wait(int semid){ // function to implement wait() 
    struct sembuf sem_buffer;
    sem_buffer.sem_op = -1;
    sem_buffer.sem_num = 1;
    sem_buffer.sem_flg = 0;
    semop(semid , &sem_buffer , 1);
}
void signal(int semid){ // function to implement signal()
    struct sembuf sem_buffer;
    sem_buffer.sem_op = 1;
    sem_buffer.sem_num = 1;
    sem_buffer.sem_flg = 0;
    semop(semid , &sem_buffer , 1);
}
int main(){
    int n;
    cout<<"Enter the number of processes: ";
    cin>>n;
    int shared_memory_size = sizeof(int);
    int shmid = shmget(IPC_PRIVATE , shared_memory_size , IPC_CREAT | 0666); // Allocating shared memeory
    if(shmid < 0){
        cout<<"An error occured in making shared memory";
        return 0;
    }
    // allocating memory for counter in shared memory
    int *counter = (int*)(shmat(shmid , NULL , 0)); 
    *counter = 0;
    // creating semaphore
    int semid = semget(IPC_PRIVATE , 1 , IPC_CREAT | 0666);
    if(semid < 0){
        cout<<"Error creating semaphore."<<endl;
        return 0;
    }
    //initialzing value to 1 for binary semaphore
    semctl(semid , 0 , SETVAL , 1);
    for(int i=0 ; i<n ; ++i){
        pid_t pid = fork();
        if(pid==0){
            sem_wait(semid);
            cout<<"Process "<<getpid()<<" entered the critical section "<<endl;
            int val = *counter;
            val++;
            *counter = val;
            cout<<"Counter: "<<(*counter)<<endl;
             cout<<"Process "<<getpid()<<" exited the critical section "<<endl;
            shmdt(counter);
            signal(semid);
            exit(0);
        }
    }
    // terminating all processes and cleaning up memory
    for(int i = 0; i < n; i++) wait(NULL);
    shmdt(counter);
    shmctl(shmid , IPC_RMID , NULL);
    semctl(semid , 0 , IPC_RMID);
    return 0;
}