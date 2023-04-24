# -*- coding: utf-8 -*-
import wx
import os
import string
import threading
import time
import math
from time import sleep
from threading import Thread
import paramiko
import xml.dom.minidom

class ExamplePanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        ## -------------- data ------------------------------

        self.userNames = ['xingwanli', 'chenshi', 'zhangxiaofei', 'zhangshenlin',
                          'zhongchongxi', 'yujia', 'yangxiaoyan',
                          'shimingyi', 'chaimao','chentao','chenweichong',
                          'kongming','zouziyi','trunk','studio','tf', 'release']
        
        # (userName, ip)
        self.userInfo = [('xingwanli(S1)','172.26.24.112'),('chenshi(S2)','172.26.24.112'),
                         ('zhangxiaofei(S3)','172.26.24.113'),('zhangshenlin(S4)','172.26.24.113'),
                         ('zhongchongxi(S5)','172.26.24.113'), ('yujia(S6)','172.26.24.112'),
                         ('yangxiaoyan(S7)','172.26.24.112'),('shimingyi(S8)','172.26.24.114'),
                         ('chaimao(S9)','172.26.24.114'),('chentao(S10)','172.26.24.114'),
                         ('chenweichong(s16)','172.26.24.113'),('kongming(S17)','172.26.24.112'),
                         ('zouziyi(S22)','172.26.24.113'),('trunk(S18)','172.26.24.114'),
                         ('studio(S19)','172.26.24.130'),('tf(S29)','172.26.24.131'),
                         ('release(S20)','172.26.24.132'),
                         ]
        # (user, pwd)
        self.loginInfo = [('cs1','cs1'),('cs2','cs2'),('cs3','cs3'),('cs4','cs4'),
                          ('cs5','cs5'),('cs6','cs6'),('cs7','cs7'),('cs8','cs8'),
                          ('cs9','cs9'),('cs10','cs10'),('cs16','cs16'),('cs17','cs17'),
                          ('cs22','cs22'),('cs18', 'cs18'),('studio','studio'),('tf','tf'),
                          ('release','release')]
        # (log path)
        self.logPath = ['r1_s1', 'r2_s1', 'r3_s1', 'r4_s1', 'r5_s1', 'r6_s1',
                        'r7_s1', 'r8_s1', 'r9_s1', 'r10_s1', 'r16_s1', 'r17_s1',
                        'r22_s1', 'r18_s1','r19_s1', 'r29_s1', 'r20_s1']

        self.ssh = []
        self.threads = [0] * len(self.userNames)

        self.restartStatus = [0] * len(self.userNames) # 0 初始状态, 1 启动中, 2 启动完毕
        self.who=0
        self.local_dir=0

        self.rootLoginInfo = ['root','P@ssw0rd']
        self.rootAddr = ['172.26.24.112', '172.26.24.113', '172.26.24.114', '172.26.24.115']
        self.sshDomain = {'s1':0, 's2':0, 's3':1, 's4':1,
                          's5':1, 's6':0, 's7':0, 's8':3,
                          's9':2, 's10':2, 's16':1, 's17':0,
                          's18':2, #'s20':2, 's22':1, 's19':3,
                         }

        self.rootssh = []

        self.navmeshAddr = ['172.26.24.115', 'liuxin', 'liuxin']
        self.navmeshSSH = 0

        self.initConnectFlag = False

        #self.logs = []
        #for i in range(len(self.userNames)):
        #    self.logs.append([])
        
        ## -------------- layout init------------------------

        filemenu= wx.Menu()
        menuOpen = filemenu.Append(wx.ID_OPEN, "&How to use"," Open a file to edit")

        menuSetName= filemenu.Append(wx.ID_SETUP, u"&Set Name"," Information about this program")
        menuSetPath= filemenu.Append(wx.ID_ABOUT, u"&Set Path"," Information about this program")
        menuBar = wx.MenuBar()
        menuBar.Append(filemenu,"&Help") # Adding the "filemenu" to the MenuBar
        parent.SetMenuBar(menuBar)  # Adding the MenuBar to the Frame content.

        parent.Bind(wx.EVT_MENU, self.OnClickHelp, menuOpen)
        parent.Bind(wx.EVT_MENU, self.OnClickSetName, menuSetName)
        parent.Bind(wx.EVT_MENU, self.OnClickSetPath, menuSetPath)
        
        # create some sizers
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        grid = wx.GridBagSizer(hgap=8, vgap=8)
        hSizer = wx.BoxSizer(wx.HORIZONTAL)
        hSizer2 = wx.BoxSizer(wx.HORIZONTAL)
        grid2 = wx.GridBagSizer(hgap=8, vgap=8)

        # label
        self.quote = wx.StaticText(self, label="ServerList:", size=(100,60))
        grid.Add(self.quote, pos=(0,0))
        font = wx.Font(12, wx.SWISS, wx.NORMAL, wx.BOLD)
        self.quote.SetFont(font)

        # button
        self.allSelectBt = wx.Button(self, label="Select All")
        grid.Add(self.allSelectBt, pos=(0,1))
        self.Bind(wx.EVT_BUTTON, self.OnClickSelectAll,self.allSelectBt)
        self.noneSelectBt = wx.Button(self, label="Select None")
        grid.Add(self.noneSelectBt, pos=(0,2))
        self.Bind(wx.EVT_BUTTON, self.OnClickSelectNone,self.noneSelectBt)

        # A multiline TextCtrl to display log
        self.logger = wx.TextCtrl(self, size=(400,400), style=wx.TE_MULTILINE | wx.TE_READONLY)
        self.logger.SetEditable(False)

        # Checkbox
        self.selects = []
        for i in range(0,len(self.userNames)):
            self.selects.append(wx.CheckBox(self, label=self.userInfo[i][0]))
        # buttons
        self.seeLogBts = []
        for i in range(0,len(self.userNames)):
            self.seeLogBts.append(wx.Button(self, -1, "See Log"))
            self.Bind(wx.EVT_BUTTON, lambda evt,index=i:self.OnClickSeeLog(evt,index) ,self.seeLogBts[i])
        
        grid.Add(self.selects[0], pos=(1,0))
        #self.Bind(wx.EVT_CHECKBOX, self.EvtCheckBox, self.selects[0])
        grid.Add(self.seeLogBts[0], pos=(1,1))

        grid.Add(self.selects[1], pos=(1,2))
        grid.Add(self.seeLogBts[1], pos=(1,3))

        grid.Add(self.selects[2], pos=(2,0))
        grid.Add(self.seeLogBts[2], pos=(2,1))

        grid.Add(self.selects[3], pos=(2,2))
        grid.Add(self.seeLogBts[3], pos=(2,3))

        grid.Add(self.selects[4], pos=(3,0))
        grid.Add(self.seeLogBts[4], pos=(3,1))

        grid.Add(self.selects[5], pos=(3,2))
        grid.Add(self.seeLogBts[5], pos=(3,3))

        grid.Add(self.selects[6], pos=(4,0))
        grid.Add(self.seeLogBts[6], pos=(4,1))

        grid.Add(self.selects[7], pos=(4,2))
        grid.Add(self.seeLogBts[7], pos=(4,3))

        grid.Add(self.selects[8], pos=(5,0))
        grid.Add(self.seeLogBts[8], pos=(5,1))

        grid.Add(self.selects[9], pos=(5,2))
        grid.Add(self.seeLogBts[9], pos=(5,3))

        grid.Add(self.selects[10], pos=(6,0))
        grid.Add(self.seeLogBts[10], pos=(6,1))

        grid.Add(self.selects[11], pos=(6,2))
        grid.Add(self.seeLogBts[11], pos=(6,3))

        grid.Add(self.selects[12], pos=(7,0))
        grid.Add(self.seeLogBts[12], pos=(7,1))

        grid.Add(self.selects[13], pos=(7,2))
        grid.Add(self.seeLogBts[13], pos=(7,3))

        grid.Add(self.selects[14], pos=(8,0))
        grid.Add(self.seeLogBts[14], pos=(8,1))

        grid.Add(self.selects[15], pos=(8,2))
        grid.Add(self.seeLogBts[15], pos=(8,3))

        grid.Add(self.selects[16], pos=(9,0))
        grid.Add(self.seeLogBts[16], pos=(9,1))

        # gauge
        self.gauge = wx.Gauge(self, -1, 400,size = (400,20), pos=(10,460))
        self.gauge.Show(False)
        # button clear
        self.clearBt = wx.Button(self, label="Clear", size=(60,30), pos=(760,420))
        self.Bind(wx.EVT_BUTTON, self.OnClickClear,self.clearBt)
        # button
        self.commitBt =wx.Button(self, label="Commit")
        self.Bind(wx.EVT_BUTTON, self.OnClickCommit,self.commitBt)
        self.restartBt =wx.Button(self, label="Restart")
        self.Bind(wx.EVT_BUTTON, self.OnClickRestart,self.restartBt)

        # combox
        servernames=[]
        for k in sorted(self.sshDomain.keys()):
            servernames.append(k)

        self.combox = wx.ComboBox(self, -1, "s1", choices=servernames, name="Server:", size=(100,-1))
        self.resetTimeBt = wx.Button(self, label="Reset  Time",size=(100,-1))
        self.Bind(wx.EVT_BUTTON, self.OnClickResetTime, self.resetTimeBt)
        self.changeTimeBt = wx.Button(self, label="Set Time",size=(100,-1))
        self.Bind(wx.EVT_BUTTON, self.OnClickChangeTime, self.changeTimeBt)

        grid2.Add(self.combox, pos=(1,0))
        grid2.Add(self.changeTimeBt, pos=(2,0))
        grid2.Add(self.resetTimeBt, pos=(3,0))

        #SpinCtrl time
        self.spinctrl = []
        timenames = ['Year', 'Month', 'Day', 'Hour', 'Min', 'Sec']
        timerange = [(1970,2050,2017),(1,12,9),(1,31,1),(0,23,5),(0,59,0),(0,59,0)]
        timenow = time.localtime()
        for i in range(0,6):
            self.spinctrl.append(wx.SpinCtrl(self, -1, "", (10,10), size=(80,-1)))
            self.spinctrl[i].SetRange(timerange[i][0],timerange[i][1])
            #self.spinctrl[i].SetValue(timerange[i][2])
            self.spinctrl[i].SetValue(timenow[i])

            k = i % 3 + 1
            j = i / 3 * 2 + 1
            grid2.Add(self.spinctrl[i], pos=(k,j))

            j = i / 3 * 2 + 2
            grid2.Add(wx.StaticText(self, label=timenames[i], size=(50,-1)), pos=(k,j))

        grid2.Add(wx.StaticText(self, label='Build Navmesh', size=(120,-1)), pos=(1,5))
        self.trunkNavmeshBt = wx.Button(self, label="Trunk Navmesh", size = (120, -1))
        self.Bind(wx.EVT_BUTTON, self.OnClickTrunkNavmesh, self.trunkNavmeshBt)
        grid2.Add(self.trunkNavmeshBt, pos=(2,5))
        self.studioNavmeshBt = wx.Button(self, label="Studio Navmesh", size = (120, -1))
        self.Bind(wx.EVT_BUTTON, self.OnClickStudioNavmesh, self.studioNavmeshBt)
        grid2.Add(self.studioNavmeshBt, pos=(3,5))
        # sizer
        hSizer2.Add(self.commitBt, 0, wx.ALL, 6)
        hSizer2.Add(self.restartBt, 0, wx.ALL, 6)
        
        hSizer.Add(grid, 0, wx.ALL, 12)
        hSizer.Add(self.logger)
        
        mainSizer.Add(hSizer, 0, wx.ALL, 6)
        
        #mainSizer.Add(self.gauge, 0, wx.ALL, 6)
        mainSizer.Add(hSizer2, 0, wx.ALL, 6)

        mainSizer.Add(grid2, 0, wx.ALL, 6)
        self.SetSizerAndFit(mainSizer)

        
        self.timer = wx.Timer(self, 0.2)
        self.Bind(wx.EVT_TIMER, self.OnTimer, self.timer)
        self.count=0
        
        ## ------------- ssh init -----------------
        self.loadconfig()
        self.initBtByStatus(False)
        self.connectThread = SSHConnectThread(self)
        #self.connectLinux()

    # loadconfig
    def loadconfig(self):
        # xml load
                    
        outfile=open("config","a+")
        allrecord=outfile.read()
        recordList=string.split(allrecord,"\n")
        outfile.close()

        for i in range(0,len(recordList)):
            if recordList[i].find('user:') != -1:
                self.who = recordList[i][5:]
            elif recordList[i].find('path:') != -1:
                self.local_dir = recordList[i][5:]
    def connectLinux(self):
        #try:
        for i in range(0,len(self.userInfo)):
            onessh = paramiko.SSHClient()
            onessh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            #connect(ip,name,pwd,timeout)
            try:
                onessh.connect(self.userInfo[i][1], 22, self.loginInfo[i][0], self.loginInfo[i][1], timeout=5)
            except:
                self.logger.AppendText('connect error: '+(self.userInfo[i][0])+'\n')
            self.ssh.append(onessh)

        for i in range(0,len(self.rootAddr)):
            onessh = paramiko.SSHClient()
            onessh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            try:
                onessh.connect(self.rootAddr[i], 22, self.rootLoginInfo[0], self.rootLoginInfo[1], timeout=5)
            except:
                self.logger.AppendText('connect error: '+self.rootAddr[i]+'\n')
            self.rootssh.append(onessh)

        self.logger.AppendText('connect success!\n')

        if self.navmeshSSH == 0:
            self.navmeshSSH = paramiko.SSHClient()
            self.navmeshSSH.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            try:
                self.navmeshSSH.connect(self.navmeshAddr[0], 22, self.navmeshAddr[1], self.navmeshAddr[2], timeout=5)
            except:
                self.logger.AppendText('connect navmesh server error: ' + '\n')
        #except:
            #self.logger.AppendText('connect error!\n')
            
    # exec cmd
    def execcmd(self, index, cmmd):
        cmd = "cd server;"
        cmd += cmmd
        stdin,stdout,stderr = self.ssh[index].exec_command(cmd)
        
        if (cmmd == "source ~/.bash_profile && ./restart"):
            stdout.readline()
        else:
            stdout.read()

    def execRootCmd(self, index, cmmd):
        stdin,stdout,stderr = self.rootssh[index].exec_command(cmmd)
        stdout.read()
        

    def restartServer(self, index):
        # check clock
        cmd = "stat -c %Y ~/server/bin/Debug/SceneServer"
        stdin,stdout,stderr = self.ssh[index].exec_command(cmd)
        filetimes = stdout.read()
        if len(filetimes) > 5:
            # get obj file modified time
            filetime = string.atoi(filetimes)
            # get server time
            cmd = "date +%s"
            stdin,stdout,stderr = self.ssh[index].exec_command(cmd)
            servertimes = stdout.read()
            servertime = string.atoi(servertimes)
            if servertime < filetime:
                #print("need clean")
                self.execcmd(index, "make touch")
                self.execcmd(index, "make clean")

        self.execcmd(index,"git pull")
        #self.execcmd(index,"make clean")
        self.execcmd(index,"make -j8")
        self.execcmd(index,"cd bin/Debug;./kill-9")
        self.execcmd(index,"source ~/.bash_profile && ./restart")
            
    def haveSelected(self):
        for i in range(0,len(self.selects)):
            if self.selects[i].GetValue() == True:
                return True
        return False

    def setSSHStatus(self, index, value):
        self.restartStatus[index] = value
    def getSSHStatus(self, index):
        return self.restartStatus[index]
    
    def warnRepeatStart(self, index):
        dlg = wx.MessageDialog( self, self.userInfo[index][0] + "is alread restarting, please wait!", "Warning", wx.OK)
        dlg.ShowModal()
        dlg.Destroy()  

    def showGauge(self):
        self.gauge.Show(True)
        self.timer.Start(100)
    def disGauge(self):
        self.gauge.Show(False)
        self.timer.Stop()
        
    def isSSHOver(self):
        for i in range(0,len(self.selects)):
            if self.getSSHStatus(i) == 'running':
                return False
        return True

    def getRemotePath(self, index):
        return "/home/" + self.loginInfo[index][0] + "/server/bin/Debug/Lua/"
    # update config
    def updateConfig(self, index):
        self.execcmd(index,"rm -rf bin/Debug/Lua")
        self.execcmd(index,"./update_resource.sh")
        print(self.local_dir)
        print(self.who)
        try:
            t=paramiko.Transport((self.userInfo[index][1], 22))
            t.connect(username=self.loginInfo[index][0], password=self.loginInfo[index][1])
            sftp=paramiko.SFTPClient.from_transport(t)

            #GameConfig
            ori = ""
            ori += self.local_dir
            ori += "GameConfig.txt"
            cur = ""
            cur += self.getRemotePath(index)
            cur += "GameConfig.txt"

            sftp.put(ori, cur)
            self.logger.AppendText(ori + '\n')

            #GameConfig
            ori = ""
            ori += self.local_dir
            ori += "CommonFun.txt"
            cur = ""
            cur += self.getRemotePath(index)
            cur += "CommonFun.txt"

            sftp.put(ori, cur)
            self.logger.AppendText(ori + '\n')

            #ServerLua
            #ori = ""
            #ori += self.local_dir
            #ori += "ServerLua.lua"
            #cur = ""
            #cur += self.getRemotePath(index)
            #cur += "ServerLua.lua"

            #sftp.put(ori, cur)
            #self.logger.AppendText(ori + '\n')

            files=os.listdir(self.local_dir + "Table\\")
            #files = sftp.listdir(self.remote_dir)
            for f in files:
                ori = ""
                ori += self.local_dir
                ori += "Table\\"
                ori += f
                cur = ""
                cur += self.getRemotePath(index)
                cur += "Table/"
                cur += f

                #sftp.get(os.path.join(dir_path,f),os.path.join(local_path,f)
                #sftp.put(os.path.join(self.local_dir, f), os.path.join(self.remote_dir, f))
                sftp.put(ori, cur)
                self.logger.AppendText(ori + '\n')

            self.logger.AppendText("")
            self.logger.AppendText('Upload file success\n\n')
            t.close()

        except Exception:
            print ("error!")
    
    ## -------------------- event ----------------     
     
    def OnClickSelectAll(self, event):
        for i in range(0,len(self.selects)):
            self.selects[i].SetValue(True)
    def OnClickSelectNone(self, event):
        for i in range(0,len(self.selects)):
            self.selects[i].SetValue(False)
        
    def OnClickRestart(self, event):
        if self.haveSelected() == False:  
            dlg = wx.MessageDialog( self, "No server be selected, please check!", "Warning", wx.OK)
            dlg.ShowModal() # Show it
            dlg.Destroy() # finally destroy it when finished.
        for i in range(0, len(self.selects)):
            if self.selects[i].GetValue() == True:
                if self.getSSHStatus(i) == 'running':
                    self.warnRepeatStart(i)
                else:    
                    self.threads[i] = SSHThread(self,i)
                    self.setSSHStatus(i, 'running')
                    self.seeLogBts[i].Enable(False)
                    self.seeLogBts[i].SetLabel("running...")
        self.showGauge()
                    
    def OnClickSeeLog(self, event, index):
        cmd = "cd server;" + "cat bin/Debug/log/xd/" + self.logPath[index] + "/" + "SceneServer1.log | grep 'ERROR' | grep -v '发送失败'"
        #cmd = "cd server;" + "cat bin/Debug/log/SceneServer1.log | grep 'Table'" #  | grep 'Table'"
        stdin,stdout,stderr = self.ssh[index].exec_command(cmd)
        str = stdout.read().decode('utf8')
        self.logger.AppendText(str)
        if len(str) < 3:
            self.logger.AppendText('there is no error log\n')

        # watch process
        stdin,stdout,stderr = self.ssh[index].exec_command("cd server;ps x")
        strs = stdout.read()
        pids = string.split(strs, "\n")
        for i in range (0, len(pids)):
            if pids[i].find('Server') != -1:
                self.logger.AppendText(pids[i] + "\n")
        #self.logger.AppendText(stdout.read())
        #self.execcmd(index,"make -j8")

    def OnClickCommit(self, event):
        print(self.who)
        print(self.local_dir)
        if self.who==0 or self.local_dir==0:
            dlg = wx.MessageDialog( self, u"请先参考菜单栏Help进行设置.", "Warn", wx.OK)
            dlg.ShowModal() # Show it
            dlg.Destroy() # finally destroy it when finished.
            return
            
        userIndex = -1;
        for i in range(0, len(self.userNames)):
            if self.userNames[i] == self.who:
                userIndex = i
                break
        if userIndex == -1:
            return
        self.updateConfig(userIndex)

    def OnTimer(self,event):
        self.count = self.count + 20
        if self.count > 400:
            self.count = 0
        self.gauge.SetValue(self.count)
        if self.isSSHOver():
            self.disGauge()

    def OnClickClear(self,event):
        self.logger.SetValue('')
    def OnClickHelp(self,event):
        dlg = wx.MessageDialog( self, u"请依次点击Help中的Set Name, Set Path设置用户名与工作路径.(工作路径需选择本地计算机Cehua/Lua目录)", "Help", wx.OK)
        dlg.ShowModal() # Show it
        dlg.Destroy() # finally destroy it when finished.

        
    def OnClickSetPath(self,event):
        dlg = wx.DirDialog(self, "Choose a directory", os.getcwd())
        #dlg = wx.FileDialog(self, "Choose a file", '', "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            
            outfile=open("config","a+")
            allrecord=outfile.read()
            recordList=string.split(allrecord,"\n")
            #print(allrecord)
            outfile.close()

            self.local_dir = dlg.GetPath() + '\\'
            outfile=open("config","w")
            for i in range(0,len(recordList)):
                if recordList[i].find('user:') != -1:
                    outfile.write(recordList[i]+'\n')
                    break
            outfile.write('path:' + dlg.GetPath()+  '\\' + '\n')     
        dlg.Destroy()
        
    def OnClickSetName(self,event):
        dlg = wx.TextEntryDialog(None, u"请在下面文本框中输入你的名字（全拼,如:liming):", u"设置用户名", u"liming")
        if dlg.ShowModal() == wx.ID_OK:
            username = dlg.GetValue()
            finduser = False
            for i in range(0,len(self.userNames)):
                if self.userNames[i] == username:
                    finduser = True
                    break
            if finduser == False:
                wdlg = wx.MessageDialog( self, u"此用户名没有权限,请检查您的输入!", "Warning", wx.OK)
                wdlg.ShowModal() # Show it
                wdlg.Destroy() # finally destroy it when finished.

                   #dlg = wx.FileDialog(self, "Choose a file", '', "", "*.*", wx.OPEN)

            self.who = username
            outfile=open("config","a+")
            allrecord=outfile.read()
            recordList=string.split(allrecord,"\n")
            outfile.close()
            
            outfile=open("config","w")
            outfile.write('user:' + username+'\n') 
            for i in range(0,len(recordList)):
                if recordList[i].find('path:') != -1:
                    outfile.write(recordList[i]+'\n')
                    break            
        dlg.Destroy() # finally destroy it when finished.       

    def getSelectServerIndex(self):
        sname = self.combox.GetValue()
        if self.sshDomain.has_key(sname) == False:
            self.logger.AppendText('Server Name Error!\n')
            return -1
        server_index = self.sshDomain[sname]
        return server_index

    def getServerNamesByIndex(self, index):
        names = "server: "
        sname = self.combox.GetValue()
        for k in self.sshDomain:
            if self.sshDomain[k] == index and sname != k:
                names = names + k + ", "
        return names

    def getTimeString(self):
        timeStr=""
        for i in range(0,3):
            timeStr = timeStr + str(self.spinctrl[i].GetValue())
            if i == 2:
                break
            timeStr = timeStr + '-'

        timeStr = timeStr + ' '
        for i in range(3,6):
            timeStr = timeStr + str(self.spinctrl[i].GetValue())
            if i == 5:
                break
            timeStr = timeStr + ':'
        return timeStr
        

    def OnClickResetTime(self, event):
        #self.logger.AppendText('click reset time\n')
        server_index = self.getSelectServerIndex()
        if server_index == -1:
            return

        #sshdir = self.rootAddr[server_index]
        #self.logger.AppendText(sshdir+'\n')
        cmd = 'hwclock -s'
        self.execRootCmd(server_index, cmd)

        timeNow = time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
        self.logger.AppendText("\n" + self.combox.GetValue() + ", Reset Time Successfully, Time Now:" + timeNow + "\n")

        othernames = self.getServerNamesByIndex(server_index)
        if len(othernames) > 8:
            self.logger.AppendText(othernames + "has changed too!" + "\n")

    def OnClickChangeTime(self, event):
        #self.logger.AppendText("click change time")
        server_index = self.getSelectServerIndex()
        if server_index == -1:
            return

        #sshdir = self.rootAddr[server_index]
        #self.logger.AppendText(sshdir+'\n')
        timeStr = self.getTimeString()
        cmd = 'date -s' + '\'' + timeStr + '\''
        #self.logger.AppendText(cmd+'\n')
        self.execRootCmd(server_index, cmd)
        self.logger.AppendText("\n" + self.combox.GetValue() + ", Set Time Successfully, Time Now:" + timeStr + "\n")

        othernames = self.getServerNamesByIndex(server_index)
        if len(othernames) > 8:
            self.logger.AppendText(othernames + "has changed too!" + "\n")

    def OnClickTrunkNavmesh(self, evet):
        #if no connect, try to connect
        if self.navmeshSSH == 0:
            self.navmeshSSH = paramiko.SSHClient()
            self.navmeshSSH.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            try:
                self.navmeshSSH.connect(self.navmeshAddr[0], 22, self.navmeshAddr[1], self.navmeshAddr[2], timeout=5)
            except:
                self.logger.AppendText('connect navmesh server error: ' + '\n')
        self.threadingNavmesh1 = NavmeshThread(self,1)
        self.trunkNavmeshBt.Enable(False)
        self.trunkNavmeshBt.SetLabel("running...")

    def OnClickStudioNavmesh(self, evet):
        #if no connect, try to connect
        if self.navmeshSSH == 0:
            self.navmeshSSH = paramiko.SSHClient()
            self.navmeshSSH.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            try:
                self.navmeshSSH.connect(self.navmeshAddr[0], 22, self.navmeshAddr[1], self.navmeshAddr[2], timeout=5)
            except:
                self.logger.AppendText('connect navmesh server error: ' + '\n')
        self.threadingNavmesh2 = NavmeshThread(self,2)
        self.studioNavmeshBt.Enable(False)
        self.studioNavmeshBt.SetLabel("buliding...")

    def buildNavmesh(self, version_type):
        #bulid navmesh
        cmd = "source ~/.bash_profile;cd; cd server/bin/Debug;./kill-9;touch gsjgsj"
        stdin, stdout, stderr = self.navmeshSSH.exec_command(cmd)
        stdout.read()
        if version_type == 1:
            #trunk
            cmd = "source ~/.bash_profile && cd /home/liuxin/server/ && ./update_resource.sh && cd bin/Debug && ./restart > navmesh.log 2>&1;"
        else:
            #studio
            cmd = "source ~/.bash_profile && cd /home/liuxin/studio/server/ && ./update_resource.sh && cd bin/Debug && ./restart > navmesh.log 2>&1;"
        #self.logger.AppendText(cmd)
        stdin, stdout, stderr = self.navmeshSSH.exec_command(cmd)
        stdout.readline()

    def checkNavmeshOver(self):
        if self.navmeshSSH == 0:
            return False
        cmd = "ps x | grep nSceneServer1"
        stdin, stdout, stderr = self.navmeshSSH.exec_command(cmd)
        str = stdout.read().decode('utf8')
        line = str.count('\n')
        #print(line)
        #self.logger.AppendText(str(line) + '\n')
        if str.count('\n') <= 2:
            self.logger.AppendText('navmesh build over! \n')
            return True
        return False
    def initBtByStatus(self, initover):
        self.restartBt.Enable(initover)
        self.commitBt.Enable(initover)
        self.resetTimeBt.Enable(initover)
        self.changeTimeBt.Enable(initover)
        for i in range(0, len(self.seeLogBts)):
            self.seeLogBts[i].Enable(initover)
        
