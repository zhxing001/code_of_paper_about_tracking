# -*- coding: utf-8 -*-
"""
Created on Mon Jan 28 20:16:05 2019

@author: zhxing
this code can draw position precision of tracking result in vot challenge
"""

import math
import matplotlib.pyplot as plt

#存放文件的路径以及各种文件的路径
path="C:\\Users\\zhxing\\Desktop\\code_of_paper_about_tracking\\results\\"
ave_fps_kcf="_ave_fps_kcf.txt"
ave_fps_kcf_inter="_ave_fps_kcf_inter.txt"
res_ground="_res_ground.txt"
res_kcf="_res_kcf.txt"
res_kcf_interpolation="_res_kcf_interpolation.txt"

file=open(path+"list.txt")
lines=file.readlines()




#定义画中心位置误差图像的函数
def drawCLE(ResGroundLines,ResKcfLines,ResKcfILines):
    CleKcf=[]
    CleKcfI=[]
    num_of_frame=len(ResGroundLines)-2        #帧数，去掉表头和最后一帧（主要是我结果好像少写了一帧）
    for index in range(1,(num_of_frame+1)):
        #每一行拿出来，第一列是分别是 frame	x	y	width	height,分离出来并转换成数字
        GroundPos=(ResGroundLines[index]).split('\t')
        KcfPos=(ResKcfLines[index]).split('\t')
        KcfIPos=(ResKcfILines[index]).split('\t')
        GroundPos=list(map(int,GroundPos))
        KcfPos=list(map(int,KcfPos))
        KcfIPos=list(map(int,KcfIPos))
        
        #提取中心位置
        P_G=[GroundPos[1]+GroundPos[3]/2,GroundPos[2]+GroundPos[3]/2]
        P_K=[KcfPos[1]+KcfPos[3]/2,KcfPos[2]+KcfPos[4]/2]
        P_KI=[KcfIPos[1]+KcfIPos[3]/2,KcfIPos[2]+KcfIPos[4]/2]
        
        CLE_KCF=math.sqrt((P_K[0]-P_G[0])**2+(P_K[1]-P_G[1])**2)
        CLE_KCF_I=math.sqrt((P_KI[0]-P_G[0])**2+(P_KI[1]-P_G[1])**2)
        
        CleKcf.append(CLE_KCF)
        CleKcfI.append(CLE_KCF_I)
    plt.figure()
    plt.plot(CleKcf)
    plt.plot(CleKcfI)
    plt.legend()
        
            


for target in lines:
    print("this is the:\t"+target)
    #target有个回车，这里需要把这个回车给去掉,然后下面把当前target下的文件读取
    AveFpsKcf=open(path+target[:-1]+ave_fps_kcf)
    AveFpsKcfI=open(path+target[:-1]+ave_fps_kcf_inter)
    ResGround=open(path+target[:-1]+res_ground)
    ResKcf=open(path+target[:-1]+res_kcf)
    ResKcfI=open(path+target[:-1]+res_kcf_interpolation)
    AveFpsKcfLines=AveFpsKcf.readlines()
    AveFpsKcfILines=AveFpsKcfI.readlines()
    ResGroundLines=ResGround.readlines()
    ResKcfLines=ResKcf.readlines()
    ResKcfILines=ResKcfI.readlines()
    
    
    drawCLE(ResGroundLines,ResKcfLines,ResKcfILines)
        
        
    
    
    
    
    



