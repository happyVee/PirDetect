#Coding = 'utf-8'

from pylab import *
import xlwt


def byte2int(cont):
	lint = []
	lnum = range(0,len(cont),2)
	for i in lnum:
		if cont[i]!= 0xFF and abs(cont[i] - 0x80) < 10:
			lint.append( (cont[i]&0b11111111)*256 + cont[i+1] )
	print('共有 ' + str(len(lint)) + '个数据')
	return lint

def saveTxt(lint):
	strlint = str(lint)[1:-1]
	with open('ans.txt','w') as f:
		f.write(strlint)
	print('保存txt成功')

def saveExcel(lint):
	wb = xlwt.Workbook()
	ws = wb.add_sheet('Sheet1')

	for colVal in range(len(lint)):
		value = lint[colVal]
		if colVal<10000:
			ws.write(colVal,0,colVal)
		ws.write(int(colVal%10000),int(colVal/10000)+1,value)

	wb.save('ans.xls')
	print('保存excel成功')

if __name__ == '__main__':
	with open('c.bin','rb') as f:
		cont = f.read()
	lint = byte2int(cont)
	saveTxt(lint)
	saveExcel(lint)


