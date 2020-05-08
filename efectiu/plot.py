import numpy as np
import matplotlib.pyplot as plt

barWidth = 2
bars1 = [3, 3, 3,3,3,3,1]
bars2 = [4, 2, 3, 4,2,3,4]
bars3 = [4, 6, 7, 10, 4, 4, 4,4,4,4,4,4,4]
bars4 = bars1 + bars2 + bars3

# The X position of bars
r1 = [1,5,9]
r2 = [2,6,10]
r3 = [3,4,7,8,11,12]
r4 = r1 + r2 + r3

fig, ax = plt.subplots()
# Make a fake dataset:
SHiP_32k = [ 2.8280, 3.4845, 1.4933, 3.1729, 3.7107, 0.8128, 3.1580, 3.8350,  3.2024, 3.2784, 3.6186, 2.6336, 3.8931, 1.3151, 2.8117, 3.7433, 3.9771, 3.2332, 1.5600, 1.9589, 3.5598, 3.6830, 2.6145, 2.3347, 3.0159, 2.6185, 0.6722]
ipc_lru=[2.5802,3.5470, 1.4640, 3.3200,3.7107, 0.8236, 3.3862, 3.5944, 2.2487,3.2847, 3.6531, 2.6273, 3.9170, 1.2580, 2.8117, 3.6472, 3.9759, 3.2380, 1.6051, 1.9139, 3.4528, 3.7198, 2.5053, 2.4891, 3.0973, 1.3701, 0.4637]
bars= ('perlbench', 'bzip2', 'gcc', 'bwaves', 'gamess','milc','zeusmp','gromacs', 'cactusADM','leslie3d','namd','gobmk','dealII','soplex','povray','calculix','hmmer','sjeng','GemsFDTD','libquantum','h264ref','tonto','lbm','astar','wrf','sphinx3','xalancbmk')
#plt.xticks( ['400.perlbench', '401.bzip2', '403.gcc', '410.bwaves', '416.gamess','433.milc','434.zeusmp','435.gromacs', '436.cactusADM','437.leslie3d','444.namd','445.gobmk','447.dealII','450.soplex','453.povray','454.calculix','456.hmmer','458.sjeng','459.GemsFDTD','462.libquantum','464.h264ref','465.tonto','470.lbm','473.astar','481.wrf','482.sphinx3','483.xalancbmk'], rotation=90)
y_pos = 6*np.arange(len(bars))
#y_pos = [i+1 for i in range(len(bars))] 
# Create bars
ax.bar(y_pos, SHiP_32k, barWidth+0.5 ,label='SHiP')
ax.bar(y_pos+barWidth, ipc_lru, barWidth+0.5,label='LRU')
 
# Create names on the x-axis
plt.xticks(y_pos+1,bars,rotation=90)
#plt.xticks( ['400.perlbench', '401.bzip2', '403.gcc', '410.bwaves', '416.gamess','433.milc','434.zeusmp','435.gromacs', '436.cactusADM','437.leslie3d','444.namd','445.gobmk','447.dealII','450.soplex','453.povray','454.calculix','456.hmmer','458.sjeng','459.GemsFDTD','462.libquantum','464.h264ref','465.tonto','470.lbm','473.astar','481.wrf','482.sphinx3','483.xalancbmk'], rotation=90)#
plt.xlabel('trace')
plt.ylabel('IPC')

ax.legend()

#for index, value in enumerate(SHiP_32k):
#    plt.text(index*1 - 0.3, 0.8, str(value))

 
# Show graphic
plt.show()

