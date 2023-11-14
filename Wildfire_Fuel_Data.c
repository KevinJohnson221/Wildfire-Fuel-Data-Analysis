/**
 * Wildfire_Fuel_Data
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMBER_OF_FUELCOUNTING_THREADS 10 // number of threads that will count types of fuel
#define NUMBER_OF_DATAVALIDATION_THREADS 10 // number of threads that will check data validation
#define NUMBER_OF_FUELTYPE 13 // number of possibel fuel types
#define NUMBER_OF_ROWS_OR_COLUMNS_IN_SECTION 20 // how many rows or columns will be checked by a thread 
#define GRID_SIZE 200  // it is a 200x200 grid

//holds fuel data gotten from the text file, GRID_SIZE x GRID_SIZE 2D array
int fuelData[GRID_SIZE][GRID_SIZE] = {0};

// global arrays that are used to get data from the worker threads to the parent threads
//used for fuel type data
int fuelTypesCounter[NUMBER_OF_FUELCOUNTING_THREADS][NUMBER_OF_FUELTYPE] = {0};
int fuelTypesResults[NUMBER_OF_FUELTYPE] = {0};
//used for validation data
int dataValidationResults[NUMBER_OF_DATAVALIDATION_THREADS] = {0};


/* data structure for passing data to threads */
typedef struct
{
	int thread_number;
	int startRow_or_Column;
	int endRow_or_Column;
} parameters;

/*runner for a worker thread counting fuel types*/
void *fuel_counting(void *param)
{
	//get the parameters
	parameters *data = (parameters *) param;
	//local variables gotten from the parameters
	int threadNumber = data->thread_number;
	int startRow = data->startRow_or_Column;
	int endRow = data->endRow_or_Column;
	//array to count the numbers of each fuel type
	int fuelTypes[NUMBER_OF_FUELTYPE] = {0};
	
	//loops through a section of data and updates the fuelTypes array with the data from fuelData 
	//goes through NUMBER_OF_ROWS_OR_COLUMNS_IN_SECTION rows
	for (int i = startRow-1; i < endRow; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			if (fuelData[i][j] > 0 && fuelData[i][j] <= NUMBER_OF_FUELTYPE) {
				fuelTypes[fuelData[i][j]-1]++;
			}
		}
	}
	
	//prints the results from that thread and updates the global array for fuel types
	printf("Child thread %d: occurance for fuel type 1, 2, ..., are: ", threadNumber);
	for (int i = 0; i < NUMBER_OF_FUELTYPE; i++) {
		printf("%d ", fuelTypes[i]);
		//global array updated here
		fuelTypesCounter[threadNumber-1][i] = fuelTypes[i];
	}
	printf("\n");
	
	pthread_exit(0);
}

/*runner for a worker thread checking data validity*/
void *data_validation(void *param)
{
	//get the parameters
	parameters *data = (parameters *) param;
	//local variables gotten from the parameters
	int threadNumber = data->thread_number;
	int startColumn = data->startRow_or_Column;
	int endColumn = data->endRow_or_Column;
	
	//loops through a section of data and updates the global data validation array if there is invalid at all in that section
	//goes through NUMBER_OF_ROWS_OR_COLUMNS_IN_SECTION columns
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = startColumn-1; j < endColumn; j++) {
			//checks if a point is invalid
			if (fuelData[i][j] <= 0 || fuelData[i][j] > NUMBER_OF_FUELTYPE) {
				//if it is invalid, those results are printed
				printf("Child thread %d: data verification result: INVALID data exists\n", threadNumber);
				//global data validation array is updated and jumps to line 94
				dataValidationResults[threadNumber-NUMBER_OF_FUELCOUNTING_THREADS-1] = 1;
				goto invalid;
			}
		}
	}
	
	//line 93 can only be reached if all the data from the section is valid; those results are printed
	printf("Child thread %d: data verification result: VALID data\n", threadNumber);
	invalid:
	
	pthread_exit(0);
}

//main method
int main(int argc, char *argv[])
{
	// get fuel data from a txt file, which needs to be input from command line
	char filename[100];
	printf("Please enter your filename:\n");
	scanf("%s",filename);
	
	FILE *myFile;
	myFile = fopen(filename, "r");

	//read file into array
	if (myFile == NULL) {
		printf("Error Reading File\n");
		exit (0);
	}
	
	for (int i = 0; i < GRID_SIZE ; i++) {
		for (int j =0; j < GRID_SIZE; j++) {
			fscanf(myFile, "%d ", &fuelData[i][j] );
		}
	}
	
	//array of thread IDs 
	pthread_t workers[NUMBER_OF_FUELCOUNTING_THREADS+NUMBER_OF_DATAVALIDATION_THREADS];
	
	// create 10 threads for counting the occurance of fuel types
	for (int i = 0; i < NUMBER_OF_FUELCOUNTING_THREADS; i++) {
		//allocates memory for the data structure to pass parameters to the thread
		parameters *data = (parameters *) malloc(sizeof(parameters));
		//parameters for the thread for the section to be counted
		data->thread_number = i+1;
		data->startRow_or_Column = i*NUMBER_OF_ROWS_OR_COLUMNS_IN_SECTION+1;
		data->endRow_or_Column = (i+1)*NUMBER_OF_ROWS_OR_COLUMNS_IN_SECTION;
		//creates a thread for fuel counting
		pthread_create(&workers[i], NULL, fuel_counting, (void *)data);
	}
	
	// create 10 threads for counting the occurance of fuel types
	//z is a counter variable to help determine the rows or columns for the threads
	int z = 0;
	for (int i = NUMBER_OF_FUELCOUNTING_THREADS; i < NUMBER_OF_FUELCOUNTING_THREADS+NUMBER_OF_DATAVALIDATION_THREADS; i++) {
		//allocates memory for the data structure to pass parameters to the thread
		parameters *data2 = (parameters *) malloc(sizeof(parameters));
		//parameters for the thread for the section to be checked for validity
		data2->thread_number = i+1;
		data2->startRow_or_Column = z*NUMBER_OF_ROWS_OR_COLUMNS_IN_SECTION+1;
		data2->endRow_or_Column = (z+1)*NUMBER_OF_ROWS_OR_COLUMNS_IN_SECTION;
		//z incremented for the next thread
		z++;
		//creates a thread for fuel data validating
		pthread_create(&workers[i], NULL, data_validation, (void *)data2);
	}
	
	// wait for the threads to exit and combine the result and print
	for (int i = 0; i < NUMBER_OF_FUELCOUNTING_THREADS+NUMBER_OF_DATAVALIDATION_THREADS; i++) {
		pthread_join(workers[i], NULL);
	}
	
	//updates the final fuel counts in global results array using the data gathered by the threads 
	for (int i = 0; i < NUMBER_OF_FUELCOUNTING_THREADS; i++) {
		for (int j = 0; j < NUMBER_OF_FUELTYPE; j++) {
			fuelTypesResults[j] += fuelTypesCounter[i][j];
		}
	}
	
	//Parent thread prints results for the fuel type counting
	printf("Parent thread: occurance for fuel type 1, 2, ... are: ");
	for (int i = 0; i < NUMBER_OF_FUELTYPE; i++) {
		printf("%d ", fuelTypesResults[i]);
	}
	printf("\n");
	
	//Parent thread prints results for the fuel data validating
	printf("Parent thread: data validation (1 is invalid, 0 is valid): ");
	for (int i = 0; i < NUMBER_OF_DATAVALIDATION_THREADS; i++) {
		printf("%d ", dataValidationResults[i]);
	}
	printf("\n");
	
	return 0;
}
