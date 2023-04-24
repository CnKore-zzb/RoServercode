import wx
from wx import adv
import os
import string
import threading
import time
import math
from time import sleep
from threading import Thread
import paramiko
import xml.dom.minidom
import xlrd as xl
import lupa
from lupa import LuaRuntime
import csv

class ExamplePanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        hSizer = wx.BoxSizer(wx.HORIZONTAL)
        grid = wx.GridBagSizer(hgap=8,vgap=8)
        
        self.logger = wx.TextCtrl(self, size=(1000,1000*0.618), style=wx.TE_MULTILINE | wx.TE_READONLY)
        self.logger.SetEditable(False)
        hSizer.Add(self.logger, 0, wx.ALL, 6)

        btlen = 100

        grid.Add(wx.StaticText(self, label='Compare File:', size=(btlen,-1)), pos=(1,0))
        self.compFileText = wx.TextCtrl(self, size = (btlen, -1), style=wx.TE_READONLY)
        grid.Add(self.compFileText, pos=(2,0))
        self.selectCompFileBt = wx.Button(self,label="Select File", size=(btlen,-1))
        self.Bind(wx.EVT_BUTTON, self.OnClickSelectCompFile, self.selectCompFileBt)
        grid.Add(self.selectCompFileBt, pos=(3,0))

        grid.Add(wx.StaticText(self, label='Start Time To End Time:', size=(btlen*2,-1)), pos=(4,0))
        self.datePicker_1 = adv.DatePickerCtrl(self, wx.ID_ANY, wx.DateTime.Now(),size=(btlen+10, -1))
        grid.Add(self.datePicker_1, pos=(5,0))
        self.timePicker_1 = adv.TimePickerCtrl(self, wx.ID_ANY, wx.DateTime.Now(),size=(btlen+10, -1))
        grid.Add(self.timePicker_1, pos=(5,1))

        self.datePicker_2 = adv.DatePickerCtrl(self, wx.ID_ANY,wx.DateTime.Now(), size=(btlen+10, -1))
        grid.Add(self.datePicker_2, pos=(6,0))
        self.timePicker_2 = adv.TimePickerCtrl(self, wx.ID_ANY,wx.DateTime.Now(), size=(btlen+10, -1))
        grid.Add(self.timePicker_2, pos=(6,1))

        grid.Add(wx.StaticText(self, label='Set FileName:', size=(btlen,-1)), pos=(7,0))
        self.produceFileNameText = wx.TextCtrl(self, size=(btlen,-1))
        grid.Add(self.produceFileNameText, pos=(8,0))

        self.produceBt = wx.Button(self, label="Produce", size=(btlen, -1))
        self.Bind(wx.EVT_BUTTON, self.OnClickProduce, self.produceBt)
        grid.Add(self.produceBt, pos=(9,0))

        self.svnCheckBox = wx.CheckBox(self, label="use svn")
        grid.Add(self.svnCheckBox, pos=(9,1))
        self.Bind(wx.EVT_CHECKBOX, self.OnCheckSvn, self.svnCheckBox)

        comparePosI = 8
        #grid.Add(wx.StaticText(self, label='Compare File:', size=(btlen,-1)), pos=(7,0))
        self.fileGetText1 = wx.TextCtrl(self, size = (btlen, -1), style=wx.TE_READONLY)
        grid.Add(self.fileGetText1, pos=(11 ,0))
        self.selectFileBt_1 = wx.Button(self, label="Select file1", size=(btlen,-1))
        self.Bind(wx.EVT_BUTTON, self.OnClickSelectF1, self.selectFileBt_1)
        grid.Add(self.selectFileBt_1, pos=(12,0))

        self.fileGetText2 = wx.TextCtrl(self, size = (btlen, -1), style=wx.TE_READONLY)
        grid.Add(self.fileGetText2, pos=(13,0))
        self.selectFileBt_2 = wx.Button(self, label="Select file2", size=(btlen,-1))
        self.Bind(wx.EVT_BUTTON, self.OnClickSelectF2, self.selectFileBt_2)
        grid.Add(self.selectFileBt_2, pos=(14,0))

        self.compareBt = wx.Button(self, label="Compare", size=(btlen ,-1))
        self.Bind(wx.EVT_BUTTON, self.OnClickCompare, self.compareBt)
        grid.Add(self.compareBt, pos=(15,0))

        hSizer.Add(grid, 0, wx.ALL, 12)
        mainSizer.Add(hSizer, 0, wx.ALL, 6)
        #mainSizer.Add(grid, 0, wx.ALL, 6)
        self.SetSizerAndFit(mainSizer)
        self.lua = LuaRuntime(unpack_returned_tuples=True)
        self.luafile = open("sourceCode/CalcProduce.lua").read()
        self.lua.execute(self.luafile)
        self.g = self.lua.globals()

        self.filename1 = ''
        self.filename2 = ''
        self.compFileName = ''

    def OnClickProduce(self, event):
        fname = self.produceFileNameText.GetValue()
        if fname == '':
            self.logger.AppendText('FileName 不可为空!\n')
            return
        if self.checkFileExist() == False:
            return
        date1 = self.datePicker_1.GetValue()
        time1 = self.timePicker_1.GetValue()
        timestr = date1.FormatISODate() + ' ' +  time1.FormatISOTime()

        date2 = self.datePicker_2.GetValue()
        time2 = self.timePicker_2.GetValue()
        timestr2 = date2.FormatISODate() + ' ' +  time2.FormatISOTime()
        self.logger.AppendText("设置时间信息: " + timestr + " -> " + timestr2 + "\n")
        self.logger.AppendText("设置文件名为: " + fname + "\n")
        self.logger.AppendText("开始生成产出文件......\n")
        self.produceBt.Enable(False)
        self.produceFileNameText.SetEditable(False)

        ProduceThread(self, fname, self.compFileName)

    def OnClickSelectF1(self, event):
        dlg = wx.FileDialog(self,message=u"选择文件",
                            defaultDir=os.getcwd(),
                            defaultFile="",
                            style=wx.FD_OPEN)
        dialogResult = dlg.ShowModal()
        if dialogResult != wx.ID_OK:
            self.logger.AppendText("文件1 没有进行选择!\n")
            return

        fname = dlg.GetFilename()
        fnames = fname.split(".")
        if len(fnames) < 2 or fnames[1] != 'csv':
            self.logger.AppendText("文件1 选择失败, 请选择csv文件!\n")
            return
        self.filename1 = fnames[0]

        path = dlg.GetPath()
        self.logger.AppendText("文件1 选择: " + path + "\n")
        self.fileGetText1.SetValue(path)

    def OnClickSelectF2(self, event):
        dlg = wx.FileDialog(self,message=u"选择文件",
                            defaultDir=os.getcwd(),
                            defaultFile="",
                            style=wx.FD_OPEN)
        dialogResult = dlg.ShowModal()
        if dialogResult != wx.ID_OK:
            self.logger.AppendText("文件2 没有进行选择!\n")
            return
        fname = dlg.GetFilename()
        fnames = fname.split(".")
        if len(fnames) < 2 or fnames[1] != 'csv':
            self.logger.AppendText("文件2 选择失败, 请选择csv文件!\n")
            return
        self.filename2 = fnames[0]

        path = dlg.GetPath()
        self.logger.AppendText("文件2 选择: " + path + "\n")
        self.fileGetText2.SetValue(path)
    def OnClickSelectCompFile(self, event):
        dlg = wx.FileDialog(self,message=u"选择文件",
                            defaultDir=os.getcwd(),
                            defaultFile="",
                            style=wx.FD_OPEN)
        dialogResult = dlg.ShowModal()
        if dialogResult != wx.ID_OK:
            self.logger.AppendText("对比文件 没有进行选择!\n")
            return
        fname = dlg.GetFilename()
        fnames = fname.split(".")
        if len(fnames) < 2 or fnames[1] != 'csv':
            self.logger.AppendText("Comparae File 选择失败, 请选择csv文件!\n")
            return
        path = dlg.GetPath()
        self.compFileName = path
        self.logger.AppendText("Compare File 选择: " + path + "\n")
        self.compFileText.SetValue(path)

    def OnClickCompare(self, event):
        f1 = self.fileGetText1.GetValue()
        f2 = self.fileGetText2.GetValue()
        if f1 == '' or f2 == '':
            self.logger.AppendText("没有选择两个有效对比文件, 请重新选择!\n")
            return
        self.csvtolua(f1, 'Table_TempProduceResult_1')
        self.csvtolua(f2, 'Table_TempProduceResult_2')

        compfilename = self.filename1 + '-' + self.filename2 + '.csv'
        self.g.compTwoFile(compfilename)
        if len(self.g.pythonCompare2FileStr) < 3:
            self.logger.AppendText("两个文件一致, 无对比信息!\n\n")
            return

        self.logger.AppendText('\n对比文件生成成功,请查看当前目录: ' + compfilename + '!\n')
        self.logger.AppendText('简略信息如下:\n'+ self.g.pythonCompare2FileStr)

    def OnCheckSvn(self, event):
        if self.svnCheckBox.IsChecked():
            if os.system('svn st') != 0:
                self.svnCheckBox.SetValue(False)
                self.logger.AppendText('\nsvn不可用, 请检查当前目录下svn.exe版本!\n')
                return
            self.logger.AppendText('已选用svn, 生成时, 将根据开始时间拉取svn对应配置, 生成产出文件!\n')
        else:
            self.logger.AppendText('已取消使用svn, 生成时, 将根据当前本地配置, 生成产出文件!\n')

    def csvtolua(self, filename, tablename):
        rcsvfile = open(filename, 'r');
        rf = csv.reader(rcsvfile)
        strout = tablename + ' = {\n'
        bix = 0
        index = 0
        for d in rf:
            if index < 2:
                index  = index + 1
                continue

            if len(d) == 7:
                bix = 0
            else:
                bix = 1
                if d[0] == 'Delete':
                    continue
            strout = strout + '[\'' + d[bix + 1] + '-' + str(d[bix]) + '\']={'
            strout = strout + 'ItemID=' + str(d[bix]) + ','
            strout = strout + 'Type=\'' + d[bix + 1] + '\','
            strout = strout + 'ItemName=\'' + d[bix + 2] + '\','
            strout = strout + 'ItemType=' + str(d[bix + 3]) +','
            strout = strout + 'ItemTypeName=\'' + d[bix + 4] + '\','
            strout = strout + 'SourceTableName=\'' + d[bix + 5] + '\','
            strout = strout + 'Params=' + d[bix + 6] + '},'

        strout = strout + '}'
        self.lua.execute(strout)
        rcsvfile.close()
    def checkFileExist(self):
        if os.path.exists('../Table/Table_ProduceLogic.txt') == False:
            self.logger.AppendText('Table目录中没有配置Table_ProduceLogic.txt, 请检查!\n')
            return False

        stable = self.g.Table_ProduceLogic
        findok = True
        for k in stable:
            tname = stable[k].TableName
            if os.path.exists('../Table/Table_'+ tname +'.txt') == False:
                self.logger.AppendText('ProduceLogic 配置的产出文件: ' + tname + ', 在Table/目录下找不到, ' + '行信息: ' + str(k) + '\n')
                findok = False
        return findok


