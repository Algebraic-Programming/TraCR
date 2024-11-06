#include "common/instr.h"

/*
* this little example runs a thread from all existing states:
unknown -(execute)-> running -(cool)-> cooling -(pause)-> paused
-(warm)-> warming -(resume)-> running -(end)-> dead    
*/
int
main(void)
{
	const int rank = 0; 
	const int nranks = 1;

	printf("Hello before OHx\n");

	// create&execute thread
	instr_start(rank, nranks);

	printf("Hello before OHc\n");

	// enter cooling phase
	instr_thread_enter_cooling();

	printf("Hello before OHp\n");

	// pause cooling and enter paused state
	instr_thread_pause();

	printf("Hello before OHw\n");

	// from pause go to warming state
	instr_thread_enter_warming();

	printf("Hello before OHr\n");

	// from warming state resume execution
	instr_thread_resume();

	printf("Hello before OHe\n");

	// end execution (dead state)
	instr_end();

	printf("Hello after OHe\n");

	return 0;
}
