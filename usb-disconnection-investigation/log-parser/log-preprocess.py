import sys

EP_IN_PRINT = False

def main(argv):
    fn = argv[0]
    nfn = fn
    if fn.lower().endswith(".csv"):
        nfn = nfn[0:-4] + ".out.csv"
    elif fn.lower().endswith(".txt"):
        nfn = nfn[0:-4] + ".out.txt"

    with open(fn) as fin:
        with open(nfn, "w") as fout:
            data_remaining_out = 0
            data_remaining_in = 0
            outstr = ""
            instr = ""
            data_ignoring = False
            line = fin.readline()
            while line:
                line = line.strip()
                if line.startswith('#'):
                    line = fin.readline()
                    continue
                cols = line.split(',')
                datastr = cols[10].strip()
                datacols = datastr.split(' ')
                databytes = []
                try:
                    try:
                        for x in datacols:
                            databytes.append(int("0x" + x, 16))
                    except:
                        pass
                    ep = -1
                    try:
                        ep = int(cols[8].strip())
                    except:
                        pass
                    nline = cols[2] + "," + cols[3] + "," + cols[8] + "," + cols[9] + "," + cols[10]
                    datalen = 0
                    if ep <= 0:
                        if "CORRUPTED" not in nline:
                            fout.write(nline + '\n')
                        line = fin.readline()
                        continue
                    elif "OUT" in cols[9] and data_remaining_out <= 0:
                        datalen = databytes[3] + (databytes[2] << 8)
                        data_ignoring = False
                        data_remaining_out = datalen + 4 - len(databytes)
                        if data_remaining_out < 0:
                            nline += ", # warning: data_remaining = %d" % data_remaining_out
                        outstr = nline.strip()
                        if len(databytes) < 512:
                            data_remaining_out = 0
                        if data_remaining_out <= 0:
                            fout.write(outstr + '\n')
                        line = fin.readline()
                        continue
                    elif "IN" in cols[9] and data_remaining_in <= 0:
                        datalen = databytes[3] + (databytes[2] << 8)
                        if (databytes[0] == 0x0A or databytes[0] == 0x04) and (databytes[1] & 0x08) == 0x08 and len(databytes) >= 512:
                            nline += ", ..."
                            data_ignoring = True
                        else:
                            data_ignoring = False
                        data_remaining_in = datalen + 4 - len(databytes)
                        if data_remaining_in < 0:
                            nline += ", # warning: data_remaining = %d" % data_remaining_in
                        instr = nline.strip()
                        if len(databytes) < 512:
                            data_remaining_in = 0
                        if data_remaining_in <= 0 and EP_IN_PRINT:
                            fout.write(instr + '\n')
                        line = fin.readline()
                        continue
                    elif "OUT" in cols[9] and data_remaining_out > 0:
                        data_remaining_out -= len(databytes)
                        nline = " " + cols[10].strip()
                        if data_remaining_out < 0:
                            nline += ", # warning: data_remaining = %d" % data_remaining_out
                        outstr += nline
                        if data_remaining_out <= 0:
                            fout.write(outstr + '\n')
                        line = fin.readline()
                        continue
                    elif "IN" in cols[9] and data_remaining_in > 0:
                        data_remaining_in -= len(databytes)
                        nline = " " + cols[10].strip()
                        if data_remaining_in < 0:
                            nline += ", # warning: data_remaining = %d" % data_remaining_in
                        instr += nline
                        if data_remaining_in <= 0 and EP_IN_PRINT:
                            fout.write(instr + '\n')
                        line = fin.readline()
                        continue
                    else:
                        if "CORRUPTED" not in nline:
                            fout.write(nline + '\n')
                        line = fin.readline()
                        continue
                except:
                    line = fin.readline()
                    continue

# 3F 19 = 0x3F19 = 16153
# 512 * 31 + 285 = 16157

# 20 27 = 0x2027 = 8231
# 512 * 16 + 43  = 8235

if __name__ == "__main__":
    main(sys.argv[1:])
