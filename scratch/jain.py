import os
import sys

def jain(file_name):
    it = os.popen('tshark -z conv,tcp -q -r ' + file_name, 'r')
    t = {}

    for line in it:
        l = line.split()
        try:
            if l[1] == '<->':
                t[l[0].split(':')[0]] = int(l[-3])
        except IndexError:
            pass

    n = sum(t.itervalues()) ** 2
    d = (len(t) * sum(map(lambda x: x ** 2, t.itervalues())))
    return len(t), n / float(d)

if __name__ == '__main__':
    for fn in sys.argv[1:]:
        n, j = jain(fn)
        print fn, n, j
