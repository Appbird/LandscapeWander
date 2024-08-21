import sys
import cv2
from os import path
from cv2 import LineSegmentDetector as LSD

assert len(sys.argv) == 2
file_name = sys.argv[1]
file_base = path.splitext(path.basename(file_name))[0]

input_color_img = cv2.imread(file_name)
height = input_color_img.shape[0]
f = 600/height
input_color_img = cv2.resize(input_color_img, None, fx=f, fy=f)
input_img = cv2.cvtColor(input_color_img, cv2.COLOR_BGR2GRAY)

N = 3
d = 8
sigma_Color = 10
sigma_Space = 90

for _ in range(N):
    img = cv2.bilateralFilter(img, d, sigmaColor=sigma_Color, sigmaSpace=sigma_Space)


lsd = cv2.createLineSegmentDetector()
lines, width, proc, nfa = lsd.detect(input_img)

w_max = 0
for [w] in proc:
    w_max = max(w_max, w)

print(w_max)

for [[x1, y1, x2, y2]], [w] in zip(lines, proc):
    [x1, y1, x2, y2] = map(int, [x1, y1, x2, y2])
    cv2.line(input_color_img, (x1, y1), (x2, y2), (0, int(50 + 205 * w / w_max), 0), 2)

cv2.imwrite(f"./sandbox/out/lsd/{file_base}.out.png", input_color_img)