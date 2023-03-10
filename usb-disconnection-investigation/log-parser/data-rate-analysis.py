import sys, math

class PktSummary(object):

    def __init__(self, microseconds, datalen):
        self.microseconds = microseconds
        self.datalen = datalen

def sum_data(arr):
    x = 0
    for i in arr:
        x += i.datalen
    return x

def prune(arr):
    us = arr[-1].microseconds
    while True:
        x = arr[0].microseconds
        if us - x > 1000 * 1000:
            arr = arr[1:]
        else:
            break
    return arr

def main(argv):
    fn = argv[0]
    nfn = fn
    if fn.lower().endswith(".out.csv"):
        nfn = nfn[0:-8] + ".datarate.csv"
    elif fn.lower().endswith(".out.txt"):
        nfn = nfn[0:-8] + ".datarate.txt"

    with open(fn) as fin:
        with open(nfn, "w") as fout:
            history = []
            line = fin.readline()
            lastsec = -1
            while line:
                line = line.strip()
                cols = line.split(',')
                tstr = cols[1]
                minuteseconds = tstr.split(':')
                subseconds = minuteseconds[1].split('.')
                microseconds = (int(subseconds[0]) * 1000 * 1000) + (int(subseconds[1]) * 1000) + int(subseconds[2]) + (int(minuteseconds[0]) * 60 * 1000 * 1000)
                databytes = cols[4].split(' ')
                datalen = len(databytes)
                history.append(PktSummary(microseconds, datalen))
                sec4 = math.floor(microseconds / 1000 / 250)
                if sec4 != lastsec:
                    lastsec = sec4
                    history = prune(history)
                    x = sum_data(history)
                    outstr = "%s, %0.2f, %d" % (cols[0], microseconds / 1000 / 1000, x)
                    print(outstr)
                    fout.write(outstr + '\n')

                line = fin.readline()

if __name__ == "__main__":
    main(sys.argv[1:])
