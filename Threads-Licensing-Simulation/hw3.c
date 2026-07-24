#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#define K 20
#define N 10

/* Structure representing a Car in the system.
 * Holds arrival time for aging mechanism, car ID, priority (VIP/Regular),
 * and a temporary VIP flag for dynamic promotion.
 */
typedef struct{
    long arrival_time;
    int id;
    int priority;
} Car;

/*
 * Node structure for the doubly linked list used in the Queues.
 * Contains a pointer to a Car and pointers to the next and previous nodes.
 */
typedef struct Node {
    Car* val;
    struct Node* next;
    struct Node* prev;
}Node;

/*
 * Queue structure representing a FIFO doubly linked list.
 * Maintains pointers to the head (for dequeue) and tail (for enqueue).
 */
typedef struct{
    Node* head;
    Node* tail;
}Queue;

//Global Queues for each station, separated by priority (Regular / VIP)
Queue RegularQ, vipQ, RegularAirPollutionQ, VipAirPollutionQ, RegularCashierQ, VipCashierQ;

//Semaphores for thread synchronization and resource management
sem_t mechanicsEmployes, airPollutionEmployes, cashier;
sem_t tablet, torque_wrench, carsInMech, carsInAirPollution;
sem_t isQfull, isQempty, airPollutionQisFull;
sem_t systemCapacity; // Limits the total number of cars in the institute to N

//Mutexes to protect shared Queues from concurrent access
sem_t mutexMechnics1, upgradeMutex;
sem_t mutexTester1, mutexCashier;

int mechanicsID[3]={1,2,3}, airPollutionID[2]={1,2};

//Initializes an empty Queue
void initQueue(Queue* Q){
    Q->head=NULL;
    Q->tail=NULL;
}

// Returns 1 if the Queue is empty, 0 otherwise
int isEmpty(Queue* Q){
    return (Q->head==NULL);
}

/* Thread-safe insertion at the tail of the Queue.
 * Handles the edge case of inserting into an empty queue.
 */
void enqueue(Queue* Q,Node* car){
    if(isEmpty(Q)){
        car->next=NULL;
        car->prev=NULL;
        Q->head=car;
        Q->tail=car;
    }
    else{
        Q->tail->next=car;
        car->prev=Q->tail;
        car->next=NULL;
        Q->tail=car;
    }
}

/* Thread-safe extraction from the head of the Queue.
 * Handles edge cases for empty queue and single-element queue.
 */
Node* dequeue(Queue* Q){
    if(isEmpty(Q)) return NULL;
    Node* temp;
    temp=Q->head;
     if(Q->head==Q->tail){
       Q->head=NULL;
       Q->tail=NULL; 
    }
    else{
        Q->head=Q->head->next;
        Q->head->prev=NULL;
    }
    
    //Completely detach the node from the queue before returning
    temp->next=NULL;
    temp->prev=NULL;
    return temp;
}

/*
 * Simulates a car entering the system.
 * Allocates memory, assigns ID, randomly determines VIP status (25% chance),
 * and records the exact arrival time in milliseconds.
 */
