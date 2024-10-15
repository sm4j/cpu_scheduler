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

    int waitTime;
    int turnaroundTime;
    int responseTime;

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

    // Setters
    void set_waitTime(int waitTime);
    void set_turnaroundTime(int turnaroundTime);
    void set_responseTime(int responseTime);
    void set_state(int state);
    void set_burst_index(int burst_index);
};

vector<Process> FCFS(vector<Process> processes);
void sort_process_by_id(vector<Process>& processes);
void print_results(vector<Process> processes);
void print_averages(vector<Process> processes);

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

    vector <Process> complete_processes = FCFS(processes);
    print_results(complete_processes);
    print_averages(complete_processes);
    return 0;
}

Process::Process(int id, vector<int> bursts) {
    this->id = id;
    this->bursts = bursts;
    this->burst_index = 0;
    this->waitTime = 0;
    this->turnaroundTime = 0;
    this->responseTime = -1;
    this->state = 1; // 1 = ready, 2 = running, 3 = i/o, 4 = completed
}

vector<Process> FCFS(vector<Process> processes) {
    list<Process> ready_list;
    list<Process> io_list;
    vector<Process> completed_list;

    int time = 0;
    int cpuUtil = 0;

    for (const auto& process : processes) {
        ready_list.push_back(process);
    }

    while (completed_list.size() < processes.size()) {

        cout << "Time: " << time << endl;

        // Increment wait time for all processes in the ready list except the first one
        if (ready_list.size() > 1) { // Check if there's more than one process
            for (auto it = next(ready_list.begin()); it != ready_list.end(); ++it) {
                it->set_waitTime(it->get_waitTime() + 1);
            }
        }

        if (!ready_list.empty()) {
            Process& current_process = ready_list.front();
            current_process.set_state(2); // Set to running

            if (current_process.get_responseTime() == -1) {
                current_process.set_responseTime(time);
            }

            current_process.get_bursts()[current_process.get_burst_index()] -= 1;
            cpuUtil++;

            cout << "Ran: [Process " << current_process.get_id() << ", CPU Burst remaining: " << current_process.get_bursts()[current_process.get_burst_index()] << "]" << endl;

            // Check if current CPU burst is finished
            if (current_process.get_bursts()[current_process.get_burst_index()] == 0) {

                current_process.set_burst_index(current_process.get_burst_index() + 1);

                // Check if we go to I/O
                if (current_process.get_burst_index() % 2 != 0 && current_process.get_burst_index() < current_process.get_bursts().size()) {
                    current_process.set_state(3); // Set to I/O
                    current_process.get_bursts()[current_process.get_burst_index()]++; // not happy about this, but seems to fix issue
                    io_list.push_back(current_process);
                }
                else {
                    current_process.set_state(4); // Set to completed
                    current_process.set_turnaroundTime(time + 1);
                    completed_list.push_back(current_process);
                    cout << "***Process " << current_process.get_id() << " completed at time " << time + 1 << "***" << endl;
                }

                ready_list.pop_front(); // Remove from ready_list
            }
        }

        if (ready_list.empty()) cout << "[No process running]" << endl;

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

    double cpuUtilPercent = (double)cpuUtil / time * 100;
    cout << "CPU Utilization: " << cpuUtilPercent << "%" << endl;
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