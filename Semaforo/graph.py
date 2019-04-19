#!/usr/bin/python


import argparse
import math
import numpy as np
import matplotlib.pyplot as plt



parser = argparse.ArgumentParser(description='Create efficiency graphs')

required = parser.add_argument_group('required arguments')
required.add_argument('-M', metavar='NUBER-OF-VALUES', type=int, required=True,
                      help='Number of values that will be produced (and consumed) by the code.')
required.add_argument('-b', '--bufferSize', metavar='BUFFER-SIZE', type=int, nargs='+', required=True,
                      help='Size of the shared memory buffer that will store numbers to be consumed. \n'
                           '\tATTENTION! All buffer sizes will need to have the same combinations of'
                           'productor and consumer thread numbers testes')
required.add_argument('-i', '--inputFileName', type=str, required=True,
                      help='Name of the input file. Should be formed by the main file.')

parser.add_argument('-o', '--outputFileRoot', type=str,
                    default="latencyGraph",
                    help='Root of the name of the output graph.')

args = parser.parse_args()

# Main function
if __name__ == "__main__":
    M = args.M
    bf = args.bufferSize
    bf.sort()

    outRoot = "latencyGraph"
    if args.outputFileRoot != "":
        outRoot = args.outputFileRoot

    colors = [None, 'r', 'g', 'm', 'b', 'y', 'c']

    # Variables to be filled
    nThrVec = [ [] for i in bf ]
    timesDict = [ {} for i in bf ]

    # Reading from file
    infile = open(args.inputFileName, "r")

    lines = infile.readlines()
    for line in lines:
        words = line.split(' ')

        try:
            thisM = int(words[0])
        except:
            continue

        if thisM == M:  # Same N as desired
            thisBS = int(words[1])

            if thisBS in bf:
                bfIdx = bf.index(thisBS)
                K_p = int(words[2])
                K_c = int(words[3])
                t = float(words[4])

                K = (K_p, K_c)

                # Add the K value to the list, if it has not
                # been done yet, and t to dictionary
                if K not in nThrVec[bfIdx]:
                    nThrVec[bfIdx] += [K]
                    timesDict[bfIdx].update( { K: [t] } )
                else:
                    timesDict[bfIdx][K] += [t]

    nThrVec[bfIdx].sort()

    nThrArr = np.array(nThrVec)

    # Create lists of execution time and uncertainty per K
    timeVec = [ [] for i in bf ]
    errTVec = [ [] for i in bf ]
    text_values = []
    x_values = None
    bf_str = ""

    fig, ax = plt.subplots(figsize=(10, 6))
    fig.suptitle("Latency of the producer-consumer algorithm",
                 y=0.96, fontweight='bold', fontsize=17)

    for bfIdx in range(len(bf)):
        for i in range(len(nThrVec[bfIdx])):
            listOfTimes = timesDict[bfIdx][nThrVec[bfIdx][i]]
            timeVec[bfIdx] += [sum(listOfTimes) / len(listOfTimes)]

            tmp = [(x - timeVec[bfIdx][i]) ** 2 for x in listOfTimes]
            errTVec[bfIdx] += [math.sqrt(sum(tmp) / len(listOfTimes))]

            if bfIdx == 0:
                text_values += [str(nThrVec[bfIdx][i])]

        if bfIdx == 0:
            x_values = np.arange(1, len(text_values) + 1, 1)

        lab = "Memory size = %i" % (bf[bfIdx])

        plt.errorbar(x_values, timeVec[bfIdx], errTVec[bfIdx], color=colors[bfIdx],
                     ecolor='black', elinewidth=.5,
                     marker='.', mec='black', mfc='black', ms=10, label=lab)

        bf_str += "%i-" % bf[bfIdx]

    bf_str = bf_str[:-1]

    plt.xlabel('Number of threads (Np x Nc)')
    plt.xticks(x_values, text_values)
    plt.ylabel('Execution time (s)')
    plt.legend()

    plt.subplots_adjust(left=0.1, right=0.94, top=0.89)
    outputFileName = "%s_%ielements_%smemorySpaces.pdf" % (outRoot, M, bf_str)
    fig.savefig(outputFileName)
