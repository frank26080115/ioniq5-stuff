import sys

def main(argv):
    fn = argv[0]
    nfn = fn
    if fn.lower().endswith(".csv"):
        nfn = nfn[0:-4] + ".out.csv"
    elif fn.lower().endswith(".txt"):
        nfn = nfn[0:-4] + ".out.txt"

    with open(fn) as fin:
        with open(nfn, "w") as fout:
            line = fin.readline()
            while line:
                if line.startswith('#'):
                    line = fin.readline()
                    continue
                cols = line.split(',')
                datastr = cols[10]
                datacols = datastr.split(' ')
                databytes = []
                for x in datacols:
                    databytes.append(int("0x" + x, 16))
                nline = cols[2] + "," + cols[3] + "," + cols[4] + "," + cols[8] + "," + cols[9] + "," + cols[10]
                line = fin.readline()
if __name__ == "__main__":
    main(sys.argv[1:])