import matplotlib.pyplot as plt
from collections import defaultdict

def get_mean(li):
    li.sort()
    return li[len(li)//2]

def make_mean(xx, yy):
    x = xx.copy()
    y = yy.copy()

    x = [int(k) for k in y.keys()]
    y = [get_mean(y[k]) for k in y.keys()]

    return x, y

def get_avg(li):
    return sum(li)/len(li)

def make_avg(xx, yy):
    x = xx.copy()
    y = yy.copy()

    x = [int(k) for k in y.keys()]
    y = [get_avg(y[k]) for k in y.keys()]

    return x, y


x = []
y = defaultdict(list)
prev = 0
with open('report', 'r') as f:
    for li in f:
        li = li.strip()
        if li.startswith('period'):
            prev = (int(li.split()[-1]))
            x.append(prev)
        else:
            y[prev].append(100 * float(li))


xx, yy = make_mean(x,y)
plt.scatter(xx, yy, color = 'red', label = 'Median')
xx, yy = make_avg(x,y)
plt.scatter(xx, yy, color = 'blue', label = 'Average')
plt.xlabel('Period')
plt.ylabel('Match, %')
plt.legend(['Median', 'Average'])
plt.grid()
plt.show()
