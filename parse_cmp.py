import argparse
parser = argparse.ArgumentParser(description="Compare files")
parser.add_argument("-v", default=False, action="store_true", help="Enable addition statistic")
args = parser.parse_args()


def parse_blocks_from_lines(lines):
    lines = [x.strip() for x in lines] # delete space symbols in end and begin of lines
    lines = [x for x in lines if len(x) > 0] # delete all empty lines
    check = lambda s: s in ['g++', 'cc1plus'] # make lambda for filter traces
    def block():
        i = 0
        N = len(lines)
        while i < N:
            new_block = []
            if lines[i][-1] == ':': # new trace start
                need_emit = check(lines[i].split()[0])
                i += 1
                while i < N and lines[i][-1] != ':':
                    new_block.append(lines[i])
                    i += 1
                if need_emit: yield new_block

    return [it for it in block()] 


def read_blocks_from_file(filename):
    traces = parse_blocks_from_lines(open(filename).readlines()) #read traces from file
    for trace in traces: trace[:] = [record.partition(' ')[2] for record in trace] #erase addresses
    return [tuple(it) for it in traces]

# Read traces from the files
traces_A = read_blocks_from_file('1.txt')
traces_B = read_blocks_from_file('2.txt')
traces_all = traces_A + traces_B
# Make sets from it
A = set(it for it in traces_A)
B = set(it for it in traces_B)
# Find set intersection
C = A & B

# -v flag enable addition printin
if args.v:
    fi = '1.txt'
    print(f'Statistic for {fi} file')
    print(f'Total traces from file: {len(traces_A)}')
    print(f'Unique traces: {len(A)}')

    fi = '2.txt'
    print(f'Statistic for {fi} file')
    print(f'Total traces from file: {len(traces_B)}')
    print(f'Unique traces: {len(B)}')


total_traces = len(traces_all)
match = sum(1 for it in traces_all if it in C)

# Count match factor
print(f'{match/total_traces}')

