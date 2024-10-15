#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <list>

using namespace std;

class Process {

private:

    int id;
    vector<int> bursts;
    int burst_index;

    int totalRunTime;
    int waitTime;
    int turnaroundTime;
    int responseTime;
    int timeEnteringReady; // for sjf
    int level_counter; //for mlfq

    int state;

public:

    // Constructor
    Process(int id, vector<int> bursts);

    // Getters
    int get_id();
    vector<int>& get_bursts();
    int get_burst_index();
    int get_waitTime();
    int get_turnaroundTime();
    int get_responseTime();
    int get_state();
    int get_timeEnteringReady();
    int get_level_counter();
    int get_total_run();

    // Setters
    void set_waitTime(int waitTime);
    void set_turnaroundTime(int turnaroundTime);
    void set_responseTime(int responseTime);
    void set_state(int state);
    void set_burst_index(int burst_index);
    void set_timeEnterReady(int timeEnterReady);

    // Helpers
    void increment_level_counter();
    void reset_level_counter();
    int calculate_total_run();

};

// Scheduling Algorithms
vector<Process> FCFS(vector<Process> processes);
vector<Process> SJF(vector<Process> processes);
vector<Process> MLFQ(vector<Process> processes);

// Helper Functions
void sort_process_by_id(vector<Process>& processes);
void print_results(vector<Process> processes);
void print_averages(vector<Process> processes);
void sort_process_by_next_cpu_burst(list<Process>& ready_list);
void sort_process_by_queue_level(list<Process>& ready_list);
void calculate_wait_time(vector<Process>& processes);
int get_preemption_time(int level_counter);

int main() {

    vector<Process> processes = {
        Process(1, {5, 27, 3, 31, 5, 43, 4, 18, 6, 22, 4, 26, 3, 24, 4}),
        Process(2, {4, 48, 5, 44, 7, 42, 12, 37, 9, 76, 4, 41, 9, 31, 7, 43, 8}),
        Process(3, {8, 33, 12, 41, 18, 65, 14, 21, 4, 61, 15, 18, 14, 26, 5, 31, 6}),
        Process(4, {3, 35, 4, 41, 5, 45, 3, 51, 4, 61, 5, 54, 6, 82, 5, 77, 3}),
        Process(5, {16, 24, 17, 21, 5, 36, 16, 26, 7, 31, 13, 28, 11, 21, 6, 13, 3, 11, 4}),
        Process(6, {11, 22, 4, 8, 5, 10, 6, 12, 7, 14, 9, 18, 12, 24, 15, 30, 8}),
        Process(7, {14, 46, 17, 41, 11, 42, 15, 21, 4, 32, 7, 19, 16, 33, 10}),
        Process(8, {4, 14, 5, 33, 6, 51, 14, 73, 16, 87, 6})
    };

    vector <Process> complete_fcfs = FCFS(processes);
    print_results(complete_fcfs);
    print_averages(complete_fcfs);

    //vector <Process> complete_sjf = SJF(processes);
    //print_results(complete_sjf);
    //print_averages(complete_sjf);

    //vector <Process> complete_mlfq = MLFQ(processes);
    //print_results(complete_mlfq);
    //print_averages(complete_mlfq);

    return 0;

}

