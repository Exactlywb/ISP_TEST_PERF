
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

first_blocks_without_addresses = erase_addresses (first_blocks)     #Aorig*
second_blocks_without_addresses = erase_addresses (second_blocks)   #Borig*

unique_first_blocks = set(tuple(it) for it in first_blocks_without_addresses)   #A*
unique_second_blocks = set(tuple(it) for it in second_blocks_without_addresses) #B*

concatenate_orig = first_blocks_without_addresses + second_blocks_without_addresses #Corig* = Aorig* U Borig*
unique_concatenate_blocks = unique_first_blocks & unique_second_blocks  #C = A* ^ B*

print (len (unique_concatenate_blocks))

res = list(it for it in concatenate_orig if tuple(it) in unique_concatenate_blocks)

print (len (res))
print (len (concatenate_orig))

print (len (res) / len(concatenate_orig))

# print (len (unique_concatenate_blocks))
# print (len (concatenate_orig))

# match = cmp_blocks (concatenate_orig, list (unique_concatenate_blocks))

# print (match / len (concatenate_orig))

# for i in unique_concatenate_blocks:
#     for j in i:
#         print (j);

# for i in concatenate_orig:
#     for j in i:
#         print (j);

