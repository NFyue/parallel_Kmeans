
import random

f = open('test.txt','a')
f.write('%d %d %d %d %d\n' % (1000,4,3,100,0))
for i in range(0,1000):
	a = random.uniform(4,7)
	b = random.uniform(2,4)
	c = random.uniform(1,7)
	d = random.uniform(0,2)
	f.write('%.1f %.1f %.1f %.1f\n' % (a,b,c,d))