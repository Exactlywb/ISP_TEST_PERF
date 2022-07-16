def read_lines(fin):
    li = []
    with open(fin, 'r') as f:
        li = [l.strip() for l in f]
    return li

def parse_nm(fin):
    syms = [it.split()[-1] for it in read_lines(fin) if len(it.split()) == 4 and it.split()[-2] in ["t", "T"]]
    return [it for it in syms if '.cold' not in it]

def parse_pass(fin):
    return [ it.split(':')[-1] for it in read_lines(fin) if it.startswith('AsmName')]


nm = parse_nm('order')
ps = parse_pass('plugin_dump.txt')



nm_set = set(nm)
ps_set = set(ps)

print(f'NM: list size = {len(nm)}; set size = {len(nm_set)}')
print(f'PS: list size = {len(ps)}; set size = {len(ps_set)}')

dif = nm_set - ps_set# - nm_set
print(len(dif))
#for it in dif:
#    print(it)

