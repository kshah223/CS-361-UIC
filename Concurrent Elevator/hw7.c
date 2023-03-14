#include"elevator.h"
#include <stdio.h>
#include <pthread.h>


struct Elevator {
    enum {
        ELEVATOR_ARRIVED, ELEVATOR_OPEN, ELEVATOR_CLOSED
    } state;
    int curr;
    int from_floor;
    int to_floor;
    int dir;
    int count;
    pthread_mutex_t   e_lock;
    pthread_mutex_t   p_lock;
    pthread_barrier_t e_barrier;
    pthread_barrier_t p_barrier;
} elevators[ELEVATORS];

void scheduler_init() {
    int i = 0;
    while(i < ELEVATORS) {
        elevators[i].state = ELEVATOR_ARRIVED;
        elevators[i].curr = 0;
        elevators[i].from_floor = -1;
        elevators[i].to_floor = -1;
        elevators[i].dir = -1;
        elevators[i].count = 0;
        pthread_mutex_init(&elevators[i].e_lock, 0);
        pthread_mutex_init(&elevators[i].p_lock, 0);
        pthread_barrier_init(&elevators[i].e_barrier, 0, 2);
        pthread_barrier_init(&elevators[i].p_barrier, 0, 2);
		i++;
    }
}

void passenger_request(int passenger, int from_floor, int to_floor,
                       void (*enter)(int, int), void (*exit)(int, int)) {
    int p  = passenger;
    int e  = p % ELEVATORS;

    pthread_mutex_lock(&elevators[e].p_lock);

    elevators[e].from_floor = from_floor;
    elevators[e].to_floor   = to_floor;

    pthread_barrier_wait(&elevators[e].p_barrier);

    if (elevators[e].state == ELEVATOR_OPEN && elevators[e].curr == from_floor && elevators[e].count == 0) {
        enter(p, e);
        elevators[e].count++;
    }

    pthread_barrier_wait(&elevators[e].e_barrier);
    pthread_barrier_wait(&elevators[e].p_barrier);

    if (elevators[e].state == ELEVATOR_OPEN && elevators[e].curr == to_floor && elevators[e].count == 1) {
        exit(p, e);
        elevators[e].from_floor = -1;
        elevators[e].to_floor   = -1;
        elevators[e].count--;
    }

    pthread_barrier_wait(&elevators[e].e_barrier);
    pthread_mutex_unlock(&elevators[e].p_lock);
}

void elevator_ready(int elevator, int at_floor, void (*move_dir)(int, int),
                    void (*door_open)(int), void (*door_close)(int)) {
    
	int e = elevator;
    pthread_mutex_lock(&elevators[e].e_lock);
	
    int from_floor  = elevators[e].from_floor;
    int to_floor  = elevators[e].to_floor;
    int dir = elevators[e].dir;
    int count = elevators[e].count;

    if (elevators[e].state == ELEVATOR_ARRIVED) {
        if ((count == 0 && at_floor == from_floor) || (count == 1 && at_floor == to_floor)) {
            door_open(e);
            elevators[e].state = ELEVATOR_OPEN;
            pthread_barrier_wait(&elevators[e].p_barrier);
            pthread_barrier_wait(&elevators[e].e_barrier);
        }
        else
            elevators[e].state = ELEVATOR_CLOSED;
    }

    else if (elevators[e].state == ELEVATOR_OPEN) {
        door_close(e);
        elevators[e].state = ELEVATOR_CLOSED;
    }

    else {
        if ((at_floor  == 0) || (at_floor == FLOORS - 1) || (dir == 1  && count == 0 && from_floor < at_floor) || (dir == 1  && count == 1 && to_floor < at_floor) ||
           (dir == -1 && count == 0 && at_floor < from_floor) || (dir == -1 && count == 1 && at_floor < to_floor))
				elevators[e].dir *= -1;

        move_dir(e, elevators[e].dir);
        elevators[e].curr = at_floor + elevators[e].dir;
        elevators[e].state = ELEVATOR_ARRIVED;
    }

    pthread_mutex_unlock(&elevators[e].e_lock);
}