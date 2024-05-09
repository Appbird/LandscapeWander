import itertools
from types import NoneType
import cv2
import numpy as np
from cv2.typing import MatLike
from numpy.linalg import norm
from math import pi
from sys import argv
import time

from utility import *

path = "./sandbox/out/lsd/"


def extract_lines_by_lsd(processed_image: MatLike) -> tuple[list[Line], list[float]]:
    lsd = cv2.createLineSegmentDetector()
    img_gray = cv2.cvtColor(processed_image, cv2.COLOR_BGR2GRAY)
    lines, width, prec, nfa = lsd.detect(img_gray)
    if (type(lines) == NoneType): return [], []
    return MatLike_to_list_of_points(lines), MatLike_to_list_of_floats(prec)


def portray_result(img:MatLike, lines: list[Line], params:list[float], max_value:float):
    drawn_img = img.copy()
    for (x1,y1,x2,y2), param  in zip(lines, params):
        print(param)
        cv2.line(drawn_img, (x1, y1), (x2, y2), (0, min(int(255 * (param / max_value)), 255), 0), 2)
    return drawn_img
    


def process_lsd(
    img:MatLike,
    alpha:float,
    gamma:float
):
    assert 0 <= alpha <= 1
    assert 0 <= gamma <= 1

    lines_in_list = extract_lines_by_lsd(img)

    return lines_in_list

# TODO: パラメータの説明を加えよう
def tester_extract_lines(
        img:MatLike,
        file_base_name:str
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

    result_lines, param = extract_lines_by_lsd(img)

    drawn_img = portray_result(scaled_img, result_lines, param, 0.25)
    cv2.imwrite(path + f"out-{file_base_name}.png", drawn_img)



if __name__ == '__main__':
    assert(len(argv) == 2)

    file_name = argv[1]
    file_base_name = file_name.split(".")[0]
    input_target = cv2.imread("App/assets/test/" + file_name)
    if True:
        tester_extract_lines(input_target.copy(), file_base_name + "-lsd-nfa")
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