import cv2
import numpy as np

filename = "./test/ex1.png"
img = cv2.imread(filename)

N=2
d=10
sigma_Color = 90
sigma_Space = 90
width = img.shape[0]
after_width = 300
f = after_width / width
img = cv2.resize(img, None, fx=f, fy=f)
for i in range(N):
    img = cv2.bilateralFilter(img, d, sigmaColor=sigma_Color, sigmaSpace=sigma_Space)

gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

gray = gray.astype(np.float32)
dst = cv2.cornerHarris(gray, 2, 3, 0.04)
img[dst > 0.1*dst.max()] = [0, 255, 0]

cv2.imwrite("./test/out/harris/ex1-img.png", img)
cv2.imwrite("./test/out/harris/ex1-dst.png", dst)
