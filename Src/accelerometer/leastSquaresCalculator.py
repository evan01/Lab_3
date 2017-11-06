from numpy import genfromtxt
import numpy as np

'''
TODO did you run bad experiments? the 6 board setups aren't optimal for least squares???
'''
def readCsv(filename):
    #read
    data = genfromtxt('calibrationData/xDown.csv', delimiter=',',filling_values=[0.0])

    #remove nans
    data = data[~np.isnan(data).any(axis=1)]

    #add 1s to inputs
    return np.hstack((data,np.ones((data.shape[0],1), dtype=np.double)))

# This python file will read all of the calibration data, and then run least squares approximation
def main():
    print("Reading all of the files...")

    # These are the 12 constants that we want to find
    a11, a12, a13, a10 = 0.0, 0.0, 0.0, 0.0
    a21, a22, a23, a20 = 0.0, 0.0, 0.0, 0.0
    a31, a32, a33, a30 = 0.0, 0.0, 0.0, 0.0

    # Solve Ax=y for A.
    # Open all of the csvfiles, store into array
    # xDown = readCsv('calibrationData/xDown.csv')
    # xUp = readCsv('calibrationData/xUp.csv')
    # yDown = readCsv('calibrationData/yDown.csv')
    # yUp = readCsv('calibrationData/yUp.csv')
    # zDown = readCsv('calibrationData/zDown.csv')
    zUp = readCsv('calibrationData/zUp.csv')

    #todo, do these need to be normalized?
    # Add the coresponding B entry for each element of data collected in the csv file
    # xDownSolution = np.tile(np.array([-1024, 0, 0],dtype=np.double),(xDown.shape[0],1))
    # xUpSolution = np.tile(np.array([1024, 0, 0],dtype=np.double),(xUp.shape[0],1))
    # yDownSolution = np.tile(np.array([0, -1024, 0],dtype=np.double),(yDown.shape[0],1))
    # yUpSolution = np.tile(np.array([0, 1024, 0],dtype=np.double),(yUp.shape[0],1))
    # zDownSolution = np.tile(np.array([0, 0, -1024],dtype=np.double),(zDown.shape[0],1))
    zUpSolution = np.tile(np.array([0.01, 0.01, 1024],dtype=np.double),(zUp.shape[0],1))

    #Combine All the solutions
    # x = np.vstack((xDown,yDown,zDown,xUp,yUp,zUp)) # now a N X 4
    # y = np.vstack((xDownSolution,yDownSolution,zDownSolution,xUpSolution,yUpSolution,zUpSolution)) #Nx3

    x = zUp
    y = zUpSolution
    A = np.linalg.lstsq(x,y)[0]
    dot = np.dot(x.T,x)

    #Try it?
    print('done')
    print(A)
    print("\n")
    print(np.dot(np.array(),A)) #Should equal -1024,0,0

main()
