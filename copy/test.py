import random
import string
import subprocess
import re

def random_input(len):
    with open("src.txt", 'w') as f:
        for _ in range(len):
            f.write(random.choice(string.ascii_letters + string.digits))
        f.write("\n")

def compile(macro):
    # macro chooses the macro to use, 1 for TIME1, 2 for TIME2

    command = f"make MACRO=TIME{macro}"
    subprocess.run(command, shell=True, check=True, text=True)

def run_and_parse_output(flag: int, len: int):
    # flag chooses the program to run, 0 for MyCopy, 1 for ForkCopy, 2 for PipeCopy
    
    random_input(len)
    cpp_input = f"src.txt dest.txt"
    if flag == 0:
        process = subprocess.run(f"./MyCopy {cpp_input}", shell=True, check=True, text=True, capture_output=True)
    elif flag == 1:
        process = subprocess.run(f"./ForkCopy {cpp_input}", shell=True, check=True, text=True, capture_output=True)
    elif flag == 2:
        process = subprocess.run(f"./PipeCopy {cpp_input}", shell=True, check=True, text=True, capture_output=True)
    # print(f"{process.stdout} with {len} ")
    if (process.stdout is not None):
        return re.findall(r'\d+\.\d+', process.stdout)[0]
    else:
        return -1

def plot(cp, fork, pipe, sizes, name):
    import matplotlib.pyplot as plt
    plt.clf()
    plt.plot(sizes, cp, marker='o', label='MyCopy')
    plt.plot(sizes, fork, marker='o', label='ForkCopy')
    plt.plot(sizes, pipe, marker='o', label='PipeCopy')
    # plt.scatter(sizes, cp, color='red')
    # plt.scatter(sizes, fork, color='blue')
    # plt.scatter(sizes, pipe, color='green')
    plt.xlabel('Input Size')
    if (name == "plot1.png"):
        plt.ylabel('Time (10^-2 ms)')
    else:
        plt.ylabel('Time (10^-1 ms)')
    # plt.gca().invert_yaxis()
    plt.title('Copy Time Comparison')
    plt.legend()
    plt.savefig(name, format='png', dpi=300)
    plt.show()

if __name__ == "__main__":
        mycp_t1 = []
        mycp_t2 = []
        forkcp_t1 = []
        forkcp_t2 = []
        pipecp_t1 = []
        pipecp_t2 = []
        sizes = []
        compile(1)
        for i in range(20):
            size = (i + 1) * 500
            sizes.append(size)
            mycp_t1.append(100 * float(run_and_parse_output(0, size)))
            forkcp_t1.append(100 * float(run_and_parse_output(1, size)))
            pipecp_t1.append(100 * float(run_and_parse_output(2, size)))
        
        compile(2)
        for i in range(20):
            size = (i + 1) * 500
            mycp_t2.append(10 * float(run_and_parse_output(0, size)))
            forkcp_t2.append(10 * float(run_and_parse_output(1, size)))
            pipecp_t2.append(10 * float(run_and_parse_output(2, size)))

        # print(mycp_t1)
        # print(forkcp_t1)
        # print(pipecp_t1)
        plot(mycp_t1, forkcp_t1, pipecp_t1, sizes, "plot1.png")
        # plot(mycp_t1, forkcp_t1, pipecp_t1, sizes, "plot1.png")
        plot(mycp_t2, forkcp_t2, pipecp_t2, sizes, "plot2.png")