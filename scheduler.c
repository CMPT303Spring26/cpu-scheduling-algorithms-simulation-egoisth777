#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 100

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int waiting_time;
    int turnaround_time;
    int finish_time;
    int is_completed;
} Process;

// Helper to reset process state between different simulation runs
void reset_processes(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].burst_time;
        p[i].waiting_time = 0;
        p[i].turnaround_time = 0;
        p[i].finish_time = 0;
        p[i].is_completed = 0;
    }
}

void print_stats(Process p[], int n, char* policy_name) {
    float avg_wait = 0, avg_tat = 0;
    printf("\n================ %s ================\n", policy_name);
    printf("PID\tArr\tBst\tFin\tWait\tTAT\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t%d\t%d\n", 
               p[i].pid, p[i].arrival_time, p[i].burst_time, 
               p[i].finish_time, p[i].waiting_time, p[i].turnaround_time);
        avg_wait += p[i].waiting_time;
        avg_tat += p[i].turnaround_time;
    }
    printf("--------------------------------------------\n");
    printf("Average Waiting Time: %.2f\n", avg_wait / n);
    printf("Average Turnaround Time: %.2f\n\n", avg_tat / n);
}

// Function Prototypes for students to implement
// 1. First-Come, First-Served (FCFS)
void simulate_FCFS(Process p[], int n) {
    int current_time = 0;
    // Note: This assumes input is sorted by arrival time
    for (int i = 0; i < n; i++) {
        if (current_time < p[i].arrival_time) {
            current_time = p[i].arrival_time;
        }
        p[i].waiting_time = current_time - p[i].arrival_time;
        current_time += p[i].burst_time;
        p[i].finish_time = current_time;
        p[i].turnaround_time = p[i].finish_time - p[i].arrival_time;
    }
    print_stats(p, n, "FCFS");
}

// 2. Shortest Job First (SJF) - Non-Preemptive
void simulate_SJF(Process p[], int n) {
    int current_time = 0, completed = 0;
    
    while (completed < n) {
        int idx = -1;
        int min_burst = 99999;

        // Find the process with the shortest burst among those that have arrived
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && !p[i].is_completed) {
                if (p[i].burst_time < min_burst) {
                    min_burst = p[i].burst_time;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            p[idx].waiting_time = current_time - p[idx].arrival_time;
            current_time += p[idx].burst_time;
            p[idx].finish_time = current_time;
            p[idx].turnaround_time = p[idx].finish_time - p[idx].arrival_time;
            p[idx].is_completed = 1;
            completed++;
        } else {
            current_time++; // IDLE
        }
    }
    print_stats(p, n, "SJF (Non-Preemptive)");
}

// 3. Round Robin (RR)
void simulate_RR(Process p[], int n, int quantum) {
    int current_time = 0, completed = 0;
    int queue[MAX_PROCESSES], head = 0, tail = 0;
    int in_queue[MAX_PROCESSES] = {0};

    // Initial check for processes arriving at time 0
    for(int i=0; i<n; i++) {
        if(p[i].arrival_time <= current_time) {
            queue[tail++] = i;
            in_queue[i] = 1;
        }
    }

    while (completed < n) {
		if (head == tail) { // Queue empty but not all done
	    	current_time++;
	    	for(int i=0; i<n; i++) {
				if(p[i].arrival_time <= current_time && !in_queue[i] && !p[i].is_completed) {
		    		queue[tail++] = i;
		    		in_queue[i] = 1;
				}
	    	}
	    	continue;
		}

        int idx = queue[head++];
        int slice = (p[idx].remaining_time > quantum) ? quantum : p[idx].remaining_time;
        
        // Execute for the duration of the slice
        for(int t=0; t<slice; t++) {
            current_time++;
            // Check for new arrivals DURING this time slice
            for(int i=0; i<n; i++) {
                if(p[i].arrival_time == current_time && !in_queue[i]) {
                    queue[tail++] = i;
                    in_queue[i] = 1;
                }
            }
        }
        
        p[idx].remaining_time -= slice;

        if (p[idx].remaining_time == 0) {
            p[idx].finish_time = current_time;
            p[idx].turnaround_time = p[idx].finish_time - p[idx].arrival_time;
            p[idx].waiting_time = p[idx].turnaround_time - p[idx].burst_time;
            p[idx].is_completed = 1;
            completed++;
        } else {
            // Add back to end of queue
            queue[tail++] = idx;
        }
    }
    print_stats(p, n, "Round Robin");
}

// 4. Modified Round Robin with Context Switch Penalty
void simulate_RR_with_penalty(Process p[], int n, int quantum, int penalty) {
    int current_time = 0, completed = 0;
    int queue[MAX_PROCESSES], head = 0, tail = 0;
    int in_queue[MAX_PROCESSES] = {0};
    int last_pid = -1; // Track the previous process to detect a switch

    // Initial check for arrivals at time 0
    for(int i=0; i<n; i++) {
        if(p[i].arrival_time <= current_time) {
            queue[tail++] = i;
            in_queue[i] = 1;
        }
    }

    while (completed < n) {
        if (head == tail) { 
            current_time++;
            // (Arrival check logic remains the same as before...)
            continue;
        }

        int idx = queue[head++];

        // --- THE REALISTIC ADDITION: Context Switch Penalty ---
        // If the new process is different from the last one, the CPU is "busy" switching
        if (last_pid != -1 && p[idx].pid != last_pid) {
            current_time += penalty; 
            // New arrivals might happen DURING the context switch
            for(int i=0; i<n; i++) {
                if(p[i].arrival_time <= current_time && !in_queue[i] && !p[i].is_completed) {
                    queue[tail++] = i;
                    in_queue[i] = 1;
                }
            }
        }
        last_pid = p[idx].pid;
        // ------------------------------------------------------

        int slice = (p[idx].remaining_time > quantum) ? quantum : p[idx].remaining_time;
        
        for(int t=0; t<slice; t++) {
            current_time++;
            // Check for arrivals during execution
            for(int i=0; i<n; i++) {
                if(p[i].arrival_time == current_time && !in_queue[i]) {
                    queue[tail++] = i;
                    in_queue[i] = 1;
                }
            }
        }
        
        p[idx].remaining_time -= slice;

        if (p[idx].remaining_time == 0) {
            p[idx].finish_time = current_time;
            p[idx].turnaround_time = p[idx].finish_time - p[idx].arrival_time;
            p[idx].waiting_time = p[idx].turnaround_time - p[idx].burst_time;
            p[idx].is_completed = 1;
            completed++;
            // Note: In some models, a penalty also applies when a process finishes
        } else {
            queue[tail++] = idx;
        }
    }
    print_stats(p, n, "Round Robin (With Penalty)");
}

int main() {
    Process proc[MAX_PROCESSES];
    int n = 10;
    
    // Hardcoded workload based on the text file provided earlier
    int arrivals[] = {0, 2, 4, 5, 8, 10, 12, 15, 18, 20};
    int bursts[]   = {20, 2, 1, 5, 3, 1, 4, 10, 2, 8};
    
    for (int i = 0; i < n; i++) {
        proc[i].pid = i + 1;
        proc[i].arrival_time = arrivals[i];
        proc[i].burst_time = bursts[i];
    }

    reset_processes(proc, n);
    simulate_FCFS(proc, n);

    reset_processes(proc, n);
    simulate_SJF(proc, n);

    reset_processes(proc, n);
    //simulate_RR(proc, n, 4); // Quantum = 4
	simulate_RR_with_penalty(proc, n, 4, 1);

    return 0;
}
