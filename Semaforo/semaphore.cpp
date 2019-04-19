#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <cmath>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

using namespace std;


//--------------------------------
sem_t* semaphoreMutex;
sem_t* semaphoreConsumers;
sem_t* semaphoreProducers;

int productsProduced = 0;
int productsConsumed = 0;
vector<int> sharedMemory;

int isPrime(int number){
  for(int i = 2; i <= sqrt(number); ++i)
  {
    if(number % i == 0)
        {
          return 0;
        }
  }
  return 1;
}

int generateRandomNumber(){
    int outPut = rand()%10000000 + 1;
    return outPut;
};

int getFirstMemoryFreePosition(){
    for (int i = 0; i < sharedMemory.size(); ++i)
    {
        if (sharedMemory[i] == 0) {
            return i;
        }
    }
    return -1;
}

int getFirstMemoryBusyPosition(){
    for (int i = 0; i < sharedMemory.size(); ++i)
    {
        if (sharedMemory[i] != 0) {
            return i;
        }
    }
  return -1;
}

void producer(int numberOfProducts){
  bool stopFlag = false;
  while(productsProduced<=numberOfProducts)
  {
    // Create variable
    int product = generateRandomNumber();

    // cout << "Entering producer semaphore" << endl;

    sem_wait(semaphoreProducers);
    sem_wait(semaphoreMutex);
    // Add variable to buffer
    int pos = getFirstMemoryFreePosition();
    if (pos == -1){     // FIXME: This is not supposed to happen.
        cout << "P: Error in memory access" << endl;
        sem_post(semaphoreMutex);
        sem_post(semaphoreConsumers);
        continue;
    }
    // cout << "Adding number " << product << " to memory " << pos <<endl;
    sharedMemory[pos] = product;
    productsProduced+=1;
    if (productsProduced>numberOfProducts)
        stopFlag = true;
    sem_post(semaphoreMutex);
    sem_post(semaphoreConsumers);

    if (stopFlag){
        // cout << "Exceeded total to be produced" << endl;
        return;
    }
  }
}

void consumer(int numberOfProducts){
  bool stopFlag = false;
  while(productsConsumed<=numberOfProducts){
    // cout << "Entering consumer semaphore" << endl;

    sem_wait(semaphoreConsumers);
    sem_wait(semaphoreMutex);
    // Get variable from buffer
    int pos = getFirstMemoryBusyPosition();
    int product = sharedMemory[pos];
    if (pos == -1){     // FIXME: This is not supposed to happen.
        cout << "C: Error in memory access" << endl;
        sem_post(semaphoreMutex);
        sem_post(semaphoreProducers);
        continue;
    }
    // cout << "Got number " << product << " from postion " << pos << endl;
    sharedMemory[pos] = 0;
    productsConsumed+=1;
    if (productsProduced>numberOfProducts)
        stopFlag = true;
    sem_post(semaphoreMutex);
    sem_post(semaphoreProducers);

    bool prime = isPrime(product);
    if (product ==-1){
        std::cout << "Erro" << endl;
    }
    if (prime and product!=-1){
        std::cout<<"The number: " << product <<" is Prime!" << endl;
    }

    if (stopFlag){
        // cout << "Exceeded total to be produced" << endl;
        return;
    }
  }
}



int main (int argc, char *argv[]){
    int nP = 0 , nC = 0;
    if (argc < 5)
    {
        fprintf(stderr,"Usage %s : NumberOfProducer NumberOfConsumer NumberOfProductions MomorySize [OutFile] \n", argv[0]);
        exit(1);
    }

    int numberOfProducerThreads = atoi(argv[1]);
    int numberOfConsumerThreads = atoi(argv[2]);

    int numberOfProducts = atoi(argv[3]); // 10000;
    int sharedMemorySize = atoi(argv[4]); // 32;

    srand(time(NULL));

    string fileRoot;
    if (argc > 5) fileRoot = argv[5];
    else fileRoot = "efficiency";
    string filename = fileRoot + ".txt";

    sharedMemory.resize(sharedMemorySize,0);

    semaphoreMutex = sem_open("semaphoreMutex", O_CREAT, 0644, 1);
    semaphoreConsumers = sem_open("semaphoreConsumers", O_CREAT, 0644, 0);
    semaphoreProducers = sem_open("semaphoreProducers", O_CREAT, 0644, sharedMemorySize);

    if ( semaphoreMutex == SEM_FAILED ){
        cout << "ERROR: Couldn't create mutex semaphore" << endl;
        cout << "       " << strerror(errno) << endl;
        return 1;
    }
    if ( semaphoreConsumers == SEM_FAILED ){
        cout << "ERROR: Couldn't create consumer semaphore" << endl;
        cout << "       " << strerror(errno) << endl;
        return 1;
    }
    if ( semaphoreProducers == SEM_FAILED ){
        cout << "ERROR: Couldn't create producer semaphore" << endl;
        cout << "       " << strerror(errno) << endl;
        return 1;
    }

    clock_t t_inic = clock(); // Clock Initial

    int totalNumberOfThreads = numberOfProducerThreads + numberOfConsumerThreads;
    thread allThreads[totalNumberOfThreads];

    //Register producer threads
    for (int i = 0; i < numberOfProducerThreads; ++i)
    {
        allThreads[i] = thread(producer,numberOfProducts);
    }

    //Register consumer threads
    for (int i = numberOfProducerThreads; i < totalNumberOfThreads; ++i)
    {
        allThreads[i] = thread(consumer,numberOfProducts);
    }

    // Merge all threads to the main thread
    for(int id = 0; id < totalNumberOfThreads; id++)
        allThreads[id].join();

    clock_t t_fim = clock();
    double totTime = (float)(t_fim-t_inic)/CLOCKS_PER_SEC;

    // Will write data to file
    cout << "Writing to file '" << filename << "'" << endl;
    std::ofstream outfile;
    outfile.open(filename, std::ios_base::app);
    outfile << numberOfProducts << " " << sharedMemorySize << " " << numberOfProducerThreads
      << " " << numberOfConsumerThreads << " " << totTime << " " << endl;
    outfile.close();
    cout << "Done. Finishing program." << endl;

    std::cout << "\nCompleted semaphore program!\n";
    std::cout << std::endl;

    return 0;
}