#This script runs the student's binary and parses the output. For this to work, students must use the print_stats format provided in the skeleton.
"""
To ensure the autograder works correctly:
1-Do not change the printf strings in the print_stats function.
2-Ensure your workload.txt is in the same directory as your code.
3-Handle Idle Time: If a process arrives at $T=10$ but the CPU finished the last task at $T=5$, the scheduler must "jump" the clock to $T=10$ (don't just start the next task at $T=5$).

Marking Rubric (Total: 100 Points)
Category        Points  Criteria
Compilation     10      Compiles with gcc -Wall without any errors or warnings.
Memory Safety   10      Valgrind reports "0 errors" and "no leaks are possible."
FCFS Logic      15      Correct Wait/TAT for non-preemptive First-Come-First-Served.
SJF Logic       20      Correct selection of the shortest job from the ready pool.
RR with Penalty 30      Correct quantum slicing and the 2ms penalty application.
Analysis Report 15      Short PDF comparing the three algorithms.

This script includes a check_memory function. It uses the --error-exitcode=1 flag, which tells Valgrind to return a failure code (1) if it finds even a single byte of leaked memory.
"""

import subprocess
import re

def run_command(cmd):
    try:
        return subprocess.run(cmd, capture_output=True, text=True, timeout=10)
    except Exception as e:
        print(f"Error running {' '.join(cmd)}: {e}")
        return None

def check_memory():
    print("--- Running Memory Check (Valgrind) ---")
    # --error-exitcode=1 makes valgrind return 1 if any leak or error is found
    cmd = ["valgrind", "--leak-check=full", "--error-exitcode=1", "./scheduler"]
    result = run_command(cmd)
    
    if result and result.returncode == 0:
        print("✅ No memory leaks detected! (+10 pts)")
        return 10
    else:
        print("❌ Memory leaks or errors detected. Check valgrind output below:")
        print(result.stderr if result else "Execution failed")
        return 0

def parse_output(output, policy_name):
    # re.escape converts "(" into "\(" so regex looks for the literal symbol
    escaped_policy = re.escape(policy_name)
    
    pattern = rf"=== {escaped_policy} ===.*?Average Waiting Time: ([\d.]+)"
    match = re.search(pattern, output, re.DOTALL)
    return float(match.group(1)) if match else None

def main():
    points = 0
    # 1. Memory Check
    points += check_memory()

    # 2. Functional Checks
    result = run_command(['./scheduler'])
    if not result: return
    output = result.stdout
    print("output: ", output)
    
    # TEST 1: FCFS (Target: 21.30)
    fcfs_wait = parse_output(output, "FCFS")
    if fcfs_wait and abs(fcfs_wait - 19.20) < 0.1:
        print("✅ FCFS Correct (+15 pts)")
        points += 15
    else:
        print("X FCFS failed: ", fcfs_wait)

    # TEST 2: SJF (Target: 16.50)
    sjf_wait = parse_output(output, "SJF (Non-Preemptive)")
    if sjf_wait and abs(sjf_wait - 16.50) < 0.1:
        print("✅ SJF Correct (+20 pts)")
        points += 20
    else:
        print("X sjf_wait failed: ", sjf_wait)

    # TEST 3: RR with Penalty (Target: 26.10)
    rr_wait = parse_output(output, "Round Robin (With Penalty)")
    if rr_wait and abs(rr_wait - 24.00) < 0.1:
        print("✅ RR with Penalty Correct (+30 pts)")
        points += 30
    else:
        print("X rr_wait failed: ", rr_wait)

    print(f"\nTotal Autograder Score: {points}/75")
    print("(Remaining 25 points: 10 for Compilation, 15 for Report Analysis)")

if __name__ == "__main__":
    main()
