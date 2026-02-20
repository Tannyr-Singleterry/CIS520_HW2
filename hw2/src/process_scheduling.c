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
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
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
	char* badChars = "\n\t\r\v\f";
	for(int i = 0; i < 32; i++)
	{
		if(input_file[i] == '\0')
			breal;
		for(int j = 0; j < 5; j++0
			if(input_file[i] == badChars[j])
				return NULL;
	}

	FILE* file = fopen(input_file, "rb");
	if(file == NULL)
		return NULL;

	//reads the first element of the file to see the size
	uint32_t* elements; //data type might need to be "size_t" but idk
	if(!fread(elements, 4, 1, file))
		return NULL;
	//checks that there are elements to read
	if(elements == 0)
		return NULL

	//array to hold read PCBs
	dyn_array_t* PCBs = dyn_array_create(elements, sizeof(ProcessControlBlock_t), NULL);
	if(PCBs == NULL)
		return NULL;

	int ctr; //counter to keep track of what we are reading ie burst time, priority, or arrival 
	ProcessControlBlock_t* currentPCB;
	for(uint32_t i = 0; i < elements*3; i++)
	{
		uint32_t info; //stores the info read
		if(!fread(&info, 4, 1, file))
			return NULL;
		//checks where to which info is read and creates a new PCB when needed
		switch(ctr)
		{
			case 0:
				current = malloc(sizeof(ProcessControlBlock_t));
				curent->remaining_burst_time = info;
				ctr++;
				break;
			case 1:
				current->priority = info;
				ctr++;
				break;
			case 2:
				current->arrival = info;
				if(!dyn_array_push_fron(PCBs, current))
					return NULL;
				ctr = 0;
				break;
		}
	}
	return PCBs;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}
