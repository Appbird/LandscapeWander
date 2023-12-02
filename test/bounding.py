import itertools
import cv2
import numpy as np
from numpy.linalg import norm
from math import pi, atan2
from sys import argv

def arg(line:tuple[int, int, int, int]):
    """
    line = (x1, y1, x2, y2)
    """
    x1 = line[0]
    y1 = line[1]
    x2 = line[2]
    y2 = line[3]
    return atan2(y2 - y1, x2 - x1)

def in_points(line:tuple[int, int, int, int]):
    return np.array(
        [line[0], line[1]] if line[0] < line[2] else [line[1], line[0]]
    ), np.array(
        [line[2], line[3]] if line[0] < line[2] else [line[3], line[2]]
    )

path = "./test/out/hough/"

# 使用例
img = cv2.imread(f'./test/media/{argv[1]}')
file_name = argv[1].split('.')[0]
width = img.shape[0]

N = 5
d = 10
sigma_Color = 90
sigma_Space = 90

canny_min = 30
canny_max = 100

elected_coef = 0.01
minLineLength_coef = 30
maxLineGap_coef = 15

after_width = 300

threshold_q = 10
extended_length = 90

f = after_width / width
# 下げすぎると木の障害物などに大きく引っ張られやすくなる
img = cv2.resize(img, None, fx=f, fy=f)

img_houghline = img.copy()
img_platform_line = img.copy()

for i in range(N):
    img = cv2.bilateralFilter(img, d, sigmaColor=sigma_Color, sigmaSpace=sigma_Space)


img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
edge_img = cv2.Canny(img_gray, canny_min, canny_max)

feature_point_count = cv2.countNonZero(edge_img)

lines = cv2.HoughLinesP(edge_img, 1, np.pi/180, int(feature_point_count * elected_coef), None, img.shape[0] / minLineLength_coef, img.shape[0] / maxLineGap_coef)
line_count = len(lines)
print(line_count)
lines_in_list:list[tuple[int, int, int, int]] = list(zip(lines[:, 0, 0], lines[:, 0, 1], lines[:, 0, 2], lines[:, 0,3]))
lines_used:list[bool] = [True for _ in lines_in_list]
for x1,y1,x2,y2 in lines_in_list:
    cv2.line(img_houghline, (x1, y1), (x2, y2), (0, 255, 0), 2)

for i, j in itertools.combinations(range(len(lines_in_list)), 2):
    if not lines_used[i] or not lines_used[j]:
        continue
    l1 = lines_in_list[i]
    l2 = lines_in_list[j]
    arg_i = arg(l1); arg_j = arg(l2)
    p1, p2 = in_points(l1); q1, q2 = in_points(l2)
    
    l1_len = norm(p2 - p1)

    if abs(arg_i - arg_j) >= pi / 12:
        continue

    p_direction = (p2 - p1) / l1_len
    modified_q1 = np.dot(q1 - p1, p_direction) * p_direction + p1
    modified_q2 = np.dot(q2 - p1, p_direction) * p_direction + p1

    if norm(q1 - modified_q1) > threshold_q:
        continue
    if norm(q2 - modified_q2) > threshold_q:
        continue


    projection = lambda v: np.dot(v - p1, p_direction)
    point_list = np.array([p1, p2, modified_q1, modified_q2])
    x_margined_list = [projection(x) for x in [p1- p_direction*extended_length, p2 + p_direction*extended_length, modified_q1, modified_q2]]
    assert p1[0] < p2[0]
    assert modified_q1[0] < modified_q2[0]
    if max(x_margined_list[0], x_margined_list[2]) > min(x_margined_list[1], x_margined_list[3]):
        continue
    
    sorted_point_list = sorted(point_list, key=projection)
    min_point = sorted_point_list[0]
    max_point = sorted_point_list[3]
    
    # x1 y1 x2 y2
    lines_in_list[i] = (int(min_point[0]),int(min_point[1]), int(max_point[0]), int(max_point[1]))
    lines_used[j] = False

for x1,y1,x2,y2 in [lines_in_list[i] for i in range(len(lines_in_list)) if lines_used[i]]:
    cv2.line(img_platform_line, (x1, y1), (x2, y2), (255, 0, 0), 2)


cv2.imwrite(path + f"{file_name}-birateral.png",    img)
cv2.imwrite(path + f"{file_name}-edge.png",         edge_img)
cv2.imwrite(path + f"{file_name}-houghline.png",    img_houghline)
cv2.imwrite(path + f"{file_name}-platform.png",     img_platform_line)