# one restart process
class SSHThread(threading.Thread):
    def __init__(self,lay,index):
        threading.Thread.__init__(self)
        self.lay = lay
        self.index = index
        self.start()
    def run(self):
        #print("begin")
        self.lay.restartServer(self.index)
        
        # wait to produce log
        sleep(20)
        
        self.lay.setSSHStatus(self.index, 'finish')
        self.lay.seeLogBts[self.index].Enable(True)
        self.lay.seeLogBts[self.index].SetLabel("See Log")
        #print("ok")

#navmesh process
class NavmeshThread(threading.Thread):
    def __init__(self, lay, version_type):
        threading.Thread.__init__(self)
        self.lay = lay
        self.timetick = 0
        self.version_type = version_type
        self.start()
    def run(self):
        self.lay.buildNavmesh(self.version_type)
        while(1):
            timenow = math.floor(time.time())
            if timenow < self.timetick:
                continue
            if self.lay.checkNavmeshOver() == True:
                break
            self.timetick = timenow + 2
        if self.version_type == 1:
            self.lay.trunkNavmeshBt.Enable(True)
            self.lay.trunkNavmeshBt.SetLabel("Build Trunk")
        else:
            self.lay.studioNavmeshBt.Enable(True)
            self.lay.studioNavmeshBt.SetLabel("Build Studio")

#connect thread
class SSHConnectThread(threading.Thread):
    def __init__(self, lay):
        threading.Thread.__init__(self)
        self.lay = lay
        self.start()
    def run(self):
        self.lay.logger.AppendText("Initializing......\n")
        self.lay.connectLinux()
        self.lay.logger.AppendText("Initialize over!")
        self.lay.initConnectFlag = True
        self.lay.initBtByStatus(True)

def main():        
    app = wx.App(False)
    frame = wx.Frame(None, 100, 'ServerControl', size=wx.Size(970,700))
    #frame.SetSize(wx.Size(600,400))
    panel = ExamplePanel(frame)
    frame.Show()
    app.MainLoop()
main()




    



