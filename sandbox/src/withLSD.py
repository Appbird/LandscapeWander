import itertools
import cv2
import numpy as np
from cv2.typing import MatLike
from numpy.linalg import norm
from math import pi
from sys import argv
import time

import matplotlib.pyplot as plt

from utility import *

path = "./sandbox/out/lsd/"

def draw_current_postprocess(
    edge_img: MatLike,
    lines_in_list:list[Line],
    lines_used:list[bool],
    waiting_interval:int,
    i:int, j:int
):
    output_picture = portray_result(
        edge_img,
        [lines_in_list[k] for k in range(len(lines_in_list)) if lines_used[k]],
        (0, 255, 0)
    )
    output_picture = portray_result(
        output_picture,
        [lines_in_list[j]],
        (0, 0, 255)
    )
    output_picture = portray_result(
        output_picture,
        [lines_in_list[i]],
        (0, 0, 200)
    )
    cv2.imshow("process", output_picture)
    return cv2.waitKey(waiting_interval)

def preprocess(img:MatLike, alpha:float) -> MatLike:
    N = 3
    d = 8
    base_param = 1/2
    sigma_Color = lerp(alpha, 90, base_param)
    sigma_Space = lerp(alpha, 90, base_param)

    for _ in range(N):
        img = cv2.bilateralFilter(img, d, sigmaColor=sigma_Color, sigmaSpace=sigma_Space)

    return img

def extract_lines_by_hough(processed_image: MatLike, alpha:float, beta:float) -> list[Line]:
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

def extract_lines_by_lsd(processed_image: MatLike) -> list[Line]:
    lsd = cv2.createLineSegmentDetector()
    img_gray = cv2.cvtColor(processed_image, cv2.COLOR_BGR2GRAY)
    lines, width, prec, nfa = lsd.detect(img_gray)
    return MatLike_to_list_of_points(lines)

def postprocess(edge_img: MatLike, lines_in_list:list[Line],  gamma:float) -> list[Line]:
    base_param = 1/2
    after_height = edge_img.shape[0]

    angle_threshold = pi / 8
    transverse_threshold = after_height * lerp(gamma, 1/30, base_param)
    longitudinal_threshold = after_height * lerp(gamma, 1/30, base_param)

    lines_used:list[bool] = [True for _ in lines_in_list]
    
    iter_count = 0;
    waiting_interval = 10
    showing_frame_interval = 100
    for i, j in itertools.combinations(range(len(lines_in_list)), 2):
        iter_count += 1
        if (iter_count % showing_frame_interval == 0 and False):
            if (draw_current_postprocess(edge_img, lines_in_list, lines_used, waiting_interval, i, j) != -1):
                waiting_interval = 500 if (waiting_interval == 10) else 10
                showing_frame_interval = 1 if (showing_frame_interval == 100) else 100
                # キーを離すまで進めない
                while (cv2.waitKey(1) != -1): pass
            
            
        # すでに結合されていた場合は無視
        if not lines_used[i] or not lines_used[j]:
            continue
        l1 = lines_in_list[i]
        l2 = lines_in_list[j]
        p1, p2 = in_points(l1); q1, q2 = in_points(l2)
        
        l1_len = norm(p2 - p1)
        l2_len = norm(q2 - q1)
        if (l1_len == 0):
            lines_used[i] = False
            continue
        
        # 傾きの近い線分のみを考慮する。
        # print(abs(cross(p2 - p1, q2 - q1)/(l1_len*l2_len)), np.sin(angle_threshold))
        if abs(cross(p2 - p1, q2 - q1)/(l1_len*l2_len)) >= np.sin(angle_threshold):
            continue
        p_direction = (p2 - p1) / l1_len
        # 直線l1の上にl2の点を正射影する。
        # print(p2 - p1)
        modified_q1 = np.dot(q1 - p1, p_direction) * p_direction + p1
        modified_q2 = np.dot(q2 - p1, p_direction) * p_direction + p1
        
        # 横の方向にあまり離れすぎていない直線のみを考慮する。
        # print(q1, modified_q1)
        # print(q2, modified_q2)
        # print(norm(q1 - modified_q1), transverse_threshold)
        # print(norm(q2 - modified_q2), transverse_threshold)
        if norm(q1 - modified_q1) > transverse_threshold or norm(q2 - modified_q2) > transverse_threshold:
            continue
        
        projection = lambda v: np.dot(v - p1, p_direction)
        point_list = np.array([p1, p2, modified_q1, modified_q2])
        # 縦の方向にあまり離れすぎていない直線のみを考慮する。
        # print(segment_distance(projection(p1), projection(p2), projection(modified_q1), projection(modified_q2)), longitudinal_threshold)
        if (segment_distance(projection(p1), projection(p2), projection(modified_q1), projection(modified_q2)) > longitudinal_threshold):
            continue
        sorted_point_list = sorted(point_list, key=projection)
        assert(len(sorted_point_list) == 4)
        min_point = sorted_point_list[0]
        max_point = sorted_point_list[3]
        
        # x1 y1 x2 y2
        lines_in_list[i] = (
            int(min_point[0]), int(min_point[1]),
            int(max_point[0]), int(max_point[1])
        )
        lines_used[j] = False
    
    reduced_line = [lines_in_list[i] for i in range(len(lines_in_list)) if lines_used[i]]
    
    return reduced_line



