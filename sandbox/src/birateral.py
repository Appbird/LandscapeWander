import cv2
import numpy as np
from time import time

path = "./test/out/birateral/"

def birateral_measure(img_original:np.ndarray, N, d):
    img = img_original.copy()
    sigma_Color = 30
    sigma_Space = 30

    begin = time()
    for i in range(N):
        img = cv2.bilateralFilter(img, d, sigmaColor=sigma_Color, sigmaSpace=sigma_Space)
    end = time()
    measured_time = end - begin

    dst_filename = f"N{N}d{d}sC{sigma_Color}sS{sigma_Space}_{img.shape[0]}_{img.shape[1]}.png"
    dst_path = path + dst_filename
    cv2.imwrite(dst_path, img)
    print(f"|{img.shape}|{N}|{d}|{sigma_Color}|{sigma_Space}|{measured_time:g}|![result image]({dst_filename})|")


# 使用例
img = cv2.imread('./test/media/image.png')

print("# result report")
print("|img_shape|N|d|sigma_Color|sigma_Space|time|dst|")
print("|---|---|---|---|---|---|---|")

for i in range(1, 6):
    for d in range(5, 31, 5):
        birateral_measure(img, i, d)

width = img.shape[0]
f = 200/width
img = cv2.resize(img, None, fx=f, fy=f)
for i in range(1, 6):
    for d in range(5, 31, 5):
        birateral_measure(img, i, d)