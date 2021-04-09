import os
import os.path
import sys
import shlex
import time
import traceback
import itertools
from pprint import pprint
from subprocess import call, check_call, check_output, CalledProcessError, STDOUT
from multiprocessing import Pool, cpu_count
from burstmemoryestimators import print_burst_results
import json


def runex(args):
    try:
        t = time.time()
        A = args
        d = os.path.join(os.getenv("HOME"), 'results-IGOR')

        c = 'tcp-qfp ' + ' '.join(['--{0}={1}'.format(*it) for it in A.items()])

        for cmd_arg in ['RunTime', 'nNodes', 'mNodes', \
                        'R', 'R1', 'R2', \
                        'queueType', 'Q1Type', 'Q2Type', \
                        'ECN', 'OPD', 'DRR', 'adaptive', 'noQueues', \
                        'Quantum', 'Target', 'Interval']:
            if cmd_arg in A:
                d = os.path.join(d, cmd_arg.replace('queueType', 'QT').replace('Q1Type', 'Q1T'). \
                                            replace('Q2Type', 'Q2T').replace('RunTime', 'rt') \
                                            + '-' + A[cmd_arg].replace('00000', '0'))
                del A[cmd_arg]

        for cmd_arg in ['writeFlowMonitor', 'writeForPlot', 'printSockStats']:
            if cmd_arg in A:
                del A[cmd_arg]

        d = os.path.join(d, ''.join(['{0}+{1}-'.format(*it) for it in A.items()]).replace(' ', '-').replace('---', '-').replace('ns3::RedQueue::AdaptInterval', 'Ai') \
                                                                                 .replace('SFQHeadMode', 'SFQhm').replace('modeBytes', 'mb')).replace('maxBytes', 'MB')
        try:
            os.makedirs(d)
        except OSError:
            pass

        cmd = 'LD_LIBRARY_PATH=../build ../build/scratch/' + c + ' --pathOut=' + d

        print '%%%'
        print cmd
        print '%%%'

        out = check_output(cmd, shell = True, stderr = STDOUT)
        with open(d + '/tcp-qfp.out', 'w') as f:
            f.write(out)
        with open(d + '/case.json', 'w') as j:
            j.write(json.dumps(args) + '\n')
        try:
            check_output('python ../src/flow-monitor/examples/flowmon-parse-results.py ' +
                         d + '/tcp-qfp.flowmon > ' + d + '/tcp-qfp-flowmon.out',
                         shell = True, stderr = STDOUT, cwd = d)
        except:
            pass

        # Print the burst stats
        print_burst_results(d)

    except:
        return "Process for %s threw an exception\n%s" % (repr(args), traceback.format_exc())
    return "Process for %s returned in %ds" % (repr(args), time.time() - t)

