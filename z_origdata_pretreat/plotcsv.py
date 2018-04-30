#Coding = 'utf-8'

import xlwt
import numpy as np
import math
import matplotlib.pyplot as plt
import os

def plotmatrix(fmtr,fname='imgtoshow.jpg',imgpath = './outimg/',showimg = False):
    fmtr = fmtr.reshape([-1,20])
    mean_matrix = np.mean(fmtr,1)
    mean_matrix = mean_matrix.reshape([len(mean_matrix),1])
    if max(mean_matrix)>500:
        maxlim = max(mean_matrix) + 100
    else:
        maxlim = 500
    if min(mean_matrix)<200:
        minlim = min(mean_matrix) - 100
    else:
        minlim = 200
    plt.plot(range(len(mean_matrix)),mean_matrix,'ro')
    plt.xlabel('time')
    plt.ylabel('value')
    plt.ylim(minlim,maxlim)
    plt.title(fname)
    if not os.path.exists(imgpath):
        os.mkdir(imgpath)
    plt.savefig(imgpath+fname+".jpg")
    if showimg:
        plt.show()
    plt.clf()
    print(imgpath,fname,'\tsave successful')

def see1img(fname):
    mtr = np.loadtxt(open('./outcsv/'+str(fname) + '.csv','rb'),delimiter = ',',skiprows=0)
    plotmatrix(mtr)

def changename(dirpath = './outcsv/'):
    flist = os.listdir(dirpath)
    for fname in flist:
        if fname[0] == 'n':
            fnum = int(fname[1:].split('.')[0])
            while os.path.exists(dirpath+ str(fnum) + '.csv'):
                fnum = fnum + 1
            os.rename(dirpath + fname,dirpath + str(fnum) + '.csv')


if __name__ == '__main__':
    csvpath = './outcsv/'

    plt.close('all')
    print("ok,let us do it")
    filelist = os.listdir(csvpath)
    print('totle files:',len(filelist))
    for fname in filelist:
        fmtr = np.loadtxt(open(csvpath + fname,'rb'),delimiter = ',',skiprows=0)
        plotmatrix(fmtr,fname)
    print('job done')