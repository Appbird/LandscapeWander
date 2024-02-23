import itertools
from types import NoneType
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

def segment_distance(l1:float, r1:float, l2:float, r2:float):
    if (r1 < l1): return segment_distance(r1, l1, l2, r2)
    if (r2 < l2): return segment_distance(l1, r1, r2, l2)
    return max(0.0, max(l1, l2) - min(r2, r1));

def lerp(x:float, a:float, b:float):
    return a/b * (x - b) + a
def lerp_neg(x:float, a:float, b:float):
    return -a/b * (x - b) + a

path = "./test/out/bounding/"

# 使用例
img = cv2.imread(f'./test/media/{argv[1]}')
file_name = argv[1].split('.')[0]
width = img.shape[0]

# 対象物体の濃さ
alpha = 0.38
# 得られる線分の微細さ
beta = 0.6
# 線分の集約度合い
gamma = 0.5

assert 0 <= alpha <= 1
assert 0 <= beta <= 1
base_param = 1/2
N = 5
d = 10
sigma_Color = lerp(alpha, 90, base_param)
sigma_Space = lerp(alpha, 90, base_param)

canny_min = lerp(alpha, 30, base_param)
canny_max = lerp(alpha, 100, base_param)

after_width = 400

elected_coef = lerp(alpha, 0.007, base_param)
minLineLength_coef = lerp(1-beta, 1/90, base_param)
maxLineGap_coef = lerp(1-beta, 1/30, base_param)


angle_threshold = lerp(gamma, pi / 4, base_param)
transverse_threshold = after_width * lerp(gamma, 0.03, base_param)
longitudinal_threshold = after_width * lerp(gamma, 0.03, base_param)

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
# 確率的ハフ変換
houghline_threshold = int(feature_point_count * elected_coef)
houghline_minline_gap = img.shape[1] * minLineLength_coef
houghline_maxline_gap = img.shape[1] * maxLineGap_coef
lines = cv2.HoughLinesP(edge_img, 1, np.pi/180, houghline_threshold, None, houghline_minline_gap, houghline_maxline_gap)

# 得られた線をタプルに変換する。
if (type(lines) == NoneType):
    print("no line obtained")
    exit(0)

line_count = len(lines)
print(f"obtained line count by Houghline: {line_count}")

lines_in_list:list[tuple[int, int, int, int]] = list(zip(lines[:, 0, 0], lines[:, 0, 1], lines[:, 0, 2], lines[:, 0,3]))
lines_used:list[bool] = [True for _ in lines_in_list]
for x1,y1,x2,y2 in lines_in_list:
    cv2.line(img_houghline, (x1, y1), (x2, y2), (0, 255, 0), 2)

for i, j in itertools.combinations(range(len(lines_in_list)), 2):
    # すでに結合されていた場合は無視
    if not lines_used[i] or not lines_used[j]:
        continue
    l1 = lines_in_list[i]
    l2 = lines_in_list[j]
    arg_i = arg(l1); arg_j = arg(l2)
    p1, p2 = in_points(l1); q1, q2 = in_points(l2)
    
    l1_len = norm(p2 - p1)
    if (l1_len == 0):
        lines_used[i] = False
        continue
    # 傾きの近い線分のみを考慮する。
    if pi - abs(pi - abs(arg_i - arg_j)) >= angle_threshold:
        continue

    p_direction = (p2 - p1) / l1_len
    # 直線l1の上にl2の点を正射影する。
    modified_q1 = np.dot(q1 - p1, p_direction) * p_direction + p1
    modified_q2 = np.dot(q2 - p1, p_direction) * p_direction + p1
    # 横の方向にあまり離れすぎていない直線のみを考慮する。
    if norm(q1 - modified_q1) > transverse_threshold or norm(q2 - modified_q2) > transverse_threshold:
        continue

    projection = lambda v: np.dot(v - p1, p_direction)
    point_list = np.array([p1, p2, modified_q1, modified_q2])
    # 縦の方向にあまり離れすぎていない直線のみを考慮する。
    if (segment_distance(projection(p1), projection(p2), projection(modified_q1), projection(modified_q2)) > longitudinal_threshold):
        continue
    sorted_point_list = sorted(point_list, key=projection)
    assert(len(sorted_point_list) == 4)
    min_point = sorted_point_list[0]
    max_point = sorted_point_list[3]
    
    # x1 y1 x2 y2
    lines_in_list[i] = (int(min_point[0]),int(min_point[1]), int(max_point[0]), int(max_point[1]))
    lines_used[j] = False

reduced_line_count = len([lines_in_list[i] for i in range(len(lines_in_list)) if lines_used[i]])
print(f"reduced line count: {reduced_line_count}")

for x1,y1,x2,y2 in [lines_in_list[i] for i in range(len(lines_in_list)) if lines_used[i]]:
    cv2.line(img_platform_line, (x1, y1), (x2, y2), (0, 255, 0), 2)

cv2.imwrite(path + f"{file_name}-birateral.png",    img)
cv2.imwrite(path + f"{file_name}-edge.png",         edge_img)
cv2.imwrite(path + f"{file_name}-houghline.png",    img_houghline)
cv2.imwrite(path + f"{file_name}-{alpha:.2f}-{beta:.2f}-{gamma:.2f}platform.png",     img_platform_line)