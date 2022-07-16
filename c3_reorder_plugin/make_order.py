def read_lines(fin):
    li = []
    with open(fin, 'r') as f:
        li = [l.strip() for l in f]
    return li

def parse_nm(fin):
    syms = [it.split()[-1] for it in read_lines(fin) if len(it.split()) == 4 and it.split()[-2] in ["t", "T"]]
    return [it for it in syms if '.cold' not in it]

for it in sorted(parse_nm('order')):
    print(it)