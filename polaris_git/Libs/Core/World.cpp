///----------------------------------------------------------------------------------------------------
/// World.cpp - Container for the Discrete Event Engine and Memory Allocator
///----------------------------------------------------------------------------------------------------

#include "Simulation_Thread.h"

namespace polaris
{

World* _world = nullptr;



///----------------------------------------------------------------------------------------------------
/// Initialize - Initialize Discrete Event Engine and Memory Allocator
///----------------------------------------------------------------------------------------------------

void World::Initialize(Simulation_Configuration& cfg)
{
	// check for correct user input and set up world

	cfg.Check_Configuration();

	_num_iterations = cfg._num_iterations;
	_num_sim_threads = cfg._num_sim_threads;
	_num_antares_threads = _num_sim_threads + 1;
	_preallocation_bytes = cfg._preallocation_bytes;
	_execution_segments_per_thread = cfg._execution_segments_per_thread;
	_execution_objects_per_block = cfg._execution_objects_per_block;
	_max_execution_objects_per_block = cfg._max_execution_objects_per_block;
	_max_free_blocks = cfg._max_free_blocks;
	_num_free_blocks_buffer = cfg._num_free_blocks_buffer;

	// build the simulation engine
	_simulation_engine = new Simulation_Engine();
	_simulation_engine->Initialize();

	// simulation not running yet
	_running = false;
	// no threads have work yet
	_threads_idle_counter=0;
	// no threads ready
	_threads_ready_counter=0;

	// "main" thread is always __num_sim_threads
	__thread_id = _num_sim_threads;

	// set up iteration to default level
	_revision._iteration = 0;
	_revision._sub_iteration = 0;

	// set up conditionals and threads by platform
	#ifdef _MSC_VER
		_threads_finished_event = CreateEvent(NULL,FALSE,FALSE,NULL);
		_threads_start_event = CreateEvent(NULL,TRUE,FALSE,NULL);
		_threads = new void*[_num_sim_threads];
	#else
		pthread_mutex_init(&_threads_finished_mutex, NULL);
		pthread_cond_init(&_threads_finished_conditional, NULL);
		pthread_mutex_init(&_threads_start_mutex, NULL);
		pthread_cond_init(&_threads_start_conditional, NULL);

		// lock this before threads get going, to ensure they don't finish improperly
		pthread_mutex_lock(&_threads_finished_mutex);
	
		_threads = new pthread_t[__num_sim_threads];
	#endif

	Thread_Information* thread_information = new Thread_Information[_num_sim_threads];

	// set up threads by platform
	for(unsigned int i=0;i < _num_sim_threads; i++)
	{
		thread_information[i]._id = i;
		thread_information[i]._world_ptr = this;

		#ifdef _MSC_VER
			_threads[i]=CreateThread(NULL,0,Simulation_Loop,&thread_information[i],0,NULL);
		#else
			pthread_create(&_threads[i],NULL,Simulation_Loop,&thread_information[i]);
		#endif
	}

	// wait for all threads to give the signal that they are at the ready
	Wait_For_Signal_From_Threads();
}

///----------------------------------------------------------------------------------------------------
/// Terminate - Terminate the world
///----------------------------------------------------------------------------------------------------

void World::Terminate()
{
	delete _simulation_engine;
	delete[] _threads;
}

///----------------------------------------------------------------------------------------------------
/// Send_Signal_To_World - Threads indicate change of execution region to World
///----------------------------------------------------------------------------------------------------

void World::Send_Signal_To_World()
{
	#ifdef _MSC_VER
		// let the world know that this thread is waiting
		if(Mark_Thread_As_Idle() == _num_sim_threads)
		{
			// when all threads are finished, signal the engine
			SetEvent(_threads_finished_event);
		}
	#else
		pthread_mutex_lock(&_threads_start_mutex);

		// let the world know that this thread is waiting
		if(Mark_Thread_As_Idle() == __num_sim_threads)
		{
			// must toggle mutex in order to get conditional functionality
			pthread_mutex_lock(&_threads_finished_mutex);
			pthread_cond_signal(&_threads_finished_conditional);
			pthread_mutex_unlock(&_threads_finished_mutex);
			// world has threads_finished_mutex at this point
		}
	#endif
}

///----------------------------------------------------------------------------------------------------
/// Send_Finished_Signal_To_World - Threads indicate they are leaving the simulation
///----------------------------------------------------------------------------------------------------

void World::Send_Finished_Signal_To_World()
{
	#ifdef _MSC_VER
		// let the world know that this thread is waiting
		if(Mark_Thread_As_Idle() == _num_sim_threads)
		{
			// when all threads are finished, signal the engine
			SetEvent(_threads_finished_event);
		}
	#else
		// let the world know that this thread is waiting
		if(Mark_Thread_As_Idle() == __num_sim_threads)
		{
			// must toggle mutex in order to get conditional functionality
			pthread_mutex_lock(&_threads_finished_mutex);
			pthread_cond_signal(&_threads_finished_conditional);
			pthread_mutex_unlock(&_threads_finished_mutex);
			// world has threads_finished_mutex at this point
		}
	#endif
}

///----------------------------------------------------------------------------------------------------
/// Wait_For_Signal_From_World - Threads wait for world to release them 
///----------------------------------------------------------------------------------------------------

void World::Wait_For_Signal_From_World()
{
	#ifdef _MSC_VER
		// wait for the engine to signal you
		WaitForSingleObject(_threads_start_event,INFINITE);
	#else
		// world will broadcast when ready to move on
		pthread_cond_wait(&_threads_start_conditional,&_threads_start_mutex);
		// mutex automatically locked, but since many threads are being released, must unlock for each
		pthread_mutex_unlock(&_threads_start_mutex);
	#endif

	// thread indicates that it has left the idle queue
	Mark_Thread_As_Ready();

	// thread waits until all threads have left the idle queue to begin actual operation
	Spin_Until_All_Threads_Ready();

	#ifdef _MSC_VER
		// signal engine that threads have begun
		ResetEvent(_threads_start_event);
	#else
	#endif
}

///----------------------------------------------------------------------------------------------------
/// Send_Signal_To_Threads - World indicate change of execution region to threads
///----------------------------------------------------------------------------------------------------

void World::Send_Signal_To_Threads()
{
	// threads are being released, make a note of it
	_threads_idle_counter=0;

	#ifdef _MSC_VER
		SetEvent(_threads_start_event);
	#else
		// must toggle start lock in order to signal properly
		pthread_mutex_lock(&_threads_start_mutex);
		pthread_cond_broadcast(&_threads_start_conditional);
		pthread_mutex_unlock(&_threads_start_mutex);
	#endif
}

///----------------------------------------------------------------------------------------------------
/// Wait_For_Signal_From_Threads - World waits for threads to release it 
///----------------------------------------------------------------------------------------------------

void World::Wait_For_Signal_From_Threads()
{
	#ifdef _MSC_VER
		// wait for the threads to signal you
		WaitForSingleObject(_threads_finished_event,INFINITE);
	#else
		pthread_cond_wait(&_threads_finished_conditional,&_threads_finished_mutex);
	#endif

	// can safely say no threads are running at this point, make a note of it
	_threads_ready_counter=0;
}

///----------------------------------------------------------------------------------------------------
/// Start_Turning - World begins the simulation execution
///----------------------------------------------------------------------------------------------------

void World::Start_Turning()
{
	if(_running)
	{
		THROW_EXCEPTION("Simulation Has Already Started!");
	}

	_running = true;
	
	// move to the first iteration (0,0)

	_simulation_engine->Update();
	
	// loose threads on the first iteration of the simulation

	Send_Signal_To_Threads();
	
	Simulation_Engine* const simulation_engine = _simulation_engine;

	while(true)
	{
		// wait until all threads have entered the finished queue

		Wait_For_Signal_From_Threads();

		// send the world thread to the simulation engine to perform the update step

		simulation_engine->Update();

		// determine whether to continue or not

		if( iteration() < (int)_num_iterations )
		{
			// continue simulation
			Send_Signal_To_Threads();
		}
		else
		{
			// end simulation
			_running = false;

			// instruct threads to leave the finished queue (and the simulation)
			Send_Signal_To_Threads();
			
			// let all threads terminate before exiting
			Wait_For_Signal_From_Threads();

			break;
		}
	}
}


}