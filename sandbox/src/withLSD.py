import itertools
from types import NoneType
import cv2
import numpy as np
from cv2.typing import MatLike
from numpy.linalg import norm
from math import pi
import time

from sandbox.src.utility import *

path = "./test/out/bounding/"


def preprocess(img:MatLike, alpha:float) -> MatLike:
    N = 3
    d = 8
    base_param = 1/2
    sigma_Color = lerp(alpha, 90, base_param)
    sigma_Space = lerp(alpha, 90, base_param)

    for _ in range(N):
        img = cv2.bilateralFilter(img, d, sigmaColor=sigma_Color, sigmaSpace=sigma_Space)

    return img

def extract_lines(processed_image: MatLike, alpha:float, beta:float) -> list[Point]:
    base_param = 1/2
    
    img_gray = cv2.cvtColor(processed_image, cv2.COLOR_BGR2GRAY)

    canny_min = lerp(alpha, 30, base_param)
    canny_max = lerp(alpha, 100, base_param)
    edge_img = cv2.Canny(img_gray, canny_min, canny_max)

    base_param = 1/2
    after_height = edge_img.shape[0]

    # 確率的ハフ変換
    houghline_threshold = int(after_height/10)
    minLineLength_coef = lerp(1-beta, 1/10, base_param)
    maxLineGap_coef = lerp(1-beta, 1/20, base_param)
    houghline_minline_gap = after_height * minLineLength_coef
    houghline_maxline_gap = after_height * maxLineGap_coef
    lines = cv2.HoughLinesP(edge_img, 1, np.pi/180, houghline_threshold, None, houghline_minline_gap, houghline_maxline_gap)
    return MatLike_to_list_of_points(lines)

def postprocess(edge_img: MatLike, lines_in_list:list[Point],  gamma:float) -> list[Point]:
    base_param = 1/2
    after_height = edge_img.shape[0]

    angle_threshold = lerp(gamma, pi / 4, base_param)
    transverse_threshold = after_height * lerp(gamma, 1/10, base_param)
    longitudinal_threshold = after_height * lerp(gamma, 1/20, base_param)

    lines_used:list[bool] = [True for _ in lines_in_list]
    
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
    
    reduced_line = [lines_in_list[i] for i in range(len(lines_in_list)) if lines_used[i]]
    
    return reduced_line


def portray_result(img:MatLike, lines: list[Point], color:tuple[int, int, int]):
    img_houghline = img.copy()
    
    for x1,y1,x2,y2 in lines:
        cv2.line(img_houghline, (x1, y1), (x2, y2), color, 2)

# TODO: パラメータの説明を加えよう
def tester_hough(
        img:MatLike,
        file_name:str,
        alpha:float,
        beta:float,
        gamma:float
    ):
    """
    パラメータ
    - `alpha` 対象物体の濃さ
    - `beta` 得られる線分の微細さ
    - `gamma` 線分の集約度合い
    """
    time_start = time.time()

    height = img.shape[0]
    after_height = 600
    f = after_height / height
    # 下げすぎると木の障害物などに大きく引っ張られやすくなる
    img = cv2.resize(img, None, fx=f, fy=f)
    assert 0 <= alpha <= 1
    assert 0 <= beta <= 1
    assert 0 <= gamma <= 1

    edge_img = preprocess(img, alpha)
    lines_in_list = extract_lines(edge_img, alpha, beta)
    result_lines = postprocess(edge_img, lines_in_list, gamma)

    # CTODO : 出力の前に移動させる
    time_end = time.time()
    print(f"total time: {time_end - time_start:0.3f} [sec]")
    

    # TODO: postprocessの後の線分数
    print(f"obtained line count: {len(lines_in_list)} -> {len(result_lines)}")
    



    cv2.imwrite(path + f"{file_name}-birateral.png",    img)
    cv2.imwrite(path + f"{file_name}-edge.png",         edge_img)
    cv2.imwrite(path + f"{file_name}-houghline.png",    img_houghline)
    cv2.imwrite(path + f"{file_name}-platform.png",     img_platform_line)