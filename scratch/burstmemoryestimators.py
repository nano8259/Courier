import os
import os.path
import sys
from scapy.all import *
import numpy as np
import matplotlib.pyplot as plt
import itertools
import math
import json
from jain import jain

def burstest(file_name, tau, time = False):
    packets = rdpcap(file_name)
    reader = ((p.time, PPP(str(p))) for p in iter(packets))
    tcp_pkt = ((time, pkt) for time, pkt in reader \
                 if TCP in pkt and \
                 (pkt[TCP].dport >= 5000 and pkt[TCP].dport <= 5100))
    last_time_stamp = None
    time_delta = 0
    last_time_delta = 0
    deviation = 0
    average = 0
    last_average = 0
    ssq = 0
    standard_deviation = 1
    last_standard_deviation = 1
    memory = 0
    res = []
    w = tau
    for time_stamp, pkt in tcp_pkt:

        if last_time_stamp is not None:
            time_delta = time_stamp - last_time_stamp

            if time:
                w = 1 - math.exp(-time_delta / tau)

            deviation = time_delta - average

            average = average + w * deviation

            ssq = (1 - w) * ssq + deviation * (time_delta - average)

            standard_deviation = math.sqrt((ssq * w * (2 - w)) / (2 * (1 - w)))

            burst = (standard_deviation - average) / (standard_deviation + average)

            memory = (1 - w) * memory + w * (time_delta - average) * \
                (last_time_delta - last_average) / (standard_deviation * last_standard_deviation)

            res.append((time_stamp, time_delta, average, standard_deviation, burst, memory))

        last_time_stamp = time_stamp
        last_time_delta = time_delta
        last_average = average
        last_standard_deviation = standard_deviation

    res = np.array(res)
    deltas = res[:, 1]
    standard_deviation_deltas = np.std(deltas)
    mean_deltas = np.mean(deltas)

    time_i = deltas[:-1]
    last_time_i = deltas[1:]
    mean_time_i = np.mean(time_i)
    mean_last_time_i = np.mean(last_time_i)

    best = np.mean(res[:, 4])
    burst = (standard_deviation_deltas - mean_deltas) / (standard_deviation_deltas + mean_deltas)

    memory = np.sum((time_i - mean_time_i) * (last_time_i - mean_last_time_i)) \
                      / (np.std(time_i) * np.std(last_time_i) * time_i.shape[0])

    return res[:, 0], res[:, 1], res[:, 2], res[:, 3], res[:, 4], res[:, 5], best, burst, memory

def find_pcap_files(parent_dir):
    pcapFiles = [i for i in os.listdir(parent_dir) if i.endswith('.pcap') and 'Left' in i]
    # sort by interface ID
    pcapFiles.sort(key = lambda s: int(s.split('-')[-1].split('.')[0]))
    # last one is the output, don't want to plot that, but we want to extract jainIndex from it
    jainPcap = pcapFiles[-1]
    pcapFiles = pcapFiles[:-1]
    nrows = len(pcapFiles)
    return pcapFiles, nrows, jainPcap

def print_burst_results(parent_dir):
    pcapFiles, nrows, jainPcap = find_pcap_files(parent_dir)

    noFlows, jainIndex = jain(os.path.join(parent_dir, jainPcap))
    file_name = os.path.join(parent_dir, 'jainIndex')
    file = open(file_name, 'w')
    file.write("noFlows\tJainIndex\n")
    file.write("{0}\t{1}\n".format(noFlows, jainIndex))
    file.close()

    for pcapFile, iter in zip(pcapFiles, range(0, nrows + 1)):
        try:
            file_name = os.path.join(parent_dir, pcapFile.replace('.pcap', '.plotme'))
            file = open(file_name, 'w')
            file.write("Time\tDeltaTime\tAverageTime\tStandardDeviation\tBurst\tMemory\n")

            pcapFile = os.path.join(parent_dir, pcapFile)
            time_i, time_delta_i, average_i, standard_deviation_i, burst_i, mem_i, best, burst, mem = burstest(pcapFile, 0.1)

            np.set_printoptions(precision = 6)
            for _time, _time_delta, _average, _standard_deviation, _burst, _mem in \
                    zip(time_i, time_delta_i, average_i, standard_deviation_i, burst_i, mem_i):
                file.write("{0}\t{1}\t{2}\t{3}\t{4}\t{5}\n".format(\
                            _time, _time_delta, _average, _standard_deviation, _burst, _mem))

            file.write("\nOverall\n")
            file.write("Best\tBurst\tMemory\n")
            file.write("{0}\t{1}\t{2}\n".format(best, burst, mem))
            file.close()
        except:
            print(sys.exc_info())
            exit (-1)

if __name__ == '__main__':

    print_burst_results(sys.argv[1])

    case = json.load(open(os.path.join(sys.argv[1], 'case.json'), 'r'))
    print(case)

    pcapFiles, nrows, jainPcap = find_pcap_files(parent_dir)

    for pcapFile, iter in zip(pcapFiles, range(0, nrows + 1)):
        print(iter, pcapFile)
        pcapFile = os.path.join(sys.argv[1], pcapFile)
        time_i, time_delta_i, average_i, standard_deviation_i, burst_i, mem_i, best, burst, mem = burstest(pcapFile, 0.1)
        plt.subplots_adjust(wspace = 0.15, hspace = 0.15, left = 0.05, right = 0.97, top = 0.97, bottom = 0.05)
        plt.subplot(nrows, 3, 3 * iter + 1)
        plt.grid(True)
        plt.tick_params(labelsize = 'x-small')
        plt.locator_params(axis = 'y', nbins = 4, prune = 'lower')
        # plt.ylim(0,0.5)

        plt.plot(time_i, time_delta_i, 'b.')  # , time_i, average_i, 'r.', time_i, standard_deviation_i, 'g.')
        plt.subplot(nrows, 3, 3 * iter + 2)
        plt.grid(True)
        plt.tick_params(labelsize = 'x-small')
        plt.ylim(-1, 1)
        plt.xlim(0, 64)
        plt.plot(time_i, burst_i, 'r-', time_i, mem_i, 'g-')
        plt.subplot(nrows, 3, 3 * iter + 3)
        plt.grid(True)
        plt.tick_params(labelsize = 'x-small')
        plt.xlim(0, 0.1)
        x = np.sort(time_delta_i)
        n = len(x)
        x2 = np.repeat(x, 2)
        y2 = np.hstack([0.0, np.repeat(np.arange(1, n) / float(n), 2), 1.0])
        plt.plot(x2, y2)
        print(best, burst, mem)
    plt.show()
