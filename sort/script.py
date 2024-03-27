import random
import subprocess


def random_array(n):
    rand_arr = [random.randint(1, 10000) for _ in range(n)]
    return ' '.join(map(str, rand_arr))

def run(size, flag):
    cpp_input = f"{size}\n{random_array(size)}"
    if (flag):
        process = subprocess.run("./MergesortMulti", shell=True, check=True, input=cpp_input, text=True)
    else:
        process = subprocess.run("./MergesortSingle", shell=True, check=True, input=cpp_input, text=True)

if __name__ == "__main__":
    size = random.randint(2000, 10000)
    multi = int(input("0 for single thread, 1 for multi-thread: "))
    for _ in range(10):
        run(size, multi)