#include <stdio.h>
#include <stdlib.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS"
#define P "P"
#define RR "RR"
#define SJF "SJF"
#define SRT "SRT"

// Add and comment your analysis code in this function.
int main(int argc, char **argv) 
{
	if (argc < 3) 
	{
		printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
		return EXIT_FAILURE;
	}

	const char* pcb_file  = argv[1];
	const char* algorithm = argv[2];

	// load the process control blocks from the binary file
	dyn_array_t* ready_queue = load_process_control_blocks(pcb_file);
	if(ready_queue == NULL)
	{
		fprintf(stderr, "Error: failed to load PCBs from file '%s'\n", pcb_file);
		return EXIT_FAILURE;
	}

	ScheduleResult_t result;
	result.average_waiting_time    = 0.0f;
	result.average_turnaround_time = 0.0f;
	result.total_run_time          = 0;

	bool success = false;

	char algo_buf[8];
	if(sscanf(algorithm, "%7s", algo_buf) != 1)
	{
		fprintf(stderr, "Error: could not read algorithm argument.\n");
		dyn_array_destroy(ready_queue);
		return EXIT_FAILURE;
	}

	if(sscanf(algo_buf, FCFS) == 0 && algo_buf[0] == 'F')
	{
		success = first_come_first_serve(ready_queue, &result);
	}
	else if(algo_buf[0] == 'S' && algo_buf[1] == 'J')
	{
		success = shortest_job_first(ready_queue, &result);
	}
	else if(algo_buf[0] == 'P' && algo_buf[1] == '\0')
	{
		success = priority(ready_queue, &result);
	}
	else if(algo_buf[0] == 'R' && algo_buf[1] == 'R')
	{
		if(argc < 4)
		{
			fprintf(stderr, "Error: Round Robin requires a quantum value.\n");
			fprintf(stderr, "Usage: %s <pcb file> RR <quantum>\n", argv[0]);
			dyn_array_destroy(ready_queue);
			return EXIT_FAILURE;
		}

		size_t quantum = 0;
		if(sscanf(argv[3], "%zu", &quantum) != 1 || quantum == 0)
		{
			fprintf(stderr, "Error: quantum must be a positive integer.\n");
			dyn_array_destroy(ready_queue);
			return EXIT_FAILURE;
		}

		success = round_robin(ready_queue, &result, quantum);
	}
	else if(algo_buf[0] == 'S' && algo_buf[1] == 'R')
	{
		success = shortest_remaining_time_first(ready_queue, &result);
	}
	else
	{
		fprintf(stderr, "Error: unknown algorithm '%s'\n", algorithm);
		fprintf(stderr, "Valid options: FCFS, SJF, P, RR, SRT\n");
		dyn_array_destroy(ready_queue);
		return EXIT_FAILURE;
	}

	dyn_array_destroy(ready_queue);

	if(!success)
	{
		fprintf(stderr, "Error: scheduling algorithm '%s' failed.\n", algorithm);
		return EXIT_FAILURE;
	}

	printf("Algorithm: %s\n",  algorithm);
	printf("Average Waiting Time: %.2f\n", result.average_waiting_time);
	printf("Average Turnaround Time: %.2f\n", result.average_turnaround_time);
	printf("Total Run Time: %lu\n",  result.total_run_time);

	return EXIT_SUCCESS;
}
