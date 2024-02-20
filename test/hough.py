import itertools
import cv2
import numpy as np
from numpy.linalg import norm
from math import pi
from numpy import cos, sin, arctan2

def arg(line:tuple[int, int, int, int]):
    """
    line = (x1, x2, y1, y2)
    """
    x1 = line[0]
    x2 = line[1]
    y1 = line[2]
    y2 = line[3]
    return arctan2(y2 - y1, x2 - x1)

def in_points(line:tuple[int, int, int, int]):
    return np.array([line[0], line[1]]), np.array([line[2], line[3]])

path = "./test/out/hough/"

# 使用例
img = cv2.imread('./test/media/image.png')

N = 5
d = 10
sigma_Color = 90
sigma_Space = 90
min_threshold = 30
max_threshold = 100

width = img.shape[0]
# 下げすぎると木の障害物などに大きく引っ張られやすくなる
f = 300/width
img = cv2.resize(img, None, fx=f, fy=f)

for i in range(N):
    img = cv2.bilateralFilter(img, d, sigmaColor=sigma_Color, sigmaSpace=sigma_Space)
img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
edge_img = cv2.Canny(img_gray, 30, 100)

feature_point_count = cv2.countNonZero(edge_img)
print(feature_point_count)
print(edge_img.shape)
print(edge_img.shape[0]*edge_img.shape[1])
if True:
    lines = cv2.HoughLinesP(edge_img, 1, np.pi/180, int(feature_point_count/100), None, img.shape[0] / 10, img.shape[0] / 10)
    line_count = len(lines)
    lines_in_list:list[tuple[int, int, int, int]] = list(zip(lines[:, 0, 0], lines[:, 0, 1], lines[:, 0, 2], lines[:, 0,3]))
    lines_used:list[bool] = [True for _ in lines_in_list]
    for x1,y1,x2,y2 in lines_in_list:
        cv2.line(img, (x1, y1), (x2, y2), (0, 255, 0), 2)
    
    for i, j in itertools.combinations(range(len(lines_in_list)), 2):
        if not lines_used[i] or not lines_used[j]:
            continue
        l1 = lines_in_list[i]
        l2 = lines_in_list[j]
        arg_i = arg(l1); arg_j = arg(l2)
        p1, p2 = in_points(l1); q1, q2 = in_points(l2)

        if abs(arg_i - arg_j) >= pi / 6:
            continue
        modified_q2 = np.dot(q2 - p1, p2 - p1) / norm(p2 - p1) + p1
        if norm(q2 - modified_q2) > 50:
            continue
        # x1 x2 y1 y2
        l1 = (p1[0], p1[1], modified_q2[0], modified_q2[1])
        lines_used[j] = False

    for x1,y1,x2,y2 in [lines_in_list[i] for i in range(len(lines_in_list)) if lines_used[i]]:
        cv2.line(img, (x1, y1), (x2, y2), (255, 0, 0), 1)
else:
    lines = cv2.HoughLines(edge_img, 100, np.pi/180, 50)
    for rho, theta in [element[0] for element in lines]:
        a = cos(theta)
        b = sin(theta)
        x0 = a*rho
        y0 = b*rho
        k = 1000
        x1 = int(x0 + k*(-b))
        y1 = int(y0 + k*(a))
        x2 = int(x0 - k*(-b))
        y2 = int(y0 - k*(a))
        cv2.line(img, (x1, y1), (x2, y2), (255, 0, 0), 2)

cv2.imwrite(path + "edge.png", edge_img)
cv2.imwrite(path + "test.png", img)