from collections import defaultdict


lines = []
branches = []
with open('raw', 'r') as f:
    lines = [li.strip() for li in f]
for li in lines[1:]:
    if len(li) == 0: continue
    extract_name = lambda it: it.split('+0x')[0]
    fr, to = map(extract_name, li.split())
    branches.append([fr, to])
print(f"Saved branches: {len(branches)}")


class Function:
    m_name = ""
    m_size = 0

    def __init__(self, name, size):
        self.m_name = name
        self.m_size = size

lines = []
syms = set()
functions = []
with open('order', 'r') as f:
    lines = [li.strip() for li in f]
    lines = [li for li in lines if len(li) > 0]
    lines = [li for li in lines if len(li.split()) == 8]
    for li in lines[2:]:
        _, st, sz, ty, _, _, _, name = li.split()
        if ty != "FUNC": continue
        sz = int(sz,0) if sz.startswith('0x') else int(sz)
        functions.append(Function(name, sz))
        syms.add(name)
print("Symbols = ", len(syms))


replace_if = lambda it: it if it in syms else "[unknown]"
branches = [(replace_if(it[0]), replace_if(it[1])) for it in branches]

check_if = lambda it: it != "[unknown]"
branches = [it for it in branches if check_if(it[0]) and check_if(it[1])]


call_graph = defaultdict(lambda: 0)
for b in branches:
    call_graph[b] += 1

call_graph = [(call_graph[key], key) for key in call_graph.keys()]
call_graph.sort(key = lambda it: -it[0])



class Cluster:

    def __init__(self, func, size):
        self.m_functions = [func]
        self.m_size = size

    def __str__(self):
        st = "" # f"Cluster: size = {self.m_size}\n"
        for f in self.m_functions:
            st += f + "\n"
        return st[:-1]

    def __lt__(self, rhs):
        return self.m_size > rhs.m_size

class ClusterEdge:
    def __init__(self, a : int, b : int, cost : int):
        self.cost = cost
        self.caller = a
        self.callee = b

    def __str__(self):
        return f"Edge: cost = {self.cost} Caller = {self.caller} Callee = {self.callee}"

class ClusterMap:
    
    # Init set of clusters
    def __init__(self, functions : list[Function]):
        global call_graph
        self.m_clusters = []
        self.m_edges = defaultdict(list)
        f2c = {}; count = 0
        for f in functions:
            self.m_clusters.append(Cluster(f.m_name, f.m_size))
            f2c[f.m_name] = count; count += 1
        
        for c in call_graph:
            cost = c[0]
            caller_id = f2c[c[1][0]]
            callee_id = f2c[c[1][1]]
            self.m_edges[callee_id].append(ClusterEdge(caller_id, callee_id, cost))

    # Merge clusters that has caller and callee functions
    def merge_cluster(self, edge):
        caller_id = edge.caller
        callee_id = edge.callee
        if caller_id == callee_id:
            self.remove_edge(edge)
            #remove edge
            return

        if self.m_clusters[caller_id].m_size + self.m_clusters[callee_id].m_size >= 0x1000 * 16:
            # remove edge
            self.remove_edge(edge)
            return

        self.m_clusters[caller_id].m_size += self.m_clusters[callee_id].m_size
        self.m_clusters[caller_id].m_functions += self.m_clusters[callee_id].m_functions

        self.merge_edges(caller_id, callee_id)

        self.m_clusters[callee_id] = None
        self.remove_edge(edge)
        self.replace_caller_to(callee_id, caller_id)


    def remove_edge(self, edge):
        callee = edge.callee
        if edge in self.m_edges[callee]:self.m_edges[callee].remove(edge)
        if len(self.m_edges[callee]) == 0: del self.m_edges[callee]

    def replace_caller_to(self, old_caller, new_caller):
        for k in self.m_edges.keys():
            for i, e in enumerate(self.m_edges[k]):
                if e.caller == old_caller:
                    self.m_edges[k][i].caller = new_caller

    def merge_edges(self, caller_id, callee_id):
        callee_edges = self.m_edges[callee_id]
        need_append = []
        for c_e in callee_edges:
            found = False
            for i, edge in enumerate(self.m_edges[caller_id]):
                if c_e.caller == edge.caller:
                    self.m_edges[caller_id][i].cost += c_e.cost
                    found = True
            if not found:
                c_e.callee = caller_id
                self.m_edges[caller_id].append(c_e)

        del self.m_edges[callee_id]

    def get_max_edge(self):
        if len(self.m_edges) == 0: return

        ret = None
        cost = 0
        for k in self.m_edges.keys():
            for edge in self.m_edges[k]:
                if cost < edge.cost:
                    ret = edge
                    cost = edge.cost

        return ret

    def print_clusters(self):
        for it in self.m_clusters:
            print(it)


cluster_map = ClusterMap(functions)


while True:
    e = cluster_map.get_max_edge()
    if not e: break
    cluster_map.merge_cluster(e)

#cluster_map.print_clusters()


final = list(filter(None, cluster_map.m_clusters))
final.sort()

for f in final:
    print(f)

