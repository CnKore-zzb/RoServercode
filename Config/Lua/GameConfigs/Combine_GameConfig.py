#!/usr/bin/env python
# encoding: utf-8

import datetime
import zipfile
import json
import gc  
import hashlib
import os
import xml.parsers.expat
from xml.dom import minidom
import re
import codecs
import shutil,sys,string

if(len(sys.argv)>1):
    file_path = sys.argv[1]
else:
    file_path = os.path.split(os.path.realpath(__file__))[0]

gameconfig_table = "GameConfig.txt"
gameconfig_path = os.path.join(file_path, "../"+gameconfig_table)

def print_error(msg):
    print '\033[1;31;40m'
    print msg
    print '\033[0m'

class Sub_GameConfig:
    def __init__(self, fileName, filePath, md5Str):
        self.md5Str = md5Str
        self.filePath = filePath
        self.realMD5 = self.calMd5Str(filePath)
        self.file_name = fileName

    def getContent(self):
        file = open(self.filePath,'r')
        content = file.read()
        if content[:3] == codecs.BOM_UTF8 :
            content = content[3:]
        file.close()
        return content

    def calMd5Str(self,filePath):
        file = open(filePath,'rb')
        md5=hashlib.md5(file.read()).hexdigest()
        file.close()
        # print(md5,filePath)
        return md5 

    def contentUpdated(self):
        if self.md5Str != self.realMD5:
            return True
        return False


class Combined_GameConfig:
    def __init__(self, fileName,filePath):
        self.subs = []
        self.subFileNames = {}
        self.file_name = fileName
        self.sub_md5sCount = 0
        self.sub_md5s = self.readMD5s(filePath)
        self.shouldReCombine = False

    def readMD5s(self,filePath):
        oldmd5s = {}
        f = open(filePath,"r")
        line = f.readline()             # 调用文件的 readline()方法  
        while line:  
            # --md5:GameConfig_XXX:<md5>
            oldMd5Str = line.split("--md5:")
            length = len(oldMd5Str)
            if(length==2):
                oldMd5Str = oldMd5Str[1]
                oldMd5Str = oldMd5Str.split(":")
                length = len(oldMd5Str)
                if(length==2):
                    fileName = oldMd5Str[0].strip()
                    md5 = oldMd5Str[1].strip()
                    oldmd5s[fileName] = md5
                    self.sub_md5sCount = self.sub_md5sCount + 1
                else:
                    break
            else:
                break
            line = f.readline()  
        f.close()
        return oldmd5s

    def getFileRecordMd5(self,fileName):
        if fileName in self.sub_md5s:
            return self.sub_md5s[fileName]
        else:
            return ""

    def addSubGameConfig(self,fileName,filePath):
        sub = Sub_GameConfig(fileName,filePath, self.getFileRecordMd5(fileName))
        self.subFileNames[fileName] = 1
        if(sub.contentUpdated() and not self.shouldReCombine):
                print(fileName,sub.realMD5,sub.md5Str)
                self.shouldReCombine = True
        self.subs.append(sub)

    def checkSubConfigRemoved(self):
        stillGetCount = 0
        for sub in self.subs:
            if sub.file_name in self.sub_md5s:
                stillGetCount = stillGetCount + 1

        if(stillGetCount!=self.sub_md5sCount):
            print("有sub配置被删除了，重新生成")
            self.shouldReCombine = True

    def recombineToFile(self,filePath):
        md5data = ""
        data = ""
        for sub in self.subs:
            md5data = md5data + "--md5:"+sub.file_name+":"+sub.realMD5+"\n"
            data = data+sub.getContent() + "\n"

        file = open(filePath, "wb")
        file.write(md5data)
        file.write("GameConfig = {} \n")
        file.write(data)
        file.close()



def file_extension(path): 
  return os.path.splitext(path)[1] 

def main():
    localPath = file_path
    print("start generating {0} config...".format(sys.argv[0]))
    print("excels path: " + localPath)
    print("gameconfig_table:" + gameconfig_table)

    combine = Combined_GameConfig(gameconfig_table,gameconfig_path)
    for p, d, fs in os.walk(localPath):
        for f in fs:
            if file_extension(f)!=".txt":
                continue

            combine.addSubGameConfig(f,os.path.join(p ,f))
    
    combine.checkSubConfigRemoved()
    if(combine.shouldReCombine):
        print("GameConfig 检测到需要更新")
        combine.recombineToFile(gameconfig_path)

if __name__ == "__main__":
    main()

