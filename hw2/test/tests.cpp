#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include "gtest/gtest.h"
#include "../include/processing_scheduling.h"

// Using a C library requires extern "C" to prevent function mangling
extern "C"
{
#include <dyn_array.h>
}

#define NUM_PCB 30
#define QUANTUM 5 // Used for Robin Round for process as the run time limit


/*
 Helper function
*/
ProcessControlBlock_t make_pcb(uint32_t arrival, uint32_t burst) {
    ProcessControlBlock_t pcb;
    pcb.arrival = arrival;
    pcb.remaining_burst_time = burst;
    return pcb;
}

/*
Test 1:
Basic FCFS ordering with two processes arriving at time 0
*/
TEST(FCFS_Test, TwoProcessesBasicOrder) {
    dyn_array_t* queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), nullptr);
    ScheduleResult_t result;

    ProcessControlBlock_t second = make_pcb(0, 3); // runs second
    ProcessControlBlock_t first  = make_pcb(0, 4); // runs first

    dyn_array_push_back(queue, &second);
    dyn_array_push_back(queue, &first);

    ASSERT_TRUE(first_come_first_serve(queue, &result));

    //Waiting times:
    //first = 0
    //second = 4
    //avg = 2.0
    EXPECT_FLOAT_EQ(result.average_waiting_time, 2.0f);

    //Turnaround:
    //first = 4
    //second = 7
    //avg = 5.5
    EXPECT_FLOAT_EQ(result.average_turnaround_time, 5.5f);

    EXPECT_EQ(result.total_run_time, 7UL);

    dyn_array_destroy(queue);
}

/*
Test 2:
Idle time before first process arrives
*/
TEST(FCFS_Test, IdleBeforeStart) {
    dyn_array_t* queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), nullptr);
    ScheduleResult_t result;

    //Arrives at time 5, burst 3
    ProcessControlBlock_t pcb = make_pcb(5, 3);
    dyn_array_push_back(queue, &pcb);

    ASSERT_TRUE(first_come_first_serve(queue, &result));

    //CPU idles until time 5
    EXPECT_FLOAT_EQ(result.average_waiting_time, 0.0f);
    EXPECT_FLOAT_EQ(result.average_turnaround_time, 3.0f);

    //CPU finishes at time 8
    EXPECT_EQ(result.total_run_time, 8UL);

    dyn_array_destroy(queue);
}

/*
Test 3:
Null filename returns NULL
*/
TEST(LoadPCB_Test, NullInputFilename)
{
    const char* input_filename = NULL;
    dyn_array_t* result = load_process_control_blocks(input_filename);
    EXPECT_EQ(result, (dyn_array_t*)NULL);
}

/*
Test 4:
Valid binary file loads correct number of PCBs and correct field values
*/
TEST(LoadPCB_Test, ValidFileLoadsCorrectly)
{
    const char* input_filename = "/tmp/test_valid_pcb.bin";

    // write a valid PCB binary file with 2 entries
    // format: [uint32_t count] then count * [burst, priority, arrival]
    FILE* f = fopen(input_filename, "wb");
    ASSERT_NE(f, (FILE*)NULL);

    uint32_t count    = 2;
    uint32_t burst0   = 5, prio0 = 1, arrival0 = 0;
    uint32_t burst1   = 3, prio1 = 2, arrival1 = 1;

    fwrite(&count,    sizeof(uint32_t), 1, f);
    fwrite(&burst0,   sizeof(uint32_t), 1, f);
    fwrite(&prio0,    sizeof(uint32_t), 1, f);
    fwrite(&arrival0, sizeof(uint32_t), 1, f);
    fwrite(&burst1,   sizeof(uint32_t), 1, f);
    fwrite(&prio1,    sizeof(uint32_t), 1, f);
    fwrite(&arrival1, sizeof(uint32_t), 1, f);
    fclose(f);

    dyn_array_t* pcbs = load_process_control_blocks(input_filename);
    ASSERT_NE(pcbs, (dyn_array_t*)NULL);

    // should have loaded exactly 2 PCBs
    EXPECT_EQ(dyn_array_size(pcbs), (size_t)2);

    ProcessControlBlock_t pcb;

    // first entry
    dyn_array_extract_back(pcbs, &pcb);
    EXPECT_EQ(pcb.remaining_burst_time, (uint32_t)5);
    EXPECT_EQ(pcb.priority,             (uint32_t)1);
    EXPECT_EQ(pcb.arrival,              (uint32_t)0);

    // second entry
    dyn_array_extract_back(pcbs, &pcb);
    EXPECT_EQ(pcb.remaining_burst_time, (uint32_t)3);
    EXPECT_EQ(pcb.priority,             (uint32_t)2);
    EXPECT_EQ(pcb.arrival,              (uint32_t)1);

    dyn_array_destroy(pcbs);
    remove(input_filename);
}
/*
unsigned int score;
unsigned int total;

class GradeEnvironment : public testing::Environment
{
	public:
		virtual void SetUp()
		{
			score = 0;
			total = 210;
		}

		virtual void TearDown()
		{
			::testing::Test::RecordProperty("points_given", score);
			::testing::Test::RecordProperty("points_total", total);
			std::cout << "SCORE: " << score << '/' << total << std::endl;
		}
};
*/


int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	// ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
	return RUN_ALL_TESTS();
}