vector<Process> FCFS(vector<Process> processes) {

    list<Process> ready_list;
    list<Process> io_list;
    vector<Process> completed_list;

    int time = 0;
    int cpuUtil = 0;

    // Initialize the ready list
    for (const auto& process : processes) {
        ready_list.push_back(process);
    }

    Process* current_process = nullptr; // Pointer for the currently running process

    while (completed_list.size() < processes.size()) {

        cout << "Time: " << time << endl;

        // Check for a running process
        if (!current_process && !ready_list.empty()) {
            current_process = &ready_list.front(); // Get the first process in the ready list
            current_process->set_state(2); // Set to running

            if (current_process->get_responseTime() == -1) {
                current_process->set_responseTime(time);
            }
        }

        // Run the current process if it exists
        if (current_process) {

            current_process->get_bursts()[current_process->get_burst_index()] -= 1;
            cpuUtil++;

            cout << "Ran: [Process " << current_process->get_id() << ", CPU Burst remaining: " << current_process->get_bursts()[current_process->get_burst_index()] << "]" << endl;

            // Check if the CPU burst is finished
            if (current_process->get_bursts()[current_process->get_burst_index()] == 0) {

                current_process->set_burst_index(current_process->get_burst_index() + 1);

                // Check for I/O or completion
                if (current_process->get_burst_index() % 2 != 0 && current_process->get_burst_index() < current_process->get_bursts().size()) {
                    current_process->set_state(3); // Set to I/O
                    current_process->get_bursts()[current_process->get_burst_index()]++; // Adjust for I/O burst
                    io_list.push_back(*current_process); // Copy process to I/O list
                }
                else {
                    current_process->set_state(4); // Set to completed
                    current_process->set_turnaroundTime(time + 1);
                    completed_list.push_back(*current_process);
                    cout << "***Process " << current_process->get_id() << " completed at time " << time + 1 << "***" << endl;
                }

                ready_list.pop_front(); // Remove from ready list
                current_process = nullptr; // Reset current process to allow next scheduling

            }
        }

        else {
            cout << "[No process running]" << endl;
        }

        // Handle I/O processes
        if (!io_list.empty()) {

            for (auto it = io_list.begin(); it != io_list.end(); ) {
                it->get_bursts()[it->get_burst_index()] -= 1;

                // Check if I/O burst is finished
                if (it->get_bursts()[it->get_burst_index()] == 0) {
                    it->set_state(1); // Set to ready
                    it->set_burst_index(it->get_burst_index() + 1);
                    ready_list.push_back(*it); // Push back to ready list
                    it = io_list.erase(it); // Erase and get the next iterator
                }
                else {
                    ++it; // Move to the next element
                }

            }
        }

        // Print Ready List
        cout << "Ready List: [";
        for (auto it = ready_list.begin(); it != ready_list.end(); it++) {
            if (it != ready_list.begin()) {
                cout << "{Process " << it->get_id() << ", ";
                cout << "Upcoming CPU Burst: " << it->get_bursts()[it->get_burst_index()] << "}";
                if (next(it) != ready_list.end()) {
                    cout << ", ";
                }
            }
        }
        cout << "]" << endl;

        // Print I/O List
        cout << "I/O List: [";
        for (auto it = io_list.begin(); it != io_list.end(); it++) {
            cout << "{Process " << it->get_id() << ", ";
            cout << "I/O Burst remaining: " << it->get_bursts()[it->get_burst_index()] << "}";
            if (next(it) != io_list.end()) {
                cout << ", ";
            }
        }
        cout << "]" << endl;

        time++; // Increment time

    }

    calculate_wait_time(completed_list);
    cout << endl << "Total run time: " << time << " units" << endl;
    double cpuUtilPercent = (double)cpuUtil / time * 100;
    cout << "CPU Utilization: " << cpuUtilPercent << "%" << endl << endl;
    return completed_list;

}

