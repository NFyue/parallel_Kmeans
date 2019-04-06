import matplotlib.pyplot as plt

y1 = [88,5175,32046]
# x = ['7','150','1000']
# plt.plot(x,y1)
# plt.ylabel('time used')
# plt.xlabel('datasets')
# plt.title("sequential")

# plt.savefig('sequential.png', format='png', dpi=1000)


y2 = [115,733,7406]
# x = ['7','150','1000']
# plt.plot(x,y2)
# plt.ylabel('time used')
# plt.xlabel('datasets')
# plt.title("cilkPlus_threads_2")
# plt.savefig('cilkPlus_threads_2.png', format='png', dpi=1000)

y3 = [168,2604,10510]
# x = ['7','150','1000']
# plt.plot(x,y3)
# plt.ylabel('time used')
# plt.xlabel('datasets')
# plt.title("cilkPlus_threads_4")
# plt.savefig('cilkPlus_threads_4.png', format='png', dpi=1000)

y4 = [62,1277,11648]
x = ['7','150','1000']
# plt.plot(x,y4)
plt.ylabel('time used')
plt.xlabel('datasets')
# plt.title("cilkPlus_threads_8")
# plt.savefig('cilkPlus_threads_8.png', format='png', dpi=1000)



plt.plot(x, y1, 'r-',label = "sequential")
plt.plot( x, y2, 'b-', label = "cilkPlus_threads_2")
plt.plot(x, y3, 'g-', label = "cilkPlus_threads_4")
plt.plot(x,y4,'y-',label = "cilkPlus_threads_8")
plt.legend(loc='upper left')
plt.savefig('all_compare.png', format='png', dpi=1000)
plt.show()
