import matplotlib.pyplot as plt
import sys

X, Y = [], []
i = 0
fileName = "output" + str(sys.argv[1]) + ".txt"
for line in open(str(fileName), 'r'):
  X.append(i)
  Y.append(int(line))
  i = i + 1

plt.plot(X, Y)

ki = "ki = " + str(sys.argv[2]) + "; "
km = "km = " + str(sys.argv[3]) + "; "
kn = "kn = " + str(sys.argv[4]) + "; "
kf = "kf = " + str(sys.argv[5]) + "; "
ps = "ps = " + str(sys.argv[6])

plt.title(ki + km + kn + kf + ps)

plt.ylabel("cwnd[MSS]")
plt.xlabel("t[RTT]")
  
# saving the file.Make sure you 
# use savefig() before show().
saveFile = "image" + str(sys.argv[1]) + ".png"
plt.savefig(saveFile)
print (saveFile + " image file (plot) created")

# plt.show()