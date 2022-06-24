
def count_empty_num_lines (lines):
    n = 0
    for i in lines:
        if (i == "\n"):
            n = n + 1;
    return n

def parse_blocks (lines):
    res = []

    numBlocks = count_empty_num_lines (lines) - 1
    lines = [x for x in lines if x.strip()] # delete all empty lines
    lines = [x.strip () for x in lines] # delete space symbols in end and begin of lines
    i = 0
    blockStartNum = 0
    while i < numBlocks:
        if (lines [blockStartNum] [len (lines [blockStartNum]) - 1] == ":"): # new trace start
            if ("g++" not in lines [blockStartNum] and "cc1plus" not in lines [blockStartNum]):
                blockStartNum = blockStartNum + 1
                while (lines [blockStartNum] [len (lines [blockStartNum]) - 1] != ':'):
                    blockStartNum = blockStartNum + 1
                i = i + 1
                continue

            blockRes = []
            blockStartNum = blockStartNum + 1
            while (lines [blockStartNum] [len (lines [blockStartNum]) - 1] != ':'):
                blockRes.append (lines [blockStartNum])
                blockStartNum = blockStartNum + 1
            res.append (blockRes)
        i = i + 1

    return res

def erase_addresses (blocks):
    for block in blocks:
        block [:] = [x.partition (' ')[2] for x in block]

    return blocks

def cmp_blocks (a, b):
    return sum(1 if it in b else 0 for it in a)

def unique_blocks (blocks):
    count = 0
    lst = []
    for i in blocks:
        if i not in lst:
            count = count + 1
            lst.append (i)
    return lst

first_lines = open ("1.txt").readlines ()
second_lines = open ("2.txt").readlines()

first_blocks = parse_blocks (first_lines)
second_blocks = parse_blocks (second_lines)

first_blocks_without_addresses = erase_addresses (first_blocks)
second_blocks_without_addresses = erase_addresses (second_blocks)

unique_first_blocks = set(tuple(it) for it in first_blocks_without_addresses)
unique_second_blocks = set(tuple(it) for it in second_blocks_without_addresses)

print (len (unique_first_blocks))
print (len (unique_second_blocks))

match = cmp_blocks (unique_first_blocks, unique_second_blocks)

print (match / len (unique_first_blocks))

# for i in unique_first_blocks:
#     for j in i:
#         print (j);