class ProduceThread(threading.Thread):
    def __init__(self, lay, filename, compfilename):
        threading.Thread.__init__(self)
        self.lay = lay
        self.fname = filename
        self.compfilename = compfilename
        self.start()

    def run(self):

        lay = self.lay
        date1 = lay.datePicker_1.GetValue()
        time1 = lay.timePicker_1.GetValue()
        timestr = date1.FormatISODate() + ' ' +  time1.FormatISOTime()
        timenum1 = time.mktime(time.strptime(timestr,'%Y-%m-%d %H:%M:%S'))

        date2 = lay.datePicker_2.GetValue()
        time2 = lay.timePicker_2.GetValue()
        timestr2 = date2.FormatISODate() + ' ' +  time2.FormatISOTime()
        timenum2 = time.mktime(time.strptime(timestr2,'%Y-%m-%d %H:%M:%S'))

        #os.system('svn revert -R *')
        if lay.svnCheckBox.GetValue() == True:
            cmd1 = 'svn up -r ' + '\"{' + timestr + '}\"' + ' ../'
            os.system(cmd1)

        lay.g.reloadLuaFile()
        if self.compfilename != '':
            self.lay.csvtolua(self.compfilename, 'Table_TempProduceResult')
            lay.g.produceItemInfo(timenum1, timenum2, self.fname+'.csv', True)
        else:
            lay.g.produceItemInfo(timenum1, timenum2, self.fname+'.csv', False)

        lay.produceBt.Enable(True)
        lay.produceFileNameText.SetEditable(True)
        lay.logger.AppendText("生成产出文件成功, " + self.fname + ".csv !\n\n")

def main():
    app = wx.App(False)
    frame = wx.Frame(None, 100, 'Produce', size = wx.Size(1400, 1200*0.618))
    panel = ExamplePanel(frame)
    frame.Show()
    app.MainLoop()
main()

