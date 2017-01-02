# coding: utf-8
import numpy as np
from scipy import signal
import matplotlib.pyplot as plt

# パラメータ設定
m = 1
c = 1
k = 400

num = [1,0 ]
den = [1, 1]
s1 = signal.lti(num, den)
w, mag, phase = signal.bode(s1, np.arange(1, 500, 1))

# プロット
plt.figure(1)
plt.subplot(211)
plt.loglog(w, 10**(mag/20))
plt.ylabel("Amplitude")
plt.axis("tight")
plt.subplot(212)
plt.semilogx(w, phase)
plt.xlabel("Frequency[Hz]")
plt.ylabel("Phase[deg]")
plt.axis("tight")
plt.ylim(-180, 180)
#plt.savefig('150612TF02.svg')
plt.show()
