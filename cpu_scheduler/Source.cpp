/*
COP4610 - Fall 2024
Programming Assignment: CPU Scheduler

Made by: Jordan Small (Z23465928)

20 OCT 2024
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <list>
#include <string>
#include <fstream>

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
vector<Process> FCFS(vector<Process> processes, ostream& out);
vector<Process> SJF(vector<Process> processes, ostream& out);
vector<Process> MLFQ(vector<Process> processes, ostream& out);

// Helper Functions
void sort_process_by_id(vector<Process>& processes);
void print_results(vector<Process> processes, ostream& out);
void print_averages(vector<Process> processes, ostream& out);
void sort_process_by_next_cpu_burst(list<Process>& ready_list);
void sort_process_by_queue_level(list<Process>& ready_list);
void calculate_wait_time(vector<Process>& processes);
int get_preemption_time(int level_counter);

// Functions for main driver
vector<Process> load_processes();
void welcome();
char mainMenu();
void makeSelection(char choice, vector<Process> processes);
char output();
void goodbye();

int main() {

    vector <Process> processes = load_processes();
    welcome();

    char menu_choice = mainMenu();

    while (menu_choice != 'Q') {

        makeSelection(menu_choice, processes);
        menu_choice = mainMenu();

    }

    goodbye();
    return 0;

}

vector<Process> FCFS(vector<Process> processes, ostream& out) {

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

        out << "Time: " << time << endl;

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

            out << "Ran: [Process " << current_process->get_id() << ", CPU Burst remaining: " << current_process->get_bursts()[current_process->get_burst_index()] << "]" << endl;

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
                    out << "***Process " << current_process->get_id() << " completed at time " << time + 1 << "***" << endl;
                }

                ready_list.pop_front(); // Remove from ready list
                current_process = nullptr; // Reset current process to allow next scheduling

            }
        }

        else {
            out << "[No process running]" << endl;
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
        out << "Ready List: [";
        for (auto it = ready_list.begin(); it != ready_list.end(); it++) {
            if (it != ready_list.begin()) {
                out << "{Process " << it->get_id() << ", ";
                out << "Upcoming CPU Burst: " << it->get_bursts()[it->get_burst_index()] << "}";
                if (next(it) != ready_list.end()) {
                    out << ", ";
                }
            }
        }
        out << "]" << endl;

        // Print I/O List
        out << "I/O List: [";
        for (auto it = io_list.begin(); it != io_list.end(); it++) {
            out << "{Process " << it->get_id() << ", ";
            out << "I/O Burst remaining: " << it->get_bursts()[it->get_burst_index()] << "}";
            if (next(it) != io_list.end()) {
                out << ", ";
            }
        }
        out << "]" << endl;

        time++; // Increment time

    }

    calculate_wait_time(completed_list);
    out << endl << "------------------------------------";
    out << endl << "Total run time: " << time << " units" << endl;
    double cpuUtilPercent = (double)cpuUtil / time * 100;
    out << "CPU Utilization: " << cpuUtilPercent << "%" << endl << endl;
    return completed_list;

}

vector<Process> SJF(vector<Process> processes, ostream& out) {

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

        out << "Time: " << time << endl;

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

            out << "Ran: [Process " << current_process->get_id() << ", CPU Burst remaining: " << current_process->get_bursts()[current_process->get_burst_index()] << "]" << endl;

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
                    out << "***Process " << current_process->get_id() << " completed at time " << time + 1 << "***" << endl;
                }

                ready_list.pop_front(); // Remove from ready list
                current_process = nullptr; // Reset current process to allow next scheduling

            }
        }
        else {
            out << "[No process running]" << endl;
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
        out << "Ready List: [";
        for (auto it = ready_list.begin(); it != ready_list.end(); it++) {
            if (it != ready_list.begin()) {
                out << "{Process " << it->get_id() << ", ";
                out << "Upcoming CPU Burst: " << it->get_bursts()[it->get_burst_index()] << "}";
                if (next(it) != ready_list.end()) {
                    out << ", ";
                }
            }
        }
        out << "]" << endl;

        // Print I/O List
        out << "I/O List: [";
        for (auto it = io_list.begin(); it != io_list.end(); it++) {
            out << "{Process " << it->get_id() << ", ";
            out << "I/O Burst remaining: " << it->get_bursts()[it->get_burst_index()] << "}";
            if (next(it) != io_list.end()) {
                out << ", ";
            }
        }
        out << "]" << endl;

        time++; // Increment time
    }

    calculate_wait_time(completed_list);
    out << endl << "------------------------------------";
    out << endl << "Total run time: " << time << " units" << endl;
    double cpuUtilPercent = (double)cpuUtil / time * 100;
    out << "CPU Utilization: " << cpuUtilPercent << "%" << endl << endl;
    return completed_list;

}

vector<Process> MLFQ(vector<Process> processes, ostream& out) {

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

        out << "Time: " << time << endl;

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

            out << "Ran: [Process " << current_process->get_id() << ", CPU Burst remaining: " << current_process->get_bursts()[current_process->get_burst_index()] << "]" << endl;

            if (remaining_tq < 11) {
                out << "Remaining tq: " << remaining_tq << endl;
            }
            else {
                out << "Remaining tq: Burst is in Queue 3 and will run until finished" << endl;
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
                        out << "***Process " << current_process->get_id() << " completed at time " << time + 1 << "***" << endl;
                    }
                }
                else {
                    // Process has completed all bursts
                    current_process->set_state(4);
                    current_process->set_turnaroundTime(time + 1);
                    completed_list.push_back(*current_process);
                    out << "***Process " << current_process->get_id() << " completed at time " << time + 1 << "***" << endl;
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
        out << "Ready List: [";
        for (auto it = ready_list.begin(); it != ready_list.end(); ++it) {
            out << "{Process " << it->get_id() << ", Upcoming CPU Burst: " << it->get_bursts()[it->get_burst_index()] << "}";
            if (next(it) != ready_list.end()) {
                out << ", ";
            }
        }
        out << "]" << endl;

        // Print I/O List
        out << "I/O List: [";
        for (auto it = io_list.begin(); it != io_list.end(); ++it) {
            out << "{Process " << it->get_id() << ", I/O Burst remaining: " << it->get_bursts()[it->get_burst_index()] << "}";
            if (next(it) != io_list.end()) {
                out << ", ";
            }
        }
        out << "]" << endl;

        time++; // Increment time
    }

    calculate_wait_time(completed_list);
    out << endl << "------------------------------------";
    out << endl << "Total run time: " << time << " units" << endl;
    double cpuUtilPercent = (double)cpuUtil / time * 100;
    out << "CPU Utilization: " << cpuUtilPercent << "%" << endl << endl;
    return completed_list;

}


// Functions for main driver
vector<Process> load_processes() {
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
    return processes;
}

void welcome() {
    cout << "******************************************************************************************************" << endl << endl;
    cout << "Welcome to my CPU Scheduling Simulator!" << endl << endl;
    cout << "In this program, users will have the option to simulate three different scheduling algorithms " << endl;
    cout << "(FCFS, SJF, and MLFQ) with a pre-loaded set of processes that include interger arrays " << endl;
    cout << "of their bursts-- both CPU and IO. Users can display a dynamic execution of these processes, followed" << endl;
    cout << "by individual process times, as well as their averages. Users can select whether to" << endl;
    cout << "output these results to the console or an output file." << endl << endl;
    cout << "******************************************************************************************************";
}

char mainMenu() {

    char choice;

    cout << endl << endl;
    cout << "Please choose from the following menu choices:" << endl;
    cout << "Select 'F' or 'f'... to run FCFS simulation" << endl;
    cout << "Select 'S' or 's'... to run SJF simulation" << endl;
    cout << "Select 'M' or 'm'... to run MLFQ simulation" << endl;
    cout << "Select 'Q' or 'q'... to QUIT" << endl << endl;

    choice = cin.get();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    choice = toupper(choice);

    return choice;

}

char output() {

    char choice;

    cout << endl;
    cout << "Please choose from the following menu choices (there is no going back to main menu before selecting): " << endl;
    cout << "Select 'A'... or 'a'... to output to console" << endl;
    cout << "Select 'b'... or 'B'... to output to a text file" << endl;

    choice = cin.get();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    choice = toupper(choice);

    return choice;
}

void makeSelection(char menu_choice, vector<Process> processes) {

    ofstream outputF("fcfs.txt");
    ofstream outputS("sjf.txt");
    ofstream outputM("mlfq.txt");

    switch (menu_choice) {

    case 'F':
        char fselect;
        cout << endl << "You have selected the FCFS algorithm!" << endl;
        do {
            fselect = output();
        } while (fselect != 'A' && fselect != 'B');

        if (fselect == 'A') {
            vector <Process> complete_fcfs = FCFS(processes, cout);
            print_averages(complete_fcfs, cout);
            print_results(complete_fcfs, cout);

        }
        else {
            vector <Process> complete_fcfs = FCFS(processes, outputF);
            print_averages(complete_fcfs, outputF);
            print_results(complete_fcfs, outputF);
            outputF.close();
            cout << endl << "Simulation complete!Your output will be found in the same directory as this program in a file named 'fcfs.txt'" << endl;
        }
        break;

    case 'S':
        char sselect;
        cout << endl << "You have selected the SJF algorithm!" << endl;
        do {
            sselect = output();
        } while (sselect != 'A' && sselect != 'B');

        if (sselect == 'A') {
            vector <Process> complete_sjf = SJF(processes, cout);
            print_averages(complete_sjf, cout);
            print_results(complete_sjf, cout);

        }
        else {
            vector <Process> complete_sjf = SJF(processes, outputS);
            print_averages(complete_sjf, outputS);
            print_results(complete_sjf, outputS);
            outputS.close();
            cout << endl << "Simulation complete! Your output will be found in the same directory as this program in a file named 'sjf.txt'" << endl;
        }
        break;

    case 'M':
        char mselect;
        cout << endl << "You have selected the MLFQ algorithm!" << endl;
        do {
            mselect = output();
        } while (mselect != 'A' && mselect != 'B');

        if (mselect == 'A') {
            vector <Process> complete_mlfq = MLFQ(processes, cout);
            print_averages(complete_mlfq, cout);
            print_results(complete_mlfq, cout);

        }
        else {
            vector <Process> complete_mlfq = MLFQ(processes, outputM);
            print_averages(complete_mlfq, outputM);
            print_results(complete_mlfq, outputM);
            outputM.close();
            cout << endl << "Simulation complete! Your output will be found in the same directory as this program in a file named 'mlfq.txt'" << endl;
        }
        break;

    default:
        cout << endl << "Invalid menu choice!" << endl;

    }
}

// Helper functions
void print_results(vector<Process> processes, ostream& out) {

    sort_process_by_id(processes);

    for (int i = 0; i < processes.size(); i++) {
        out << "Process " << processes[i].get_id() << ": " << endl;
        out << "  - Turnaround Time: " << processes[i].get_turnaroundTime() << endl;
        out << "  - Wait Time: " << processes[i].get_waitTime() << endl;
        out << "  - Response Time: " << processes[i].get_responseTime() << endl;
        out << endl;
    }

    out << endl << "Simulation complete!" << endl;

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

void print_averages(vector<Process> processes, ostream& out) {

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

    out << "Average Turnaround Time: " << average_turnaround_time << endl;
    out << "Average Wait Time: " << average_wait_time << endl;
    out << "Average Response Time: " << average_response_time << endl;
    out << "------------------------------------" << endl << endl;

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

// ASCII -- why not?!
void goodbye() {

    cout << endl << "Thanks for participating! Have a great day!" << endl << endl;

    cout << "                                                                      ddddddd       bbbbbbb                                                          " << endl;
    cout << "         GGGGGGGGGGGGG                                                d:::::d       b:::::b                                                    !!!   " << endl;
    cout << "       GGG::::::::::::G                                               d:::::d       b:::::b                                                   !!::!  " << endl;
    cout << "      GG:::::::::::::::G                                              d:::::d       b:::::b                                                   !:::!  " << endl;
    cout << "     G:::::GGGGGGGG::::G                                              d:::::d       b:::::b                                                   !:::!  " << endl;
    cout << "    G:::::G       GGGGGG   ooooooooooo     ooooooooooo        ddddddddd:::::d       b:::::bbbbbbbbb yyyyyyy           yyyyyyy eeeeeeeeeeee    !:::!  " << endl;
    cout << "   G:::::G               oo:::::::::::o ooo:::::::::::oo    dd::::::::::::::d       b::::::::::::::bby:::::y         y:::::yee::::::::::::ee  !:::!  " << endl;
    cout << "   G:::::G              o:::::::::::::::oo:::::::::::::::o d::::::::::::::::d       b::::::::::::::::by:::::y       y:::::ye::::::eeeee:::::ee!:::!  " << endl;
    cout << "   G:::::G    GGGGGGGGGGo:::::ooooo:::::oo:::::ooooo:::::od:::::::ddddd:::::d       b:::::bbbbb:::::::by:::::y     y:::::ye::::::e     e:::::e!:::!  " << endl;
    cout << "   G:::::G    G::::::::Go::::o     o::::oo::::o     o::::od::::::d    d:::::d       b:::::b    b::::::b y:::::y   y:::::y e:::::::eeeee::::::e!:::!  " << endl;
    cout << "   G:::::G    GGGGG::::Go::::o     o::::oo::::o     o::::od:::::d     d:::::d       b:::::b     b:::::b  y:::::y y:::::y  e:::::::::::::::::e !:::!  " << endl;
    cout << "   G:::::G        G::::Go::::o     o::::oo::::o     o::::od:::::d     d:::::d       b:::::b     b:::::b   y:::::y:::::y   e::::::eeeeeeeeeee  !!:!!  " << endl;
    cout << "    G:::::G       G::::Go::::o     o::::oo::::o     o::::od:::::d     d:::::d       b:::::b     b:::::b    y:::::::::y    e:::::::e            !!!   " << endl;
    cout << "     G:::::GGGGGGGG::::Go:::::ooooo:::::oo:::::ooooo:::::od::::::ddddd::::::d       b:::::bbbbbb::::::b     y:::::::y     e::::::::e                 " << endl;
    cout << "      GG:::::::::::::::Go:::::::::::::::oo:::::::::::::::od:::::::::::::::::d       b::::::::::::::::b       y:::::y       e::::::::eeeeeeee   !!!   " << endl;
    cout << "        GGG::::::GGG:::G oo:::::::::::oo  oo:::::::::::oo  d:::::::::ddd::::d       b:::::::::::::::b       y:::::y         ee:::::::::::::e  !!:!!  " << endl;
    cout << "           GGGGGG   GGGG   ooooooooooo      ooooooooooo      ddddddddd  ddddd       bbbbbbbbbbbbbbbb       y:::::y            eeeeeeeeeeeeee   !!!   " << endl;
    cout << "                                                                                                          y:::::y                                    " << endl;
    cout << "                                                                                                         y:::::y                                     " << endl;
    cout << "                                                                                                        y:::::y                                      " << endl;
    cout << "                                                                                                       y:::::y                                       " << endl;
    cout << "                                                                                                      yyyyyyy                                        " << endl;

}