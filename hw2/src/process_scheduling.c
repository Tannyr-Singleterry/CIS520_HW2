#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"


// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
	// decrement the burst time of the pcb
	--process_control_block->remaining_burst_time;
}

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	if(ready_queue == NULL || result == NULL)
		return false;

	size_t num_processes = dyn_array_size(ready_queue);
	if(num_processes == 0)
		return false;

	result->average_waiting_time = 0.0f;
	result->average_turnaround_time - 0.0f;
	result->total_run_time = 0;

	float total_waiting_time = 0.0f;
	float total_turnaround_time = 0.0f;
	unsigned long current_time = 0;

	//Process each PCB in order of when it arrived. So back of queue is first arrived.
	for(size_t i = 0; i < num_processes; i++){
		ProcessControlBlock_t pcb;
		if(!dyn_array_extract_back(ready_queue, &pcb))
			return false;

		//if idle, advance.
		if(current_time < (unsigned long)pcb.arrival)
			current_time = (unsigned)pcb.arrival;

		float waiting_time = (float)(current_time - pcb.arrival);
		total_waiting_time += waiting_time;

		while(pcb.remaining_burst_time > 0){
			virtual_cpu(&pcb);
			current_time++;
		}

		float turnaround_time = (float)(current_time - pcb.arrival);
		total_turnaround_time += turnaround_time;
		
	}

	result->total_run_time = current_time;
	result->average_waiting_time = total_waiting_time / (float)num_processes;
	result->average_turnaround_time = total_turnaround_time / (float)num_processes;

	return true;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	UNUSED(quantum);
	return false;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
	//checks for valid input file
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

	//reads the first element of the file to see the size
	uint32_t elements = 0; //data type might need to be "size_t" but idk
	if(!fread(elements, 4, 1, file))
	{
		fclose(file);
		return NULL;
	}	

	//checks that there are elements to read
	if(elements == 0)
	{
		fclose(file);
		return NULL
	}

	//array to hold read PCBs
	dyn_array_t* PCBs = dyn_array_create(elements, sizeof(ProcessControlBlock_t), NULL);
	if(PCBs == NULL)
	{
		fclose(file);
		return NULL;
	}

	int ctr = 0; //counter to keep track of what we are reading ie burst time, priority, or arrival 
	ProcessControlBlock_t* currentPCB;
	for(uint32_t i = 0; i < elements*3; i++)
	{
		uint32_t info; //stores the info read
		if(!fread(&info, 4, 1, file))
		{
			free(currentPCB);
			dyn_array_destroy(PCBs);
			fclose(file);
			return NULL;
		}
		//checks where to which info is read and creates a new PCB when needed
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
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}
