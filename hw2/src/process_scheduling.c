#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
	// decrement the burst time of the pcb
	--process_control_block->remaining_burst_time;
}

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	// validate inputs
	if(ready_queue == NULL || result == NULL)
		return false;

	size_t num_processes = dyn_array_size(ready_queue);
	if(num_processes == 0)
		return false;

	// initialize result fields
	result->average_waiting_time    = 0.0f;
	result->average_turnaround_time = 0.0f;
	result->total_run_time          = 0;

	float total_waiting_time    = 0.0f;
	float total_turnaround_time = 0.0f;
	unsigned long current_time  = 0;

	// process each PCB in arrival order (back of queue = first arrived)
	for(size_t i = 0; i < num_processes; i++)
	{
		ProcessControlBlock_t pcb;
		if(!dyn_array_extract_back(ready_queue, &pcb))
			return false;

		// if CPU is idle before process arrives, advance time
		if(current_time < (unsigned long)pcb.arrival)
			current_time = (unsigned long)pcb.arrival;

		// waiting time = start time - arrival time
		float waiting_time = (float)(current_time - pcb.arrival);
		total_waiting_time += waiting_time;

		// run the process to completion
		while(pcb.remaining_burst_time > 0)
		{
			virtual_cpu(&pcb);
			current_time++;
		}

		// turnaround time = completion time - arrival time
		float turnaround_time = (float)(current_time - pcb.arrival);
		total_turnaround_time += turnaround_time;
	}

	result->total_run_time          = current_time;
	result->average_waiting_time    = total_waiting_time    / (float)num_processes;
	result->average_turnaround_time = total_turnaround_time / (float)num_processes;

	return true;
}

int cmp_burst_time(const void* pcb1, const void* pcb2)
{
	ProcessControlBlock_t* a = (ProcessControlBlock_t*)pcb1;
	ProcessControlBlock_t* b = (ProcessControlBlock_t*)pcb2;
	return a->remaining_burst_time - b->remaining_burst_time;

}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	if(ready_queue == NULL || result == NULL)
		return false;

	size_t n = dyn_array_size(ready_queue);
	if(n == 0)
		return false;
	if(!dyn_array_sort(ready_queue, cmp_burst_time))
		return false;

	result->average_waiting_time = 0.0f;
	result->average_turnaround_time = 0.0f;
	result->total_run_time = 0;

	float total_waiting_time = 0.0f;
	float total_turnaround_time = 0.0f;
	unsigned long current_time;

	for(size_t i = 0; i < n; i++)
	{
		ProcessControlBlock_t pcb;
		if(!dyn_array_extract_front(ready_queue, &pcb))
			return false;
		if(current_time < (unsigned long)pcb.arrival)
			current_time = (unsigned long)pcb.arrival;
		total_waiting_time += (float)(current_time - pcb.arrival);
		while(pcb.remaining_burst_time > 0)
		{
			virtual_cpu(&pcb);
			current_time++;
		}
		total_turnaround_time += (float)(current_time - pcb.arrival);
	}
	result->total_run_time = current_time;
	result->average_waiting_time = total_waiting_time/n;
	result->average_turnaround_time = total_turnaround_time/n;
	return true;
}