vector<Process> SJF(vector<Process> processes) {

    list<Process> ready_list;
    list<Process> io_list;
    vector<Process> completed_list;

    int time = 0;
    int cpuUtil = 0;

    // Initialize the ready list
    for (const auto& process : processes) {
        ready_list.push_back(process);
    }

    Process* current_process = nullptr; // Pointer for the currently running process

    while (completed_list.size() < processes.size()) {

        cout << "Time: " << time << endl;

        // Check for a running process
        if (!current_process && !ready_list.empty()) {
            sort_process_by_next_cpu_burst(ready_list);
            current_process = &ready_list.front(); // Get the process with the shortest burst
            current_process->set_state(2); // Set to running

            if (current_process->get_responseTime() == -1) {
                current_process->set_responseTime(time);
            }
        }

        // Run the current process if it exists
        if (current_process) {

            // Execute for one time unit
            current_process->get_bursts()[current_process->get_burst_index()] -= 1;
            cpuUtil++;

            cout << "Ran: [Process " << current_process->get_id() << ", CPU Burst remaining: " << current_process->get_bursts()[current_process->get_burst_index()] << "]" << endl;

            // Check if the CPU burst is finished
            if (current_process->get_bursts()[current_process->get_burst_index()] == 0) {
                current_process->set_burst_index(current_process->get_burst_index() + 1);

                // Check for I/O or completion
                if (current_process->get_burst_index() % 2 != 0 && current_process->get_burst_index() < current_process->get_bursts().size()) {
                    current_process->set_state(3); // Set to I/O
                    current_process->get_bursts()[current_process->get_burst_index()]++; // Adjust for I/O burst
                    io_list.push_back(*current_process); // Copy process to I/O list
                }
                else {
                    current_process->set_state(4); // Set to completed
                    current_process->set_turnaroundTime(time + 1);
                    completed_list.push_back(*current_process);
                    cout << "***Process " << current_process->get_id() << " completed at time " << time + 1 << "***" << endl;
                }

                ready_list.pop_front(); // Remove from ready list
                current_process = nullptr; // Reset current process to allow next scheduling

            }
        }
        else {
            cout << "[No process running]" << endl;
        }

        // Handle I/O processes
        if (!io_list.empty()) {

            for (auto it = io_list.begin(); it != io_list.end(); ) {
                it->get_bursts()[it->get_burst_index()] -= 1;

                // Check if I/O burst is finished
                if (it->get_bursts()[it->get_burst_index()] == 0) {
                    it->set_state(1); // Set to ready
                    it->set_burst_index(it->get_burst_index() + 1);
                    it->set_timeEnterReady(time);
                    ready_list.push_back(*it); // Push back to ready list
                    it = io_list.erase(it); // Erase and get the next iterator
                }
                else {
                    ++it; // Move to the next element
                }

            }
        }

        // Print Ready List
        cout << "Ready List: [";
        for (auto it = ready_list.begin(); it != ready_list.end(); it++) {
            if (it != ready_list.begin()) {
                cout << "{Process " << it->get_id() << ", ";
                cout << "Upcoming CPU Burst: " << it->get_bursts()[it->get_burst_index()] << "}";
                if (next(it) != ready_list.end()) {
                    cout << ", ";
                }
            }
        }
        cout << "]" << endl;

        // Print I/O List
        cout << "I/O List: [";
        for (auto it = io_list.begin(); it != io_list.end(); it++) {
            cout << "{Process " << it->get_id() << ", ";
            cout << "I/O Burst remaining: " << it->get_bursts()[it->get_burst_index()] << "}";
            if (next(it) != io_list.end()) {
                cout << ", ";
            }
        }
        cout << "]" << endl;

        time++; // Increment time
    }

    calculate_wait_time(completed_list);
    cout << endl << "Total run time: " << time << " units" << endl;
    double cpuUtilPercent = (double)cpuUtil / time * 100;
    cout << "CPU Utilization: " << cpuUtilPercent << "%" << endl << endl;
    return completed_list;

}

