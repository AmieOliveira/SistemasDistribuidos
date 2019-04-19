#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
// #include <mutex>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#define SHARED 0 

using namespace std;


//--------------------------------
sem_t semaphoreMutex;
sem_t semaphoreConsumers;
sem_t semaphoreProducers;
sem_t semaphoreVariable;
sem_t producerCount;
sem_t semaphoreSecurity;
int numberOfProducts;
//int productsProduced = 0;
//int productsConsumed = 0;
vector<int> sharedMemory;

int isPrime(int number)
{
  for(int i = 2; i < sqrt(number); ++i)
  {
    if(number % i == 0)
        {
          return 0;
        }
  }
  return 1;
}

int generateRandomNumber()
{
    //srand(time(NULL));
    int outPut = rand()%10000000 + 1;
    return outPut;
};


void print_buffer()
{
    //srand(time(NULL));
    for (int i = 0; i < sharedMemory.size(); ++i)
    {
       std::cout<<sharedMemory[i]<<'\t';
    }

};
int getFirstMemoryFreePosition()
{
    for (int i = 0; i < sharedMemory.size(); ++i)
    {
        if (sharedMemory[i] == 0) {
            return i;
        }
    }
    return -1;
}

int getFirstMemoryBusyPosition()
{
    for (int i = 0; i < sharedMemory.size(); ++i)
    {
        if (sharedMemory[i] != 0) {
            return i;
        }
    }
  return -1;
}

void *producer(void*arg)
{ //bool stopFlag = false;
//productsProduced=1;
    int productsProduced = 0;
  while(productsProduced<numberOfProducts)
  {
    // Create variable
    //cout<<"Aqui "<<productsProduced<<endl;
    int product = generateRandomNumber();
    std::cout<<"\nProducer"<<endl;
    sem_wait(&semaphoreProducers);
    sem_wait(&semaphoreMutex);
    // Add variable to buffer
    int pos = getFirstMemoryFreePosition();
    std::cout << "\nAdding number " << product << " to memory " << pos <<endl;
    sharedMemory[pos] = product;
    std::cout<<"\nProducts Produced: "<<productsProduced<<endl;
    print_buffer();
    productsProduced++;
    if (productsProduced>numberOfProducts){
        cout<<"Hey"<<endl;
        pthread_exit(0);
    }
    sem_post(&semaphoreMutex);
    sem_post(&semaphoreConsumers);
    //productsProduced+=1;
    }
    return 0;
}

void *consumer(void *arg)
{
  //bool stopFlag = false;
  int productsConsumed = 0;  
  while (productsConsumed<numberOfProducts)
  {
    
    if (sem_trywait(&producerCount)){
        pthread_exit(0);
    }
    else {
    std::cout<<"\nConsumer"<<endl;
    sem_wait(&semaphoreConsumers);
    sem_wait(&semaphoreMutex);
    // Get variable from buffer
    int pos = getFirstMemoryBusyPosition();
    int product = sharedMemory[pos];
    //std::cout << "\nGot number " << product << " from postion " << pos << endl;
    sharedMemory[pos] = 0;
    productsConsumed++;
    //if (productsProduced>numberOfProducts)
    //    {stopFlag = true;}
    std::cout<<"\nProducts Consumed: "<<productsConsumed<<endl;
    print_buffer();
    sem_post(&semaphoreMutex);
    sem_post(&semaphoreProducers);
    bool prime = isPrime(product);
    if (product ==-1)
    {
        std::cout<<"Erro" << endl;
    }
    if (prime and product!=-1)
    {
        //std::cout<<"The number: " << product <<" is Prime!" << endl;
    }
    //if (stopFlag){
    //    cout << "Exceeded total to be produced" << endl;
    //    return;}
    }
  }
  	return 0;
}


int main (int argc, char *argv[])
{
    int nP = 0 , nC = 0;
    if (argc < 5)
    {
        fprintf(stderr,"Usage %s : NumberOfProducer NumberOfConsumer NumberOfProductions MemorySize [OutFile] \n", argv[0]);
        exit(1);
    }

    int numberOfProducerThreads = atoi(argv[1]);
    int numberOfConsumerThreads = atoi(argv[2]);

    numberOfProducts = atoi(argv[3]); // 10000;
    //std::cout<<numberOfProducts<<endl;
    int sharedMemorySize = atoi(argv[4]); // 32;
    srand(time(NULL));
    string fileRoot;
    if (argc > 5) fileRoot = argv[5];
    else fileRoot = "efficiency";
    string filename = fileRoot + ".txt";

    sharedMemory.resize(sharedMemorySize,0);
    clock_t t_inic = clock(); // Clock Initial

    pthread_t pid[numberOfProducerThreads], cid[numberOfConsumerThreads];
    sem_init(&semaphoreMutex,0,1);
    sem_init(&semaphoreSecurity,0,1);
    sem_init(&semaphoreConsumers,0,0);
    sem_init(&semaphoreProducers,0,sharedMemorySize);
    sem_init(&producerCount,0,numberOfProducts);
    //sem_init(&producerCount,0,numberOfProducts);

    for(int i = 0; i < numberOfProducerThreads; i++) {	
		pthread_create(&pid[numberOfProducerThreads], NULL, producer, NULL);
		//pthread_create(&cid[N], NULL, Consumer, NULL);
	}
    for(int i = 0; i < numberOfConsumerThreads; i++) {	
		//pthread_create(&pid[N], NULL, Producer, NULL);
		pthread_create(&cid[numberOfConsumerThreads], NULL, consumer, NULL);
	}

    // And wait for them to finish.
	for(int i = 0; i < numberOfProducerThreads; i++) {
		pthread_join(pid[numberOfProducerThreads], NULL);
		//pthread_join(cid[N], NULL);
	}
    // And wait for them to finish.
	for(int i = 0; i < numberOfConsumerThreads; i++) {
		//pthread_join(pid[N], NULL);
		pthread_join(cid[numberOfConsumerThreads], NULL);
	}
    clock_t t_fim = clock();
    double totTime = (float)(t_fim-t_inic)/CLOCKS_PER_SEC;

    // Will write data to file
    //cout << "Writing to file '" << filename << "'" << endl;
    std::ofstream outfile;
    outfile.open(filename, std::ios_base::app);
    outfile << numberOfProducts << " " << sharedMemorySize << " " << numberOfProducerThreads
      << " " << numberOfConsumerThreads << " " << totTime << " " << endl;
    outfile.close();
    std::cout << "Done. Finishing program." << endl;

    std::cout << "\nCompleted semaphore program! with time:\n"<<totTime;
    std::cout << std::endl;

    return 0;
}