int cmp_priority(const void* pcb1, const void* pcb2)
{
	ProcessControlBlock_t* a = (ProcessControlBlock_t*)pcb1;
	ProcessControlBlock_t* b = (ProcessControlBlock_t*)pcb2;
	return a->priority - b->priority;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	//checks valid params
	if(ready_queue == NULL || result == NULL)
		return false;

	//clearing results
	result->average_waiting_time = 0.0f;
	result->average_turnaround_time = 0.0f;
	result->total_run_time = 0;

	//setting up tracking vars
	float total_waiting_time = 0.0f;
	float total_turnaround_time = 0.0f;
	unsigned long current_time = 0;

	size_t n = dyn_array_size(ready_queue); // number of processes

	//checking that there are processes and setting up dyn_array
	if(n == 0)
		return false;
	if(!dyn_array_sort(ready_queue, cmp_priority))
		return false;

	//processing
	for(size_t i = 0; i < n; i++)
	{
		ProcessControlBlock_t pcb;
		if(!dyn_array_extract_front(ready_queue, &pcb))
			return false;


		if(current_time < (unsigned long)pcb.arrival)
			current_time = pcb.arrival;

		total_waiting_time += (float)(current_time-pcb.arrival);


		while(pcb.remaining_burst_time > 0)
		{
			virtual_cpu(&pcb);
			current_time++;
		}

		total_turnaround_time += (float)(current_time-pcb.arrival);
	}

	result->total_run_time = current_time;
	result->average_waiting_time = total_waiting_time/n;
	result->average_turnaround_time = total_turnaround_time/n;
	return true;
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
	if(ready_queue == NULL || result == NULL || quantum == 0)
		return false;

	size_t n = dyn_array_size(ready_queue);
	if(n == 0)
		return false;

	result->average_waiting_time    = 0.0f;
	result->average_turnaround_time = 0.0f;
	result->total_run_time          = 0;

	float total_waiting_time    = 0.0f;
	float total_turnaround_time = 0.0f;
	unsigned long current_time  = 0;

	// copy all PCBs into a local array so we can requeue them
	ProcessControlBlock_t* processes = malloc(n * sizeof(ProcessControlBlock_t));
	if(processes == NULL)
		return false;

	// extract all PCBs preserving arrival order (back is first arrived)
	for(size_t i = 0; i < n; i++)
	{
		if(!dyn_array_extract_back(ready_queue, &processes[i]))
		{
			free(processes);
			return false;
		}
	}

	size_t finished = 0;

	while(finished < n)
	{
		bool any_ran = false;

		for(size_t i = 0; i < n; i++)
		{
			// skip already finished processes
			if(processes[i].remaining_burst_time == 0)
				continue;

			// skip processes that haven't arrived yet
			if((unsigned long)processes[i].arrival > current_time)
				continue;

			any_ran = true;

			// mark start time for waiting time calculation
			if(!processes[i].started)
			{
				processes[i].started = true;
				total_waiting_time += (float)(current_time - processes[i].arrival);
			}

			size_t ticks = 0;
			while(processes[i].remaining_burst_time > 0 && ticks < quantum)
			{
				virtual_cpu(&processes[i]);
				current_time++;
				ticks++;
			}

			// if process finished, record turnaround time
			if(processes[i].remaining_burst_time == 0)
			{
				total_turnaround_time += (float)(current_time - processes[i].arrival);
				finished++;
			}
		}

		// if nothing ran this cycle, advance time to next arriving process
		if(!any_ran)
		{
			unsigned long next_arrival = (unsigned long)-1;
			for(size_t i = 0; i < n; i++)
			{
				if(processes[i].remaining_burst_time > 0 &&
				   (unsigned long)processes[i].arrival < next_arrival)
				{
					next_arrival = (unsigned long)processes[i].arrival;
				}
			}
			if(next_arrival != (unsigned long)-1)
				current_time = next_arrival;
		}
	}

	free(processes);

	result->total_run_time          = current_time;
	result->average_waiting_time    = total_waiting_time    / (float)n;
	result->average_turnaround_time = total_turnaround_time / (float)n;
	return true;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
	// checks for valid input file
	if(input_file == NULL)
		return NULL;
	const char* badChars = "\n\t\r\v\f";
	for(int i = 0; i < 32; i++)
	{
		if(input_file[i] == '\0')
			break;
		for(int j = 0; j < 5; j++)
		{
			if(input_file[i] == badChars[j])
				return NULL;
		}
	}

	FILE* file = fopen(input_file, "rb");
	if(file == NULL)
		return NULL;

	// reads the first element of the file to see the size
	uint32_t elements = 0;
	if(!fread(&elements, 4, 1, file))
	{
		fclose(file);
		return NULL;
	}

	// checks that there are elements to read
	if(elements == 0)
	{
		fclose(file);
		return NULL;
	}

	// array to hold read PCBs
	dyn_array_t* PCBs = dyn_array_create(elements, sizeof(ProcessControlBlock_t), NULL);
	if(PCBs == NULL)
	{
		fclose(file);
		return NULL;
	}

	int ctr = 0; // counter to keep track of what we are reading ie burst time, priority, or arrival
	ProcessControlBlock_t* currentPCB = NULL;
	for(uint32_t i = 0; i < elements * 3; i++)
	{
		uint32_t info; // stores the info read
		if(!fread(&info, 4, 1, file))
		{
			free(currentPCB);
			dyn_array_destroy(PCBs);
			fclose(file);
			return NULL;
		}
		// checks which info is read and creates a new PCB when needed
		switch(ctr)
		{
			case 0:
				currentPCB = malloc(sizeof(ProcessControlBlock_t));
				currentPCB->remaining_burst_time = info;
				ctr++;
				break;
			case 1:
				currentPCB->priority = info;
				ctr++;
				break;
			case 2:
				currentPCB->arrival = info;
				currentPCB->started = false;
				if(!dyn_array_push_front(PCBs, currentPCB))
				{
					free(currentPCB);
					dyn_array_destroy(PCBs);
					fclose(file);
					return NULL;
				}
				free(currentPCB);
				currentPCB = NULL;
				ctr = 0;
				break;
		}
	}

	fclose(file);
	return PCBs;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	if(ready_queue == NULL || result == NULL)
		return false;

	size_t n = dyn_array_size(ready_queue);
	if(n == 0)
		return false;

	result->average_waiting_time    = 0.0f;
	result->average_turnaround_time = 0.0f;
	result->total_run_time          = 0;

	float total_waiting_time    = 0.0f;
	float total_turnaround_time = 0.0f;
	unsigned long current_time  = 0;

	// copy all PCBs into a local array
	ProcessControlBlock_t* processes = malloc(n * sizeof(ProcessControlBlock_t));
	if(processes == NULL)
		return false;

	// extract all PCBs in arrival order
	for(size_t i = 0; i < n; i++)
	{
		if(!dyn_array_extract_back(ready_queue, &processes[i]))
		{
			free(processes);
			return false;
		}
		processes[i].started = false;
	}

	size_t finished = 0;

	while(finished < n)
	{
		// find the process with the shortest remaining burst time that has arrived
		int shortest_idx = -1;
		for(size_t i = 0; i < n; i++)
		{
			if(processes[i].remaining_burst_time == 0)
				continue;
			if((unsigned long)processes[i].arrival > current_time)
				continue;
			if(shortest_idx == -1 ||
			   processes[i].remaining_burst_time < processes[shortest_idx].remaining_burst_time)
			{
				shortest_idx = (int)i;
			}
		}

		// advance time to next arrival if not process
		if(shortest_idx == -1)
		{
			unsigned long next_arrival = (unsigned long)-1;
			for(size_t i = 0; i < n; i++)
			{
				if(processes[i].remaining_burst_time > 0 &&
				   (unsigned long)processes[i].arrival < next_arrival)
				{
					next_arrival = (unsigned long)processes[i].arrival;
				}
			}
			if(next_arrival != (unsigned long)-1)
				current_time = next_arrival;
			continue;
		}

		if(!processes[shortest_idx].started)
		{
			processes[shortest_idx].started = true;
			total_waiting_time += (float)(current_time - processes[shortest_idx].arrival);
		}

		virtual_cpu(&processes[shortest_idx]);
		current_time++;

		// if process finished, record turnaround time
		if(processes[shortest_idx].remaining_burst_time == 0)
		{
			total_turnaround_time += (float)(current_time - processes[shortest_idx].arrival);
			finished++;
		}
	}

	free(processes);

	result->total_run_time          = current_time;
	result->average_waiting_time    = total_waiting_time    / (float)n;
	result->average_turnaround_time = total_turnaround_time / (float)n;
	return true;
}