vector<Process> MLFQ(vector<Process> processes) {

    list<Process> ready_list; // Combined ready queue
    list<Process> io_list;    // I/O Queue
    vector<Process> completed_list;

    int time = 0;
    int cpuUtil = 0;
    Process* current_process = nullptr; // Pointer for the currently running process
    int remaining_tq = 0; // Variable to track remaining time quantum

    // Initialize the ready list
    for (auto& process : processes) {
        ready_list.push_back(process);
    }

    while (completed_list.size() < processes.size()) {

        cout << "Time: " << time << endl;

        // Check for a running process
        if (!current_process && !ready_list.empty()) {
            sort_process_by_queue_level(ready_list);
            current_process = &ready_list.front(); // Get the first process in the ready list
            current_process->set_state(2); // Set to running

            if (current_process->get_responseTime() == -1) {
                current_process->set_responseTime(time);
            }

            remaining_tq = get_preemption_time(current_process->get_level_counter());

        }

        // When running the current process
        if (current_process) {

            current_process->get_bursts()[current_process->get_burst_index()] -= 1;
            cpuUtil++;
            remaining_tq--;

            cout << "Ran: [Process " << current_process->get_id() << ", CPU Burst remaining: " << current_process->get_bursts()[current_process->get_burst_index()] << "]" << endl;

            if (remaining_tq < 11) {
                cout << "Remaining tq: " << remaining_tq << endl;
            }
            else {
                cout << "Remaining tq: Burst is in Queue 3 and will run until finished" << endl;
            }

            // If the process did not finish and the quantum has expired
            if (remaining_tq == 0 && current_process->get_bursts()[current_process->get_burst_index()] != 0) {

                current_process->increment_level_counter();
                ready_list.push_back(*current_process);
                ready_list.pop_front();
                current_process = nullptr;

            }

            // Check if the CPU burst is finished
            if ((current_process) && current_process->get_bursts()[current_process->get_burst_index()] == 0) {
                current_process->set_burst_index(current_process->get_burst_index() + 1);

                // Check if the burst index is still valid before accessing
                if (current_process->get_burst_index() < current_process->get_bursts().size()) {
                    // If it is an I/O burst
                    if (current_process->get_burst_index() % 2 != 0) {
                        current_process->set_state(3); // Set to I/O
                        io_list.push_back(*current_process); // Move to I/O queue
                    }
                    else {
                        // Completed process
                        current_process->set_state(4);
                        current_process->set_turnaroundTime(time + 1);
                        completed_list.push_back(*current_process);
                        cout << "***Process " << current_process->get_id() << " completed at time " << time + 1 << "***" << endl;
                    }
                }
                else {
                    // Process has completed all bursts
                    current_process->set_state(4);
                    current_process->set_turnaroundTime(time + 1);
                    completed_list.push_back(*current_process);
                    cout << "***Process " << current_process->get_id() << " completed at time " << time + 1 << "***" << endl;
                }

                current_process->reset_level_counter();
                ready_list.pop_front();
                current_process = nullptr;
            }
        }

        // Handle I/O processes
        if (!io_list.empty()) {
            for (auto it = io_list.begin(); it != io_list.end();) {
                it->get_bursts()[it->get_burst_index()] -= 1;

                // Check if I/O burst is finished
                if (it->get_bursts()[it->get_burst_index()] == 0) {
                    it->set_state(1); // Set to ready
                    it->set_burst_index(it->get_burst_index() + 1);
                    it->set_timeEnterReady(time);
                    // Ensure the new burst index is valid
                    if (it->get_burst_index() < it->get_bursts().size()) {
                        ready_list.push_back(*it); // Move back to ready_list
                    }
                    it = io_list.erase(it); // Remove from I/O list
                }
                else {
                    ++it; // Move to the next element
                }
            }
        }

        // Print Ready List
        cout << "Ready List: [";
        for (auto it = ready_list.begin(); it != ready_list.end(); ++it) {
            cout << "{Process " << it->get_id() << ", Upcoming CPU Burst: " << it->get_bursts()[it->get_burst_index()] << "}";
            if (next(it) != ready_list.end()) {
                cout << ", ";
            }
        }
        cout << "]" << endl;

        // Print I/O List
        cout << "I/O List: [";
        for (auto it = io_list.begin(); it != io_list.end(); ++it) {
            cout << "{Process " << it->get_id() << ", I/O Burst remaining: " << it->get_bursts()[it->get_burst_index()] << "}";
            if (next(it) != io_list.end()) {
                cout << ", ";
            }
        }
        cout << "]" << endl;

        time++; // Increment time
    }

    calculate_wait_time(completed_list);
    cout << endl << "Total run time: " << time << " units" << endl;
    double cpuUtilPercent = (double)cpuUtil / time * 100;
    cout << "CPU Utilization: " << cpuUtilPercent << "%" << endl << endl;
    return completed_list;

}

void print_results(vector<Process> processes) {

    sort_process_by_id(processes);

    for (int i = 0; i < processes.size(); i++) {
        cout << "Process " << processes[i].get_id() << ": " << endl;
        cout << "  - Turnaround Time: " << processes[i].get_turnaroundTime() << endl;
        cout << "  - Wait Time: " << processes[i].get_waitTime() << endl;
        cout << "  - Response Time: " << processes[i].get_responseTime() << endl;
        cout << endl;
    }

}

void calculate_wait_time(vector<Process>& processes) {

    for (int i = 0; i < processes.size(); i++) {
        processes[i].set_waitTime(processes[i].get_turnaroundTime() - processes[i].get_total_run());
    }

}
void sort_process_by_id(vector<Process>& processes) {
    sort(processes.begin(), processes.end(), [](Process a, Process b) {
        return a.get_id() < b.get_id();
        });
}

