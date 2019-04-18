#!/usr/bin/python


import argparse
import math
import matplotlib.pyplot as plt
# from matplotlib import rc

# rc('text', usetex=True)

parser = argparse.ArgumentParser(description='Create efficiency graphs')

required = parser.add_argument_group('required arguments')
required.add_argument('-N', metavar='NUBER-OF-VALUES', type=int, required=True,
                      help='Number of values that were summed together by the code.')
required.add_argument('-i', '--inputFileName', type=str, required=True,
                      help='Name of the input file. Should be formed by the main file.')

parser.add_argument('-o', '--outputFileRoot', type=str,
                    default="latencyGraph",
                    help='Root of the name of the output graph.')

args = parser.parse_args()


# Main function
if __name__ == "__main__":
    N = args.N

    outRoot = "latencyGraph"
    if args.outputFileRoot != "":
        outRoot = args.outputFileRoot

    # Variables to be filled
    nThrVec = []
    timesDict = {}

    # Reading from file
    infile = open(args.inputFileName, "r")

    lines = infile.readlines()
    for line in lines:
        words = line.split(' ')

        try:
            thisN = int(words[0])
        except:
            continue

        if thisN == N:  # Same N as desired
            K = int(words[1])
            t = float(words[2])

            # Add the K value to the list, if it has not
            # been done yet, and t to dictionary
            if K not in nThrVec:
                nThrVec += [K]
                timesDict.update( { K: [t] } )
            else:
                timesDict[K] += [t]

    nThrVec.sort()

    # Create lists of execution time and uncertainty per K
    timeVec = []
    errTVec = []
    for i in range(len(nThrVec)):
        listOfTimes = timesDict[nThrVec[i]]
        timeVec += [ sum(listOfTimes)/len(listOfTimes) ]

        tmp = [ (x - timeVec[i])**2 for x in listOfTimes]
        errTVec += [ math.sqrt( sum( tmp )/len(listOfTimes) ) ]

    fig, ax = plt.subplots(figsize=(10, 6))
    fig.suptitle("Latency of the sum algorithm",
                 y=0.96, fontweight='bold', fontsize=17)
    # fig.suptitle(r"Lat\{^}encia do somador",
    #              fontweight='bold', fontsize=15)
    plt.errorbar(nThrVec, timeVec, errTVec, ecolor='black', elinewidth=.5,
                 marker='.', mec='black', mfc='black', ms=10)
    plt.xlabel('Number of threads')
    ax.set_xscale('log', basex=2)
    plt.ylabel('Execution time (s)')
    # plt.xlabel(r'N\'{u}mero de threads')
    # plt.ylabel(r'Tempo de execu\c{c}\~{a}o (s)')
    plt.subplots_adjust(left=0.1, right=0.94, top=0.89)
    outputFileName = "%s_%ielements.pdf"%(outRoot, N)
    fig.savefig(outputFileName)