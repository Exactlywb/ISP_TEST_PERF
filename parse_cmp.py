def count_empty_num_lines (lines):
    n = 0
    for i in lines:
        if (i == "\n"):
            n = n + 1;
    return n

def parse_blocks (lines):
    res = []

    numBlocks = count_empty_num_lines (lines) - 1
    lines[:] = [x for x in lines if x.strip()] # delete all empty lines
    lines[:] = [x.strip () for x in lines] # delete space symbols in end and begin of lines
    i = 0
    blockStartNum = 0
    while i < numBlocks:
        if (lines [blockStartNum] [len (lines [blockStartNum]) - 1] == ":"): # new trace start
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

def cmp_blocks (first_blocks, second_blocks):
    colors = [] # 0 if there is no the same block in second list
                # 1 if there is
    i = 0
    while i < len (first_blocks):
        colors.append (first_blocks [i] in second_blocks)
        i = i + 1

    return colors

first_lines = open ("1.txt").readlines ()
second_lines = open ("2.txt").readlines()

first_blocks = parse_blocks (first_lines)
second_blocks = parse_blocks (second_lines)

first_blocks_without_addresses = erase_addresses (first_blocks)
second_blocks_without_addresses = erase_addresses (second_blocks)

colors = cmp_blocks (first_blocks_without_addresses, second_blocks_without_addresses)

match = 0
for i in colors:
    match = match + i

print (match / len (first_blocks))