void print_averages(vector<Process> processes) {

    double total_turnaround_time = 0;
    double total_wait_time = 0;
    double total_response_time = 0;

    for (int i = 0; i < processes.size(); i++) {
        total_turnaround_time += processes[i].get_turnaroundTime();
        total_wait_time += processes[i].get_waitTime();
        total_response_time += processes[i].get_responseTime();
    }

    double average_turnaround_time = total_turnaround_time / processes.size();
    double average_wait_time = total_wait_time / processes.size();
    double average_response_time = total_response_time / processes.size();

    cout << "Average Turnaround Time: " << average_turnaround_time << endl;
    cout << "Average Wait Time: " << average_wait_time << endl;
    cout << "Average Response Time: " << average_response_time << endl;

}

void sort_process_by_next_cpu_burst(list<Process>& ready_list) {
    ready_list.sort([](Process& a, Process& b) {
        // First compare the upcoming CPU burst
        if (a.get_bursts()[a.get_burst_index()] != b.get_bursts()[b.get_burst_index()]) {
            return a.get_bursts()[a.get_burst_index()] < b.get_bursts()[b.get_burst_index()];
        }
        // If they are equal, compare timeEnteringReady (for SJF)
        return a.get_timeEnteringReady() < b.get_timeEnteringReady();
        });
}

void sort_process_by_queue_level(list<Process>& ready_list) {
    ready_list.sort([](Process& a, Process& b) {
        // First compare the current level
        if (a.get_level_counter() != a.get_level_counter()) {
            return a.get_bursts()[a.get_burst_index()] < b.get_bursts()[b.get_burst_index()];
        }
        // If they are equal, compare timeEnteringReady (for MLFQ)
        return a.get_timeEnteringReady() < b.get_timeEnteringReady();
        });

}

int get_preemption_time(int level_counter) {
    switch (level_counter) {
    case 0: return 5;  // RR with tq = 5
    case 1: return 10; // RR with tq = 10
    case 2: return INT_MAX; // FCFS (runs to completion)
    default: return 5;  // Default case (should not happen)
    }
}

// Constructor
Process::Process(int id, vector<int> bursts) {

    this->id = id;
    this->bursts = bursts;
    this->burst_index = 0;
    this->waitTime = 0;
    this->turnaroundTime = 0;
    this->responseTime = -1;
    this->state = 1; // 1 = ready, 2 = running, 3 = i/o, 4 = completed
    this->timeEnteringReady = 0;
    this->level_counter = 0;
    this->totalRunTime = calculate_total_run();

}

// Getters
int Process::get_id() {
    return this->id;
}

vector<int>& Process::get_bursts() {
    return this->bursts;
}

int Process::get_burst_index() {
    return this->burst_index;
}

int Process::get_waitTime() {
    return this->waitTime;
}

int Process::get_turnaroundTime() {
    return this->turnaroundTime;
}

int Process::get_responseTime() {
    return this->responseTime;
}

int Process::get_state() {
    return this->state;
}

int Process::get_timeEnteringReady() {
    return this->timeEnteringReady;
}

int Process::get_level_counter() {
    return this->level_counter;
}

int Process::get_total_run() {
    return this->totalRunTime;
}

// Setters
void Process::set_waitTime(int waitTime) {
    this->waitTime = waitTime;
}

void Process::set_turnaroundTime(int turnaroundTime) {
    this->turnaroundTime = turnaroundTime;
}

void Process::set_responseTime(int responseTime) {
    this->responseTime = responseTime;
}

void Process::set_state(int state) {
    this->state = state;
}

void Process::set_burst_index(int burst_index) {
    this->burst_index = burst_index;
}

void Process::set_timeEnterReady(int timeEnterReady) {
    this->timeEnteringReady = timeEnterReady;
}

// Helpers
int Process::calculate_total_run() {

    int totalRun = 0;

    for (int i = 0; i < get_bursts().size(); i++) {
        totalRun += get_bursts()[i];
    }

    return totalRun;

}
void Process::increment_level_counter() {
    if (level_counter < 2) {
        level_counter++;
    }
}

void Process::reset_level_counter() {
    level_counter = 0;
}