def portray_result(img:MatLike, lines: list[Line], color:tuple[int, int, int]):
    drawn_img = img.copy()
    for x1,y1,x2,y2 in lines:
        cv2.line(drawn_img, (x1, y1), (x2, y2), color, 2)
    return drawn_img
    

def process_hough(
    img:MatLike,
    alpha:float,
    beta:float,
    gamma:float
):
    assert 0 <= alpha <= 1
    assert 0 <= beta <= 1
    assert 0 <= gamma <= 1

    edge_img = preprocess(img, alpha)
    lines_in_list = extract_lines_by_hough(edge_img, alpha, beta)
    result_lines = postprocess(edge_img, lines_in_list, gamma)

    return lines_in_list, result_lines

def process_lsd(
    img:MatLike,
    alpha:float,
    gamma:float
):
    assert 0 <= alpha <= 1
    assert 0 <= gamma <= 1

    edge_img = preprocess(img, alpha)
    lines_in_list = extract_lines_by_lsd(edge_img)
    result_lines = postprocess(edge_img, lines_in_list, gamma)

    return lines_in_list, result_lines

# TODO: パラメータの説明を加えよう
def tester_extract_lines(
        img:MatLike,
        file_base_name:str,
        tester_exec
    ):
    """
    パラメータ
    - `alpha` 対象物体の濃さ
    - `beta` 得られる線分の微細さ
    - `gamma` 線分の集約度合い
    """
    height = img.shape[0]
    after_height = 600
    f = after_height / height
    # 下げすぎると木の障害物などに大きく引っ張られやすくなる
    scaled_img = cv2.resize(img, None, fx=f, fy=f)

    time_start = time.time()
    lines_in_list, result_lines = tester_exec(scaled_img)
    time_end = time.time()

    print(f"total time: {(time_end - time_start):0.3f} [sec]")
    print(f"obtained line count: {len(lines_in_list)} -> {len(result_lines)}")
    drawn_img = portray_result(scaled_img, result_lines, (0, 255, 0))
    cv2.imwrite(path + f"out-{file_base_name}.png", drawn_img)



if __name__ == '__main__':
    assert(len(argv) == 2)

    file_name = argv[1]
    file_base_name = file_name.split(".")[0]
    input_target = cv2.imread("App/assets/test/" + file_name)
    if True:
        alpha = 0.5
        beta = 0.9
        gamma = 0
        tester_extract_lines(input_target.copy(), file_base_name + "-hough", lambda img: process_hough(img, alpha, beta, gamma))
        tester_extract_lines(input_target.copy(), file_base_name + "-lsd", lambda img: process_lsd(img, alpha, gamma))
    else:
        # FIXME: このデータの時おかしい
        first = [
            (200, 139, 10, 1),
            (160, 106, 305, 204)
        ]
        second = postprocess(input_target, first, gamma=0.5)
        cv2.imshow("p", portray_result(input_target, first, (0, 255, 0)))
        cv2.waitKey(0)
        cv2.imshow("p", portray_result(input_target, second, (0, 0, 255)))
        cv2.waitKey(0)