Car* carEntering(int id){
    Car* car;
    struct timeval tv;
    if((car=malloc(sizeof(Car)))==NULL){
        printf("ERROR in creating car\n");
        exit(1);
    }
    car->id=id;
    // 0 for regular car 
    // 1 for vip car (25% probability)
    if(rand()%4==0) car->priority=1;
    else car->priority=0;
    
    gettimeofday(&tv,NULL);
    //Calculate precise arrival time in milliseconds
    car->arrival_time=(tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    return car;
}

/*
 * Mechanic Station Thread Routine.
 * Mechanics wait for cars, acquire specific tools (tablet, torque wrench),
 * simulate work, and then forward the car to the Air Pollution station.
 */
void* mechanicStation(void* id){
    int myID=*(int*)id;
    struct timeval tv;
    struct tm *time_info;
    Node* carInService;
    
    while(1){
        //Wait for a car to be available in the mechanic queues
        sem_wait(&carsInMech);
        
        //Acquire necessary tools (limits concurrent mechanics based on tool availability)
        sem_wait(&tablet);
        sem_wait(&torque_wrench);
       
        //Critical segment: Dequeue a car (VIP priority first)
        sem_wait(&mutexMechnics1);
        if(!isEmpty(&vipQ)) carInService=dequeue(&vipQ);
        else carInService=dequeue(&RegularQ);
        sem_post(&mutexMechnics1);
        
        //Prints start of work
        gettimeofday(&tv,NULL);
        time_info=localtime(&tv.tv_sec);
        printf("[%02d:%02d:%02d.%03ld] [   Mech #%02d] Mech-%d: Working (Tools Acquired)\n",
            time_info->tm_hour, time_info->tm_min, time_info->tm_sec, (long)(tv.tv_usec / 1000),carInService->val->id,myID);
        
        //Simulate work duration beteween 0.1 to 0.2 seconds (100000 microseconds = 0.1 seconds)
        //(rand() % 100001)random number between 0 to 100000 
        usleep(100000 + (rand() % 100001));
        
        //Prints end of work
        gettimeofday(&tv,NULL);
        time_info=localtime(&tv.tv_sec);
        printf("[%02d:%02d:%02d.%03ld] [   Mech #%02d] Mech-%d: Finished & Returned Tools\n",
            time_info->tm_hour, time_info->tm_min, time_info->tm_sec, (long)(tv.tv_usec / 1000),carInService->val->id,myID);
        
        //Free the working tools for other mechanics
        sem_post(&tablet);
        sem_post(&torque_wrench);
        
        //Wait if the transit queue to Air Pollution is full (capacity limit of 3)
        sem_wait(&airPollutionQisFull);
        
        //Critical segment: Move car to the appropriate Air Pollution queue 
        //If the car belong to the regular Queue update its arrival time the the air pollution Queue
        sem_wait(&mutexTester1); 
        if(carInService->val->priority==1) enqueue(&VipAirPollutionQ,carInService);
        else{
            gettimeofday(&tv, NULL);
            carInService->val->arrival_time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
            enqueue(&RegularAirPollutionQ,carInService);
        }
        
        gettimeofday(&tv,NULL);
        time_info=localtime(&tv.tv_sec);
        printf("[%02d:%02d:%02d.%03ld] [   Mech #%02d] Mech-%d: Moved Car to Pollution Queue\n",
            time_info->tm_hour, time_info->tm_min, time_info->tm_sec, (long)(tv.tv_usec / 1000),carInService->val->id,myID);
        sem_post(&mutexTester1); 
        
        //Signal the Air Pollution station that a new car has arrived
        sem_post(&carsInAirPollution);
    }
}

/*
 * Air Pollution Station Thread Routine.
 * Testers wait for cars, process them, and forward them to the Payment station.
 */
void* airPollution(void* id){
    int myID=*(int*)id;
    Node* carInService;
    struct timeval tv;
    struct tm *time_info;
    
    while(1){
        // Wait for a car to arrive from the mechanic station
        sem_wait(&carsInAirPollution);
            
        // Critical Segment: Dequeue the car (VIP priority first)
        sem_wait(&mutexTester1);
        if(!isEmpty(&VipAirPollutionQ))  carInService=dequeue(&VipAirPollutionQ);
        else carInService=dequeue(&RegularAirPollutionQ);
        sem_post(&mutexTester1);
        
        // Signal the mechanics that a spot has opened up in the Air Pollution transit queue
        sem_post(&airPollutionQisFull);
        
        // Log start of pollution test
        gettimeofday(&tv,NULL);
        time_info=localtime(&tv.tv_sec);
        printf("[%02d:%02d:%02d.%03ld] [ Tester #%02d] Tester-%d: Starting Pollution Test\n", 
               time_info->tm_hour, time_info->tm_min, time_info->tm_sec, (long)(tv.tv_usec / 1000),carInService->val->id, myID);

        // Simulate test duration (0.15 seconds)
        usleep(150000);
            
        // Critical Segment: Move the car to the appropriate Payment Station queue
        sem_wait(&mutexCashier);
        if(carInService->val->priority==1) enqueue(&VipCashierQ,carInService);
        else{ 
            gettimeofday(&tv, NULL);
            carInService->val->arrival_time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
            enqueue(&RegularCashierQ,carInService);
        }
        // Log completion and departure to cashier
        gettimeofday(&tv,NULL);
        time_info=localtime(&tv.tv_sec);
        printf("[%02d:%02d:%02d.%03ld] [ Tester #%02d] Tester-%d: Done. Left Institute.\n", 
               time_info->tm_hour, time_info->tm_min, time_info->tm_sec, (long)(tv.tv_usec / 1000),carInService->val->id, myID);
        sem_post(&mutexCashier); 
        
        // Signal the cashier that a car is ready for payment
        sem_post(&cashier);
    }
}

/*
 * Payment Station Thread Routine (Cashier).
 * Processes payments, frees allocated memory, and signals system capacity
 */
void* paymentStation(){
    Node* carInService;
    while(1){
        // Wait for a car to arrive in the cashier queue
        sem_wait(&cashier);
        
        // Critical Segment: Dequeue the car
        sem_wait(&mutexCashier);
        if(!isEmpty(&VipCashierQ)) carInService=dequeue(&VipCashierQ);
        else carInService=dequeue(&RegularCashierQ);
        sem_post(&mutexCashier);

        // Free the memory allocated for the car data and the queue node
        free(carInService->val);
        free(carInService);
            
        // Signal the overall system that a car has left, allowing a new one to enter
        sem_post(&systemCapacity); 
    }
}

/*
 * Helper function for the Aging mechanism.
 * Checks a specific station's regular queue and promotes cars waiting over 400ms to VIP.
 */
void check_and_upgrade(Queue* regQ, Queue* vipQ, sem_t* station_mutex, long currTime) {
    Node* upgradedCar;
    struct timeval tv;
    struct tm *time_info;
    sem_wait(station_mutex); // Lock the specific station's queue
    
    //Check if the car at the head of the regular queue has starved (>400ms)
    while(!isEmpty(regQ) && (currTime - regQ->head->val->arrival_time >= 400)) {
        
        upgradedCar = dequeue(regQ);
        gettimeofday(&tv, NULL);
        time_info = localtime(&tv.tv_sec);
        
        //Prints the promotion
        printf("[%02d:%02d:%02d.%03ld] [ System #%02d] Aging Alert! Waited %03ldms -> Promoted to VIP\n", 
                time_info->tm_hour, time_info->tm_min, time_info->tm_sec, (long)(tv.tv_usec / 1000),
                upgradedCar->val->id, 
                currTime - upgradedCar->val->arrival_time);
        
        //Grant temporary VIP status and enqueue into the VIP queue of the same station
        enqueue(vipQ, upgradedCar);
    }
    sem_post(station_mutex);
}

/*
 * Aging Thread Routine.
 * Runs in the background, periodically checking all queues to prevent starvation.
 */
void* upgradeCar(void* arr){
    struct timeval tv;
    long currTime;
    
    while(1){
        gettimeofday(&tv, NULL);
        currTime = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
        
        // Check Mechanic queues
        check_and_upgrade(&RegularQ, &vipQ, &mutexMechnics1, currTime);
        
        // Check Air Pollution queues
        check_and_upgrade(&RegularAirPollutionQ, &VipAirPollutionQ, &mutexTester1, currTime);
        
        // Check Cashier queues
        check_and_upgrade(&RegularCashierQ, &VipCashierQ, &mutexCashier, currTime);

        // Sleep to prevent high CPU usage (50ms interval)
        usleep(50000);
    }
}

void main() {
    char* carType;
     Node* carNode;
    struct timeval tv;
    struct tm *time_info;
    //Insture every run the random numbers will be diffrent
    srand(time(NULL));

    // Initialize all Queues
    initQueue(&RegularQ);
    initQueue(&vipQ);
    initQueue(&RegularAirPollutionQ);
    initQueue(&VipAirPollutionQ);
    initQueue(&RegularCashierQ);
    initQueue(&VipCashierQ);

    // Initialize semaphores for resources and capacity limits
    sem_init(&tablet, 0, 2);
    sem_init(&torque_wrench, 0, 2);
    sem_init(&airPollutionQisFull, 0, 3);
    sem_init(&cashier, 0, 0);
    sem_init(&systemCapacity, 0, N); // Max N=10 cars inside the institute simultaneously

    // Initialize signaling semaphores (start at 0, incremented when a car arrives)
    sem_init(&carsInMech, 0, 0);
    sem_init(&carsInAirPollution, 0, 0);

    // Initialize Mutexes to protect shared data structures
    sem_init(&mutexMechnics1, 0, 1);
    sem_init(&mutexTester1, 0, 1);
    sem_init(&mutexCashier, 0, 1);

    pthread_t mechs[3], testers[2], cashier_thread, aging_thread;

    printf("--- Institute Open(N=%d, K=%d) ---\n",N,K);
     
    // Create Mechanic threads
    for(int i = 0; i < 3; i++) {
        pthread_create(&mechs[i], NULL, mechanicStation, &mechanicsID[i]);
    }
    // Create Air Pollution Tester threads
    for(int i = 0; i < 2; i++) {
        pthread_create(&testers[i], NULL, airPollution, &airPollutionID[i]);
    }
    // Create Cashier and Aging threads
    pthread_create(&cashier_thread, NULL, paymentStation, NULL);
    pthread_create(&aging_thread, NULL, upgradeCar, NULL);

    // Car generation loop (generates exactly K cars)
    for(int i = 1; i <= K; i++) {
        // Blocks here if system capacity (N) is reached
        sem_wait(&systemCapacity);
        
        carNode = malloc(sizeof(Node));
        carNode->val = carEntering(i);
        
        // Fetch current time for car arrival log
       
        gettimeofday(&tv, NULL);
        time_info = localtime(&tv.tv_sec);
        
        if(carNode->val->priority == 1)
            carType="VIP";
        else 
            carType="Regular";
        
        printf("[%02d:%02d:%02d.%03ld] [   Main #%02d] Arrived (%s)\n", 
               time_info->tm_hour, time_info->tm_min, time_info->tm_sec, (long)(tv.tv_usec / 1000),
               i, carType);
               
        // Critical Segment: Add new car to the mechanic's queue
        sem_wait(&mutexMechnics1);
        if (carNode->val->priority == 1) {
            enqueue(&vipQ, carNode);
        } else {
            enqueue(&RegularQ, carNode);
        }
        sem_post(&mutexMechnics1);
        
        // Wake up a mechanic
        sem_post(&carsInMech);
        
        // Delay between car arrivals (50ms)
        usleep(50000);
    }
    
    printf("--- All cars generated. Waiting for completion ---\n");
    
    // Wait until all N capacity spots are freed
    // Meaning all generated cars have fully exited the institute
    for(int i = 0; i < N; i++) {
        sem_wait(&systemCapacity);
    }
}
