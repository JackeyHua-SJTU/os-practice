import random
import subprocess
import re


def random_array(n):
    rand_arr = [random.randint(1, 10000) for _ in range(n)]
    return ' '.join(map(str, rand_arr))

def run(size, flag):
    cpp_input = f"{size}\n{random_array(size)}"
    if (flag):
        process = subprocess.run("./MergesortMulti", shell=True, check=True, input=cpp_input, text=True, capture_output=True)
    else:
        process = subprocess.run("./MergesortSingle", shell=True, check=True, input=cpp_input, text=True, capture_output=True)
    
    if (process.stdout is not None):
        return re.findall(r'\d+\.\d+', process.stdout)[0]
    else:
        return -1
    
def plot(single, multi, sizes, name):
    import matplotlib.pyplot as plt
    plt.clf()
    plt.plot(sizes, single, marker='o', label='Single Thread')
    plt.plot(sizes, multi, marker='o', label='Multi Thread')
    plt.xlabel('Input Size')
    plt.ylabel('Time (ms)')
    plt.title('Merge Sort Time Comparison')
    plt.legend()
    plt.savefig(name, format='png', dpi=300)
    plt.show()

if __name__ == "__main__":
    single = []
    multi = []
    for size in range(200, 1200, 200):
        single.append(run(size, 0))
        multi.append(run(size, 1))
    plot(single, multi, [200, 400, 600, 800, 1000], "plot.png")
    