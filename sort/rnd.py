import random

random_array = [random.randint(1, 10000) for _ in range(2000)]

print(2000)
# 将数组转换成字符串并用逗号隔开每个元素
random_array_str = ' '.join(map(str, random_array))

print(random_array_str)