if __name__ == '__main__':
    R2 = 4
    expts = []
    n = 50
    defaults = {'Q1Type':'CoDel',
                'Q2Type':'CoDel',
                'maxBytes':'0',
                'writeFlowMonitor':'1',
                'nNodes':'100',
                'mNodes':'45',
                'RunTime':'60',
                'ns3::RedQueue::AdaptInterval':'0.1s',
                'writeForPlot':'1',
                'printSockStats':'1',
                'ECN':'0',
                'adaptive':'0',
                'DRR':'0',
                'OPD':'0',
                'noQueues':'0'
                }
    '''
User Arguments:
    --RunTime: Number of seconds to run
    --pathOut: Path to save results from --writeForPlot/--writePcap/--writeFlowMonitor
    --writeFlowMonitor: <0/1> to enable Flow Monitor and write their results
    --maxBytes: Total number of bytes for application to send
    --queueType: Set Queue type to CoDel, DropTail, RED, or SFQ
    --modeBytes: Set RED Queue mode to Packets <0> or bytes <1>
    --stack: Set TCP stack to NSC <0> or linux-2.6.26 <1> (warning, linux stack is really slow in the sim)
    --modeGentle: Set RED Queue mode to standard <0> or gentle <1>
    --maxPackets: Max Packets allowed in the queue
    --Quantum: SFQ and fq_codel quantum
    --nNodes: Number of client nodes
    --mNodes: Number of low latency client noodes
    --R: Bottleneck rate
    --R1: Low latency node edge link bottleneck rate
    --Q1Type: Set Queue type to DropTail or RED
    --Q1redMinTh: RED queue minimum threshold (packets)
    --Q1redMaxTh: RED queue maximum threshold (packets)
    --Q1maxPackets: Max Packets allowed in the queue
    --R2: High latency node edge link bottleneck rate
    --Q2Type: Set Queue type to DropTail or RED
    --Q2redMinTh: RED queue minimum threshold (packets)
    --Q2redMaxTh: RED queue maximum threshold (packets)
    --Q2maxPackets: Max Packets allowed in the queue
    --redMinTh: RED queue minimum threshold (packets)
    --redMaxTh: RED queue maximum threshold (packets)
    --SFQHeadMode: New SFQ flows go to the head
    --Interval: CoDel algorithm interval
    --Target: CoDel algorithm target queue delay
    --ECN: Use ECN capable Tcp
    --OPD: RED Uses Oldest Packet Drop
    --DRR: RED Uses Deficit Round Robin
    --noQueues: Number of queues for RED with DRR
    --wq1: weight of RED queue 1
    --wq2: weight of RED queue 2
    --wq3: weight of RED queue 3
    --wq4: weight of RED queue 4
    --wq5: weight of RED queue 5
    --wq6: weight of RED queue 6
    --wq7: weight of RED queue 7
    --wq8: weight of RED queue 8
    --adaptive: Use adaptive RED
    --appsPerIf: No apps per if
    '''
    cases = itertools.product([100.0, 10.0],
                              [1.0, 10.0],
        # (1.+(1.*r)/n for r in range(0,n+1)),
                              iter([{'queueType':'RED',
                                     'Q1Type':'RED',
                                     'Q2Type':'RED',
                                     'adaptive':'1',
                                     'noQueues':'8',
                                     'modeBytes':'1',
                                     'DRR': '1'},
                                    {'queueType':'fq_codel',
                                     'Quantum':'4500',
                                     'Q1Type':'CoDel',
                                     'Q2Type':'CoDel'},
                                    {'queueType':'RED',
                                     'Q1Type':'RED',
                                     'Q2Type':'RED',
                                     'adaptive':'1',
                                     'noQueues':'8',
                                     'modeBytes':'1',
                                     'DRR': '1',
                                     'ECN':'1'},
                                    {'queueType':'fq_codel',
                                     'Quantum':'1500',
                                     'Q1Type':'CoDel',
                                     'Q2Type':'CoDel'},
                                    {'queueType':'RED',
                                     'Q1Type':'RED',
                                     'Q2Type':'RED',
                                     'adaptive':'1',
                                     'noQueues':'8',
                                     'modeBytes':'1',
                                     'DRR': '1',
                                     'OPD': '1',
                                     'ECN': '1'},
                                    {'queueType':'fq_codel',
                                     'Quantum':'9000',
                                     'Q1Type':'CoDel',
                                     'Q2Type':'CoDel'},
                                    {'queueType':'RED',
                                     'Q1Type':'RED',
                                     'Q2Type':'RED',
                                     'adaptive':'1',
                                     'noQueues':'8',
                                     'modeBytes':'1',
                                     'DRR': '1',
                                     'OPD': '1'},
                                    {'queueType':'DropTail',
                                     'maxPackets':'1000'},
                                    {'queueType':'fq_codel',
                                     'Quantum':'256',
                                     'Q1Type':'CoDel',
                                     'Q2Type':'CoDel'},
                                    ]),
                              iter([1]))
    ncores = cpu_count()
    # sys.exit(0)
    pool = Pool(processes = ncores)

    expts

    for R, R1, Q, run in cases:
        print R, R1, Q, run
        D = dict(defaults)
        D.update(Q)
        D.update({'R':'%fMbps' % R,
                  'R1':'%fMbps' % R1,
                  'R2':'%fMbps' % R2,
                  'RngRun':'%d' % run})
        expts.append(D)
        # Process ncores processes before processing others
        if len(expts) == ncores:
            print "There are", len(expts), "cases, run time should be about", 21.*len(expts) / (ncores * 60.), 'minutes'
            result = pool.map(runex, expts)
            for o in result:
                print o
            del expts[:]


