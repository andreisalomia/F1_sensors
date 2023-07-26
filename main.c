#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

/*shortcut for struct name*/
#define P_M_U power_management_unit

void (*func[8])();

void *get_operations(void (**)());

void display(int i, sensor *arr, int nr_sensors)
{
	/*Printing each sensor's data*/
	if (i < 0 || i >= nr_sensors)
	{
		printf("Index not in range!\n");
		return;
	}
	if (arr[i].sensor_type == 1)
	{
		printf("Power Management Unit\n");
		/*Using auxiliary variable to get the data from the void pointer*/
		P_M_U aux = *(P_M_U *)arr[i].sensor_data;
		printf("Voltage: %.2f\n", aux.voltage);
		printf("Current: %.2f\n", aux.current);
		printf("Power Consumption: %.2f\n", aux.power_consumption);
		printf("Energy Regen: %d%%\n", aux.energy_regen);
		printf("Energy Storage: %d%%\n", aux.energy_storage);
	}
	else
	{
		printf("Tire Sensor\n");
		tire_sensor aux = *(tire_sensor *)arr[i].sensor_data;
		printf("Pressure: %.2f\n", aux.pressure);
		printf("Temperature: %.2f\n", aux.temperature);
		printf("Wear Level: %d%%\n", aux.wear_level);
		/*Checking if the performance score was previously calculated*/
		if (aux.performace_score)
			printf("Performance Score: %d\n", aux.performace_score);
		else
			printf("Performance Score: Not Calculated\n");
	}
}

void sort(int tail, sensor *arr, int nr_sensors)
{
	/*Sorting the tire part of the main array of sensors by switching the
	first and last, then first+1 last-1 so on*/
	sensor temp;
	while (tail < (nr_sensors - 1))
	{
		temp = arr[tail];
		arr[tail] = arr[nr_sensors - 1];
		arr[nr_sensors - 1] = temp;
		tail++;
		nr_sensors--;
	}
}

void operation(void (*f[])(), int i, sensor *arr, int nr_sensors)
{
	if (i < 0 || i >= nr_sensors)
	{
		printf("Index not in range!\n");
		return;
	}
	int j;
	/*Performing the requested operation by calling the array of
	pointers to functions*/
	for (j = 0; j < arr[i].nr_operations; j++)
		(*f[arr[i].operations_idxs[j]])(arr[i].sensor_data);
}

int check_sensor(sensor *array, int i)
{
	/*Checking if each sensor is in the normal range.*/
	if (array[i].sensor_type == 1)
	{
		P_M_U aux = *(P_M_U *)array[i].sensor_data;
		if (aux.voltage < 10.0 || aux.voltage > 20.0)
			return 1;
		if (aux.current < -100.0 || aux.current > 100.0)
			return 1;
		if (aux.power_consumption < 0.0 || aux.power_consumption > 1000.0)
			return 1;
		if (aux.energy_regen < 0 || aux.energy_regen > 100)
			return 1;
		if (aux.energy_storage < 0 || aux.energy_storage > 100)
			return 1;
		return 0;
	}
	if (array[i].sensor_type == 0)
	{
		tire_sensor aux = *(tire_sensor *)array[i].sensor_data;
		if (aux.pressure < 19.0 || aux.pressure > 28.0)
			return 1;
		if (aux.temperature < 0.0 || aux.temperature > 120.0)
			return 1;
		if (aux.wear_level < 0 || aux.wear_level > 100)
			return 1;
		return 0;
	}
	return 0;
}

void clear_array(sensor **array, int *nr_sensors)
{
	/*Freeing the memory occupied by the data and the
	array of operations of a wrong sensor. Shifting all elements
	right of the wrong sensor to obtain the correct array.
	Reallocating then memory for the obtained correct array.*/
	int i, j;
	for (i = 0; i < *nr_sensors; i++)
	{
		if (check_sensor(*array, i) == 1)
		{
			free((*array)[i].sensor_data);
			free((*array)[i].operations_idxs);
			(*nr_sensors)--;
			for (j = i; j < *nr_sensors; j++)
				(*array)[j] = (*array)[j + 1];
			*array = realloc(*array, (*nr_sensors) * sizeof(sensor));
			i--;
		}
	}
}

void cleanup_crew(sensor **array, int nr_sensors)
{
	/*Freeing all the memory allocated inside the array's
	elements, then freeing up the array itself.*/
	int i;
	for (i = 0; i < nr_sensors; i++)
	{
		free((*array)[i].sensor_data);
		free((*array)[i].operations_idxs);
	}
	free((*array));
}

int main(int argc, char const *argv[])
{
	FILE *file = fopen(argv[1], "r");
	char buffer[20];
	int nr_sensors, i, j, index;
	fread(&nr_sensors, sizeof(int), 1, file);
	/*Variables used for reading into the main array,
	which later helped to an easier sorting*/
	int head = 0, tail = nr_sensors - 1;
	sensor *arr = malloc(nr_sensors * (sizeof(sensor)));
	get_operations(func);
	for (i = 0; i < nr_sensors; i++)
	{
		/*Reading all input into the array's elements. PMU type sensors
		are put at the front of the array, while TS ones are put the back of
		the array. This way, I can just reverse the order of the tire sensors
		and I will get the correct order for the sensors printing*/
		fread(&arr[head].sensor_type, sizeof(enum sensor_type), 1, file);
		if (arr[head].sensor_type == 1)
		{
			arr[head].sensor_data = malloc(sizeof(P_M_U));
			fread(arr[head].sensor_data, sizeof(P_M_U), 1, file);
			fread(&arr[head].nr_operations, sizeof(int), 1, file);
			arr[head].operations_idxs =
				malloc(arr[head].nr_operations * sizeof(int));
			for (j = 0; j < arr[head].nr_operations; j++)
				fread(&arr[head].operations_idxs[j], sizeof(int), 1, file);
			head++;
		}
		else
		{
			arr[tail].sensor_data = malloc(sizeof(tire_sensor));
			fread(arr[tail].sensor_data, sizeof(tire_sensor), 1, file);
			fread(&arr[tail].nr_operations, sizeof(int), 1, file);
			arr[tail].operations_idxs =
				malloc(arr[tail].nr_operations * sizeof(int));
			for (j = 0; j < arr[tail].nr_operations; j++)
				fread(&arr[tail].operations_idxs[j], sizeof(int), 1, file);
			tail--;
		}
	}
	sort(tail + 1, arr, nr_sensors);
	char aux[10];
	scanf("%[^\n]s", buffer);
	/*Taking user input consisting of print X, analyze X, clear and exit,
	which stops the program.*/
	while (strcmp("exit", buffer) != 0)
	{
		if (strstr(buffer, "print"))
		{
			strcpy(aux, buffer + 6);
			/*Turning the number from the read string into an int.*/
			index = atoi(aux);
			display(index, arr, nr_sensors);
		}
		if (strstr(buffer, "analyze"))
		{
			strcpy(aux, buffer + 8);
			index = atoi(aux);
			operation(func, index, arr, nr_sensors);
		}
		if (strstr(buffer, "clear"))
			clear_array(&arr, &nr_sensors);
		scanf("\n%[^\n]s", buffer);
	}
	cleanup_crew(&arr, nr_sensors);
	fclose(file);
	return 0;
}
